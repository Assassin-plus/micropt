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
    //Group* photonGroup = new Group();
    /* for(int i = 0; i < cam->getWidth() * cam->getHeight(); i ++){
        PixelMap.push_back(SPPMPixel());
    } */
    PixelMap = std::vector<SPPMPixel>(cam->getWidth() * cam->getHeight(), SPPMPixel(sharedRadius));
    int numProcs = omp_get_num_procs();
    for(int iter = 0; iter < iteration; iter ++){
        //distributed ray tracing
        std::cout << "iteration " << iter << std::endl;

        #pragma omp parallel for schedule(dynamic, 1)
        for(int i = 0; i < PixelMap.size(); i ++){
            //printf("\rray tracing progress: %.2f%%", (float)i / (float)PixelMap.size() * 100);
            auto& pixel = PixelMap[i];
            pixel.hasHit = false;
            pixel.radius = iter == 0 ? sharedRadius : pixel.radius;
            //generate ray
            unsigned short Xi[3] = {0, 0, i * i * i};
            double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
            double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        
            Vector2f xy = Vector2f(i % cam->getWidth() + dx / 4 + 0.5, i / cam->getWidth() + dy / 4 + 0.5);
            Ray ray = cam->generateRay(xy);
            Hit hit;
            //std::cout << "ray: " << ray << std::endl;
            //ray tracing
            //bounce until hit diffuse surface or reach max depth
            Vector3f throughput = Vector3f(1, 1, 1);
            int currentDepth = 0;

            while(true){
                if(!group->intersect(ray, hit, EPS)) break;
                //std::cout << "hit: " << hit << std::endl;
                Material* material = hit.getMaterial();
                Vector3f hitPoint = ray.pointAtParameter(hit.getT());
                Vector3f normal = hit.getNormal();
                Vector3f wo = -ray.getDirection();
                //std::cout << "hit point: " << hitPoint << std::endl;

            
                DiscreteMaterial *m = dynamic_cast<DiscreteMaterial*>(material);
                if(m != nullptr){
                    if(m->getMaterialType() == BRDFType::DIFFUSE){
                        //direct lighting
                        for(auto light : lights){
                            Vector3f dirToLight, col;
                            light->getIllumination(hitPoint, dirToLight, col);
                            Ray shadowRay = Ray(hitPoint, dirToLight);
                            Hit shadowHit;
                            if(!group->intersect(shadowRay, shadowHit, EPS) || shadowHit.getT() > Vector3f::dot(dirToLight, dirToLight)){
                                pixel.Ld += throughput * col * m->getDiffuseColor() ;
                            }
                        }
                        //indirect lighting
                        pixel.hasHit = true;
                        pixel.vp = SPPMPixel::VisiblePoint(hitPoint, normal, wo, m, throughput);
                        //sample new direction
                        //cosine weighted hemisphere sampling
                        double r1 = 2 * M_PI * erand48(Xi), r2 = erand48(Xi), r2s = sqrt(r2);
                        Vector3f w = normal, u = (Vector3f::cross((fabs(w.x()) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0)),  w)).normalized(), v = Vector3f::cross(w, u);
                        Vector3f newDirection = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();
                        ray = Ray(hitPoint, newDirection);
                        throughput *= m->getDiffuseColor() ;
                        currentDepth ++;
                        //Russian Roulette
                        if(currentDepth > depth){
                            break;
                        }
                    
                    }else if(m->getMaterialType() == BRDFType::SPECULAR){
                        //specular reflection
                        Vector3f wi = normal * (2 * Vector3f::dot(normal, wo)) - wo;
                        throughput *= m->getDiffuseColor() ;
                        ray = Ray(hitPoint, wi);
                        currentDepth ++;
                        if(currentDepth > depth){
                            break;
                        }
                    }else if(m->getMaterialType() == BRDFType::REFRACTION){
                        Vector3f nl = Vector3f::dot(normal, wo) < 0 ? normal : normal * -1;
                        bool into = Vector3f::dot(normal, nl) > 0;
                        Vector3f reflectionDirection = (- wo + normal * 2 * Vector3f::dot(normal, wo)).normalized();
                        double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = Vector3f::dot(wo, nl), cos2t;
                        if((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0){
                            ray = Ray(hitPoint, reflectionDirection);
                            throughput *= m->getDiffuseColor();
                            currentDepth ++;
                        }else{
                            Vector3f refractDirection = (-wo * nnt - normal * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalized();
                            double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : Vector3f::dot(refractDirection, normal));
                            double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = 0.25 + 0.5 * Re, RP = Re / P, TP = Tr / (1 - P);
                            currentDepth ++;
                            if(currentDepth > depth){
                                if(erand48(Xi) < P){
                                    throughput *= 2 * RP;
                                    ray = Ray(hitPoint, reflectionDirection);   
                                }else{
                                    throughput *= 2 * TP;
                                    ray = Ray(hitPoint, refractDirection);
                                }
                            }else{
                                Ray reflectionRay = Ray(hitPoint, reflectionDirection);
                                Ray refractRay = Ray(hitPoint, refractDirection);
                                if(erand48(Xi) < 0.5f){
                                    throughput *= Re;
                                    ray = reflectionRay;
                                }else{
                                    throughput *= Tr;
                                    ray = refractRay;
                                }
                            }
                        }
                    }else{
                        std::cout << "Unsupported material" << std::endl;
                        exit(-1);
                    }
                }else {
                    EmpiricalMaterial *m1 = dynamic_cast<EmpiricalMaterial*>(material);
                    if(m1 != nullptr){

                    }else{
                        std::cout << "Unsupported material" << std::endl;
                        exit(-1);
                    }
                }
            }

        }
        printf("\n");
        //photon tracing
        #pragma omp parallel for schedule(dynamic, 1)
        for(int i = 0; i < photonCount; i++){
            //printf("\rphoton tracing progress: %.2f%%", (float)i / (float)photonCount * 100);
            unsigned short Xi[3] = {0, 0, i * i * i};
            int lightIndex = int(erand48(Xi) * lights.size());
            Light* light = lights[lightIndex];
            Photon photon = light->emitPhotonSampler();
            Ray ray = Ray(photon.p, photon.wi);
            Hit hit;
            Vector3f throughput = photon.alpha ;
            int currentDepth = 0;

            while(true){
                if(!group->intersect(ray, hit, EPS)) break;
                Material* material = hit.getMaterial();
                Vector3f hitPoint = ray.pointAtParameter(hit.getT());
                Vector3f normal = hit.getNormal();
                Vector3f wi = ray.getDirection();
                DiscreteMaterial *m = dynamic_cast<DiscreteMaterial*>(material);
                if(m!=nullptr){
                    if(m->getMaterialType() == BRDFType::DIFFUSE){
                        for(auto& pixel : PixelMap){
                            if(pixel.hasHit){
                                if((hitPoint - pixel.vp.p).length() < pixel.radius){
                                    pixel.tau += throughput * m->getDiffuseColor();
                                    pixel.vp.cnt ++;
                                }
                            }
                        }
                        //sample new direction
                        //cosine weighted hemisphere sampling
                        double r1 = 2 * M_PI * erand48(Xi), r2 = erand48(Xi), r2s = sqrt(r2);
                        Vector3f w = normal, u = (Vector3f::cross((fabs(w.x()) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0)),  w)).normalized(), v = Vector3f::cross(w, u);
                        Vector3f newDirection = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();
                        ray = Ray(hitPoint, newDirection);
                        throughput *= m->getDiffuseColor() ;
                        currentDepth ++;

                        //Russian Roulette
                        if(currentDepth > depth){
                            break;
                        }
                    }else if(m->getMaterialType() == BRDFType::SPECULAR){
                        Vector3f wo = wi - normal * 2 * Vector3f::dot(normal, wi);
                        throughput *= m->getDiffuseColor() ;
                        ray = Ray(hitPoint, wo);
                        currentDepth ++;
                        if(currentDepth > depth){
                            break;
                        }
                    }else if(m->getMaterialType() == BRDFType::REFRACTION){
                        Vector3f nl = Vector3f::dot(normal, wi) < 0 ? normal : normal * -1;
                        bool into = Vector3f::dot(normal, nl) > 0;
                        Vector3f reflectionDirection = ( wi - normal * 2 * Vector3f::dot(normal, wi)).normalized();
                        double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = Vector3f::dot(wi, nl), cos2t;
                        if((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0){
                            ray = Ray(hitPoint, reflectionDirection);
                            throughput *= m->getDiffuseColor();
                            currentDepth ++;
                        }else{
                            Vector3f refractDirection = (wi * nnt - normal * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalized();
                            double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : Vector3f::dot(refractDirection, normal));
                            double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = 0.25 + 0.5 * Re, RP = Re / P, TP = Tr / (1 - P);
                            currentDepth ++;
                            if(currentDepth > depth){
                                if(erand48(Xi) < P){
                                    throughput *= RP;
                                    ray = Ray(hitPoint, reflectionDirection);   
                                }else{
                                    throughput *= TP;
                                    ray = Ray(hitPoint, refractDirection);
                                }
                            }else{
                                Ray reflectionRay = Ray(hitPoint, reflectionDirection);
                                Ray refractRay = Ray(hitPoint, refractDirection);
                                if(erand48(Xi) < 0.5f){
                                    throughput *= 2 * Re;
                                    ray = reflectionRay;
                                }else{
                                    throughput *= 2 * Tr;
                                    ray = refractRay;
                                }
                            }
                        }
                    }
                }else{
                    EmpiricalMaterial *m1 = dynamic_cast<EmpiricalMaterial*>(material);
                    if(m1 != nullptr){

                    }else{
                        std::cout << "Unsupported material" << std::endl;
                        exit(-1);
                    }
                }
            }
        }

        //update pixel map
        #pragma omp parallel for schedule(dynamic, 1)
        for(auto& pixel: PixelMap){
            if(pixel.vp.cnt == 0 && pixel.n == 0) continue;
            float shrink = (float) (pixel.n + alpha * pixel.vp.cnt) / (float) (pixel.n + pixel.vp.cnt);
            pixel.radius *= sqrt(shrink);
            pixel.tau = (pixel.tau + pixel.vp.tau) * shrink;
            pixel.n += alpha * pixel.vp.cnt;
            pixel.vp = SPPMPixel::VisiblePoint();
        }

        std::cout << "iteration " << iter << " finished" << std::endl;
        std::cout << "pixel radius: " << PixelMap[0].radius << std::endl;
        std::cout << "pixel n: " << PixelMap[0].n << std::endl;
        std::cout << "pixel tau: " << PixelMap[0].tau << std::endl;

    }
    std::cout << "rendering finished" << std::endl;
    //write image
    for(int i = 0; i < cam->getWidth(); i ++){
        for(int j = 0; j < cam->getHeight(); j ++){
            int index = j * cam->getWidth() + i;
            auto pixel = PixelMap[index];
            Vector3f color = pixel.tau / (M_PI * pixel.radius * pixel.radius * photonCount * iteration) + pixel.Ld / (iteration);
            color = clamp(color);
            //std::cout << "color: " << color * 255 << std::endl;
            image->SetPixel(i, cam->getHeight() - j - 1, color * 255);
        }
    }

}