#include "../include/sppm.hpp"
#include "../include/utils.hpp"
#include "../include/group.hpp"
#include "../include/image.hpp"
#include "../include/scene_parser.hpp"
#include "../include/classical_object.hpp"
#include "../include/camera.hpp"
#include <iostream>
#include <memory>
#include <vector>

void SPPMIntegrator::render(const SceneParser& scene, RgbImage *&image) {
    std::cout
            << "\npixel nums: " << PixelMap.size()
            << "\niteration nums: " << iteration
            << "\nphoton nums per pass: " << photonCount
            << std::endl;
    std::cout << "start rendering..." << std::endl;

    //initialize
    const int depth = 20;
    std::vector<Light*> lights = scene.getLights();
    Group* group = scene.getGroup();
    Camera* cam = scene.getCamera();
    Group* photonGroup = new Group();
    /* for(int i = 0; i < cam->getWidth() * cam->getHeight(); i ++){
        PixelMap.push_back(SPPMPixel());
    } */
    PixelMap = std::vector<SPPMPixel>(cam->getWidth() * cam->getHeight(), SPPMPixel());

    for(int iter = 0; iter < iteration; iter ++){
    //photon tracing
        photonMap = std::make_unique<KDTree<Photon>>();
        photonMap->reserve(photonCount);
        std::vector<Photon> photons;

        int photonStored = 0;
        int photonEmitted = 0;
        int photonControl = photonCount * 5;

        #pragma omp parallel for schedule(dynamic, 1) shared(photonStored, photonEmitted)
        for(int i = 0; i < photonControl; i ++){
            if(photonStored >= photonCount){
                continue;
            }
            //randomly choose a light
            int lightIndex = (int)(rand() % lights.size());
            Light* light = lights[lightIndex];
            Photon photon = light->emitPhotonSampler();
            #pragma omp atomic
            photonEmitted ++;
            Hit hit ;

            Ray ray = Ray(photon.p, photon.wi);
            Vector3f beta = Vector3f(1.0f, 1.0f, 1.0f);
            int currentDepth = 0;

            if(!group->intersect(ray, hit, 0.0001f)){
                continue;
            }

            while(true){
                Material* material = hit.getMaterial();
                Vector3f p = ray.pointAtParameter(hit.getT());

                if(material->getMaterialType() == BRDFType::DIFFUSE ||
                    material->getMaterialType() == BRDFType::MICROFACET){
                    //store photon
                    photonStored ++;
                    #pragma omp critical
                    photons.push_back(Photon(p, photon.alpha * beta, photon.wi));
                    
                    //visualize photon
                    //#pragma omp critical
                    //photonGroup->addObject(new Sphere(p, 1f, new PhongMaterial(Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 1.0f, 1.0f), 20)));
                    //break;
                }
                
                Vector3f albedo = material->getDiffuseColor();
                if(material->hasTexture()){
                    EmpiricalMaterial* m = dynamic_cast<EmpiricalMaterial*>(material);
                    albedo = m->getDiffuseColor(hit.getTexCoord());
                }
                if(albedo == Vector3f::ZERO){
                    break;
                }
                beta *= albedo;

                Vector3f newDirection;
                Vector3f n = hit.getNormal();

                if(material->getMaterialType() == BRDFType::SPECULAR){
                    newDirection =  ray.getDirection() - 2 * Vector3f::dot(ray.getDirection(), n) * n;
                    newDirection.normalize();
                }else if(material->getMaterialType() == BRDFType::REFRACTION){
                    Vector3f wi = -ray.getDirection();
                    Vector3f nl = Vector3f::dot(n,wi) < 0 ? n : n * -1; //orienting normal
                    Vector3f reflectionDirection = wi - n * 2 * Vector3f::dot(n,(wi));
                    reflectionDirection.normalize();
                    bool into = Vector3f::dot(n,nl) > 0;
                    double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = Vector3f::dot(wi,nl), cos2t;
                    if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) {
                        //total internal reflection
                        newDirection = reflectionDirection;
                    }else{
                    Vector3f refractionDirection = (wi * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalized();
                    
                    double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : Vector3f::dot(refractionDirection,n));
                    double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = 0.25 + 0.5 * Re, RP = Re / P, TP = Tr / (1 - P);
                    unsigned short Xi[3] = {0, 0, 8192};
                    if(currentDepth > depth){
                    if(erand48(Xi) < P){
                        //beta *= RP;
                        newDirection = reflectionDirection;
                    }else{
                        //beta *= TP;
                        newDirection = refractionDirection;
                    }
                    }else{
                        if(erand48(Xi) < 0.5){
                            //beta *= 2 * Re;
                            newDirection = reflectionDirection;
                        }else{
                            //beta *= 2 * Tr;
                            newDirection = refractionDirection;
                        }
                    }
                    }
                }else if(material->getMaterialType() == BRDFType::DIFFUSE){
                    //sample a new direction
                    Vector3f w = n;
                    Vector3f u = Vector3f::cross((fabs(w.x()) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0)), w).normalized();
                    Vector3f v = Vector3f::cross(w, u);
                    unsigned short Xi[3] = {0, 0, iter * iter * iter};
                    float r1 = 2 * M_PI * erand48(Xi);
                    float r2 = erand48(Xi), r2s = sqrt(r2);
                    newDirection = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();
                    beta *= material->getDiffuseColor();
                }else if(material->getMaterialType() == BRDFType::MICROFACET){
                    //sample according to material brdf
                    EmpiricalMaterial* m = dynamic_cast<EmpiricalMaterial*>(material);
                    if(m == nullptr){
                        std::cout << "error: material type not supported" << std::endl;
                        break;
                    }
                    Vector3f wi = ray.getDirection();
                    Vector3f wo = m->sampleBRDF(wi, n);
                    if(wo == Vector3f::ZERO){
                        break;
                    }
                    newDirection = wo;
                    beta *= m->evalBRDF(wi, wo, n);
                }

                ray = Ray(p, newDirection);

                if(!group->intersect(ray, hit, 0.0001f)){
                    break;
                }

                if(currentDepth >= depth){
                    currentDepth ++;
                }else{
                    //russian roulette
                    float p = std::max(beta.x(), std::max(beta.y(), beta.z()));
                    if(rand() / (float)RAND_MAX > p){
                        break;
                    }
                    beta /= p;
                }

            }
        }
        //visualize photon
        /* for(auto photon: photons){
            //std::cout << "Photon: "<< photon.p << std::endl;
            photonGroup->addObject(new Sphere(photon.p, 0.1f, new PhongMaterial(Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 1.0f, 1.0f), 20)));
        }
        for(int i = 0; i < cam->getWidth();i++){
            for(int j = 0; j < cam->getHeight();j++){
                Vector2f pix = Vector2f(i + 0.5f, j + 0.5f);
                Ray camRay = scene.getCamera()->generateRay(pix);
                Hit hit;
                if(!photonGroup->intersect(camRay, hit, 0.0001f)){
                    continue;
                }
                //std::cout<<"Intersect at: "<<hit.getT()<<std::endl;
                image->SetPixel(i, j, Vector3f(1.0f, 1.0f, 1.0f) * 255);
            }
        } */

        photonMap->build(photons, 3);
        photons.clear();
        std::cout << "photon stored: " << photonStored << std::endl;
        std::cout << "photon emitted: " << photonEmitted << std::endl;
        photonTotal += photonEmitted;
    //visible point gathering
    #pragma omp parallel for schedule(dynamic, 1) shared(photonStored, photonEmitted, photonTotal)
        for(int i = 0; i < PixelMap.size(); i ++){
            auto& pixel = PixelMap[i];
            //sample a ray
            int y = i / image->Height();
            int x = i % image->Height();
            unsigned short Xi[3] = {0, 0, iter * iter * iter};
            double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
            double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
            //generate ray
            Vector2f pix = Vector2f(x + (0.5 + dx) , y + (0.5 + dy) );
            Ray camRay = scene.getCamera()->generateRay(pix);
            Hit hit;
            Vector3f beta = Vector3f(1.0f, 1.0f, 1.0f);
            int currentDepth = 0;

            if(!group->intersect(camRay, hit, 0.0001f)){
                continue;
            }
            
            while(true){
                Material* material = hit.getMaterial();
                Vector3f p = camRay.pointAtParameter(hit.getT());

                if(material->getMaterialType() == BRDFType::EMISSION){
                    //visualize pixel
                    DiscreteMaterial *m = dynamic_cast<DiscreteMaterial*>(material);
                    pixel.Ld += beta * m->getEmissionColor();
                    break;
                }
                if(material->getMaterialType() == BRDFType::DIFFUSE ||
                    material->getMaterialType() == BRDFType::MICROFACET){
                    DiscreteMaterial *m = dynamic_cast<DiscreteMaterial*>(material);
                    pixel.Ld += beta * m->getEmissionColor();
                    if(pixel.hasHit == false){
                        pixel.hasHit = true;
                        photonMap->kNNSearch(Photon(p, Vector3f::ZERO, Vector3f::ZERO), 100, &(pixel.radius));
                    }
                    auto photons = photonMap->rangeSearch(p, pixel.radius);
                    int photonNum = photons.size();
                    if(photonNum == 0){
                        break;
                    }
                    double shrink = (double) (pixel.n + alpha * photonNum) / (pixel.n + photonNum);
                    pixel.n += photonNum * alpha;
                    pixel.radius *= sqrt(shrink);
                    Vector3f flux = Vector3f::ZERO;
                    for(auto& photon : photons){
                        Vector3f wi = photon.wi;
                        Vector3f wo = camRay.getDirection();
                        Vector3f n = hit.getNormal();
                        Vector3f nl = Vector3f::dot(n,wo) < 0 ? n : n * -1; //orienting normal
                        Vector3f f ;
                        EmpiricalMaterial* m = dynamic_cast<EmpiricalMaterial*>(material);
                        if(m != nullptr){
                            f = m->evalBRDF(wi, wo, nl);
                        }else{
                            DiscreteMaterial* m1 = dynamic_cast<DiscreteMaterial*>(material);
                            //uniform brdf
                            if(m1 != nullptr){
                                //Phong
                                f = m1->getDiffuseColor();
                            }
                            else{
                                std::cout << "error: material type not supported" << std::endl;
                                f = Vector3f::ZERO;
                            }
                        }
                        flux += f * photon.alpha ;
                    }
                    pixel.tau = (pixel.tau + flux) * shrink;
                    break;
                }
                Vector3f newDirection;

                if(material->getMaterialType() == BRDFType::SPECULAR){
                    Vector3f wi = camRay.getDirection();
                    Vector3f wo = wi - 2 * Vector3f::dot(wi, hit.getNormal()) * hit.getNormal();
                    newDirection = wo;
                    DiscreteMaterial *m1 = dynamic_cast<DiscreteMaterial*>(material);
                    Vector3f albedo;
                    if(m1 != nullptr)
                        albedo = m1->getDiffuseColor();
                    if(material->hasTexture()){
                        EmpiricalMaterial* m = dynamic_cast<EmpiricalMaterial*>(material);
                        albedo = m->getDiffuseColor(hit.getTexCoord());
                    }
                    if(albedo == Vector3f::ZERO){
                        break;
                    }
                    beta *= albedo;//TODO: check if this is correct
                }else if(material->getMaterialType() == BRDFType::REFRACTION){
                    Vector3f wi = camRay.getDirection();
                    Vector3f n = hit.getNormal().normalized();
                    Vector3f nl = Vector3f::dot(n,wi) < 0 ? n : n * -1; //orienting normal
                    Vector3f reflectionDirection = wi- n * 2 * Vector3f::dot(n,(wi));
                    bool into = Vector3f::dot(n,nl) > 0;
                    double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = Vector3f::dot(wi,nl), cos2t;
                    if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) {
                        //total internal reflection
                        newDirection = reflectionDirection;
                    }else{
                    Vector3f refractionDirection = (wi * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalized();
                    
                    double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : Vector3f::dot(refractionDirection,n));
                    double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = 0.25 + 0.5 * Re, RP = Re / P, TP = Tr / (1 - P);
                    unsigned short Xi[3] = {0, 0, 8192};
                    if(currentDepth > depth){
                    if(erand48(Xi) < P){
                        //beta *= RP;
                        newDirection = reflectionDirection;
                    }else{
                        //beta *= TP;
                        newDirection = refractionDirection;
                    }
                    }else{
                        if(erand48(Xi) < 0.5){
                            //beta *= 2 * Re;
                            newDirection = reflectionDirection;
                        }else{
                            //beta *= 2 * Tr;
                            newDirection = refractionDirection;
                        }
                    }
                    }
                }
                camRay = Ray(p, newDirection);

                if(!group->intersect(camRay, hit, 0.0001f)){
                    break;
                }

                if(currentDepth <= depth){
                    currentDepth ++;
                }else{
                    //russian roulette
                    float p = std::max(beta.x(), std::max(beta.y(), beta.z()));
                    if(rand() / (float)RAND_MAX > p){
                        break;
                    }
                    beta /= p;
                }
            }
        }
        std::cout << "iteration " << iter << " finished" << std::endl;
        std::cout << "Flux: " << PixelMap[0].tau << std::endl;
        std::cout << "Radius: " << PixelMap[0].radius << std::endl;
        std::cout << "Number: " << PixelMap[0].n << std::endl;
    }
    for(int y = 0; y < image->Height(); ++y) {
        for(int x = 0; x < image->Width(); ++x) {
        auto& pixel = PixelMap[y * image->Width() + x];
        Vector3f color = pixel.tau / (iteration * M_PI * pixel.radius * pixel.radius) + pixel.Ld / (iteration);
        image->SetPixel(x, image->Height()-1-y,  clamp(color) * 255);
        //std::cout << "pixel " << x << " " << y << " " << color * 255 << std::endl;
        }
    }
    std::cout << "rendering finished" << std::endl;

}