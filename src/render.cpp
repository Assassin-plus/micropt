#include <iostream>
#include <memory>
#include <omp.h>

#include <vecmath.h>
#include "../include/render.hpp"
#include "../include/image.hpp"
#include "../include/scene_parser.hpp"
#include "../include/camera.hpp"
#include "../include/light.hpp"
#include "../include/group.hpp"
#include "../include/hit.hpp"
#include "../include/utils.hpp"
#include "../include/curve.hpp"
#include "../include/sppm.hpp"

Vector3f radiance(const Ray &ray,int currentDepth, int depth, unsigned short *Xi, const SceneParser& scene) {
    Hit hit;
    if (!scene.getGroup()->intersect(ray, hit, EPS)) {
        return scene.getBackgroundColor();
    }
    Vector3f x = ray.pointAtParameter(hit.getT());//hit point
    Vector3f n = hit.getNormal().normalized();
    Vector3f nl = Vector3f::dot(n,ray.getDirection()) < 0 ? n : n * -1; //orienting normal
    //EmpiricalMaterial* m = dynamic_cast<EmpiricalMaterial*>(hit.getMaterial());
    DiscreteMaterial* m = dynamic_cast<DiscreteMaterial*>(hit.getMaterial());
    if(m == nullptr) { 
        EmpiricalMaterial* m = dynamic_cast<EmpiricalMaterial*>(hit.getMaterial());
        if(m == nullptr) {
            std::cout << "Error: material is neither discrete nor empirical." << std::endl;
            exit(1);
        }
        //Empirical Material
        Vector3f f = m->getDiffuseColor();
        if(m->hasTexture()) {
            Vector2f texCoord = hit.getTexCoord();
            f = m->getDiffuseColor(texCoord);
        }
        
        double p = f.x() > f.y() && f.x() > f.z() ? f.x() : f.y() > f.z() ? f.y() : f.z();// max reflectance
        currentDepth++;
        //std::cout << "currentDepth: " << currentDepth << std::endl;
        if (currentDepth > depth) {
            if (erand48(Xi) < p && currentDepth <= 10) {
                f = f * (1 / p);
            } else {
                return m->getEmissionColor();
            }
        }

        if (m->getMaterialType() == BRDFType::MICROFACET) {
        // importance sample BRDF
            Vector3f wo = -ray.getDirection();
            Vector3f wi = m->sampleBRDF(wo, nl);
            Ray newRay(x, wi);
                     f = m->evalBRDF(wi, wo, nl);
            return m->getEmissionColor() + f * radiance(newRay, currentDepth + 1, depth, Xi, scene);
        } else if (m->getMaterialType() == BRDFType::DIFFUSE) {
            double angle = 2 * M_PI * erand48(Xi), distance = erand48(Xi), distanceSqrt = sqrt(distance);
            //generate orthonormal basis (w, u, v) according to normal
            Vector3f w = nl, u = (Vector3f::cross((fabs(w.x()) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0)),w)).normalized(), v = Vector3f::cross(w,u);
            //generate random reflection ray direction
            Vector3f direction = (u * cos(angle) * distanceSqrt + v * sin(angle) * distanceSqrt + w * sqrt(1 - distance)).normalized();
            return m->getEmissionColor() + f * (radiance(Ray(x, direction), currentDepth, depth, Xi, scene));
        } else if (m->getMaterialType() == BRDFType::SPECULAR) {
            // Ideal Specular Reflection
            Vector3f reflectionDirection = ray.getDirection() - n * 2 * Vector3f::dot(n,(ray.getDirection()));
            Ray reflectionRay(x, reflectionDirection);
            return m->getEmissionColor() + f * (radiance(reflectionRay, currentDepth + 1, depth, Xi, scene));
        } else if (m->getMaterialType() == BRDFType::REFRACTION) {
            // Ideal Specular Refraction
            //judge whether ray is entering or leaving the material
            Vector3f reflectionDirection = ray.getDirection() - n * 2 * Vector3f::dot(n,(ray.getDirection()));
            bool into = Vector3f::dot(n,nl) > 0;
            //nc: refractive index of air
            //nt: refractive index of material (water: 1.33, glass: 1.52, diamond: 2.42)
            //nnt: ratio of refractive indices
            //ddn: cosine of incident angle
            //cos2t: cosine of refraction angle
            double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = Vector3f::dot(ray.getDirection(),nl), cos2t;
            if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) {
                //total internal reflection
                return m->getEmissionColor() + f * (radiance(Ray(x, reflectionDirection), currentDepth, depth, Xi, scene));
            }
            Vector3f refractionDirection = (ray.getDirection() * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalized();
            //R0: reflectance at normal incidence based on IOR
            //c: 1 - cosine of angle between incident ray and normal
            //Re: Fresnel reflectance
            //Tr: Fresnel transmittance
            //P: Schlick's approximation
            //RP: probability of reflection
            //TP: probability of transmission
            double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : Vector3f::dot(refractionDirection,n));
            double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = 0.25 + 0.5 * Re, RP = Re / P, TP = Tr / (1 - P);
            return m->getEmissionColor() + f * (currentDepth > depth ? 
                (erand48(Xi) < P ?//Russian roulette
                radiance(Ray(x, reflectionDirection), currentDepth, depth, Xi, scene) * RP :
                radiance(Ray(x, refractionDirection), currentDepth, depth, Xi, scene) * TP) :
                    radiance(Ray(x, reflectionDirection), currentDepth, depth, Xi, scene) * Re + 
                    radiance(Ray(x, refractionDirection), currentDepth, depth, Xi, scene) * Tr);
        } else if (m->getMaterialType() == BRDFType::EMISSION) {
            // Emission
            return m->getEmissionColor();
        } else if (m->getMaterialType() == BRDFType::SUBSURFACE) {
            // Subsurface Scattering
        } else if (m->getMaterialType() == BRDFType::MEDIA) {
            // Media
        } else {
            // None
            std::cout << "Error: material type unexpected." << std::endl;
        }

        
    }else {//Discrete Material

        Vector3f f = m->getDiffuseColor();
        
        double p = f.x() > f.y() && f.x() > f.z() ? f.x() : f.y() > f.z() ? f.y() : f.z();// max reflectance
        currentDepth++;
        //std::cout << "currentDepth: " << currentDepth << std::endl;
        if (currentDepth > depth) {
            if (erand48(Xi) < p && currentDepth <= 10) {
                f = f * (1 / p);
            } else {
                return m->getEmissionColor();
            }
        }
        if (m->getMaterialType() == BRDFType::DIFFUSE) {
            //Ideal Diffuse Reflection
            //cosine-weighted importance sampling
            //sample hemisphere
            double angle = 2 * M_PI * erand48(Xi), distance = erand48(Xi), distanceSqrt = sqrt(distance);
            //generate orthonormal basis (w, u, v) according to normal
            Vector3f w = nl, u = (Vector3f::cross((fabs(w.x()) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0)),w)).normalized(), v = Vector3f::cross(w,u);
            //generate random reflection ray direction
            Vector3f direction = (u * cos(angle) * distanceSqrt + v * sin(angle) * distanceSqrt + w * sqrt(1 - distance)).normalized();
            return m->getEmissionColor() + f * (radiance(Ray(x, direction), currentDepth, depth, Xi, scene));
        } else if (m->getMaterialType() == BRDFType::SPECULAR) {
            //Ideal Specular Reflection
            Vector3f reflectionDirection = ray.getDirection() - n * 2 * Vector3f::dot(n,(ray.getDirection()));
            Ray reflectionRay(x, reflectionDirection);
            return m->getEmissionColor() + f * (radiance(reflectionRay, currentDepth, depth, Xi, scene));
        } else {
            //Ideal Specular Refraction
            //judge whether ray is entering or leaving the material
            Vector3f reflectionDirection = ray.getDirection() - n * 2 * Vector3f::dot(n,(ray.getDirection()));
            bool into = Vector3f::dot(n,nl) > 0;
            //nc: refractive index of air
            //nt: refractive index of material (water: 1.33, glass: 1.52, diamond: 2.42)
            //nnt: ratio of refractive indices
            //ddn: cosine of incident angle
            //cos2t: cosine of refraction angle
            double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = Vector3f::dot(ray.getDirection(),nl), cos2t;
            if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) {
                //total internal reflection
                return m->getEmissionColor() + f * (radiance(Ray(x, reflectionDirection), currentDepth, depth, Xi, scene));
            }
            Vector3f refractionDirection = (ray.getDirection() * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalized();
            //R0: reflectance at normal incidence based on IOR
            //c: 1 - cosine of angle between incident ray and normal
            //Re: Fresnel reflectance
            //Tr: Fresnel transmittance
            //P: Schlick's approximation
            //RP: probability of reflection
            //TP: probability of transmission
            double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : Vector3f::dot(refractionDirection,n));
            double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = 0.25 + 0.5 * Re, RP = Re / P, TP = Tr / (1 - P);
            return m->getEmissionColor() + f * (currentDepth > depth ? 
                (erand48(Xi) < P ?//Russian roulette
                radiance(Ray(x, reflectionDirection), currentDepth, depth, Xi, scene) * RP :
                radiance(Ray(x, refractionDirection), currentDepth, depth, Xi, scene) * TP) :
                    radiance(Ray(x, reflectionDirection), currentDepth, depth, Xi, scene) * Re + 
                    radiance(Ray(x, refractionDirection), currentDepth, depth, Xi, scene) * Tr);
        }

    }
    
}

void PathTracingRenderer::render(const SceneParser& scene, RgbImage*& image, int samples, int threads, int depth, bool DOF, float aperture, float focalLength) {
    std::cout << "Rendering with Path Tracing..." << std::endl;
    //set main parameters
    Camera *camera = scene.getCamera();
    camera->setDOF(DOF, aperture, focalLength);
    std::cout << "camera: " << camera->getWidth() << " " << camera->getHeight() << std::endl;
    image = new RgbImage(camera->getWidth(), camera->getHeight());
    assert(image!=nullptr);

    //currently set material.
    Group* baseGroup = scene.getGroup();

    omp_set_num_threads(threads);
    Vector3f finalColor = Vector3f::ZERO;
    Vector3f color = Vector3f::ZERO;
#pragma omp parallel for schedule(dynamic, 1) private(finalColor,color)
    //Loop over screen space pixels
    for(int y = 0; y < camera->getHeight(); ++y) {
        unsigned short Xi[3] = {0, 0, y*y*y};
        for(int x = 0; x < camera->getWidth(); ++x) {
            //SAMPLER
            //current: SMAA x4
            for(int sy = 0; sy < 2; ++sy) {
                for(int sx = 0; sx < 2; ++sx) {
                    color = Vector3f::ZERO;
                    for(int s = 0; s < samples; ++s) {
                        //FILTER
                        //current: tent filter
                        double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        //generate ray
                        Vector2f pixel = Vector2f((sx + 0.5 + dx) / 2 + x, (sy + 0.5 + dy) / 2 + y);
                        Ray camRay = camera->generateRay(pixel);
                        //trace ray
                        color += radiance(camRay, 0, depth, Xi, scene) * (1.0 / samples);
                        //std::cout << "color: " << color << std::endl;
                    }
                    finalColor += color * 0.25;
                    //std::cout << "finalColor: " << finalColor << std::endl;
                }
            }
            //convert linear color to sRGB color
            //std::cout << "finalColor: " << finalColor << std::endl;
            finalColor = clamp(finalColor); //TODO: use tone mapping
            //std::cout << "finalColor: " << finalColor << std::endl;
            finalColor = gammaCorrection(finalColor);
            //std::cout << "finalColor: " << finalColor << std::endl;
            image->SetPixel(x, camera->getHeight()-1-y, finalColor);
            finalColor = Vector3f::ZERO;
        }
        fprintf(stderr,"\rRendering %5.2f%%",100.*y/(image->Height()-1));
    }
    std::cout<<"Rendering finished"<<std::endl;
}

void SPPMRenderer::render(const SceneParser& scene, RgbImage*& image, int samples, int threads, int depth, bool DOF, float aperture, float focalLength) {
    std::cout << "Rendering with Stochastic Progressive Photon Mapping..." << std::endl;
    //initialize main parameters
    Camera *camera = scene.getCamera();
    camera->setDOF(DOF, aperture, focalLength);
    std::cout << "camera: " << camera->getWidth() << " " << camera->getHeight() << std::endl;
    image = new RgbImage(camera->getWidth(), camera->getHeight());
    assert(image!=nullptr);

    omp_set_num_threads(threads);

    //initialize SPPMIntegrator
    //in SPPM, samples means number of photons of every iteration
    //in SPPM, depth means number of iterations 
    SPPMIntegrator sppmIntegrator = SPPMIntegrator(camera->getWidth(), camera->getHeight(), samples, depth);

    sppmIntegrator.render(scene, image);

}



void RayCastingRenderer::render(const SceneParser& scene, RgbImage*& image, int samples, int threads, int depth, bool DOF, float aperture, float focalLength) {
    std::cout << "Rendering with Ray Casting..." << std::endl;
    // 实现光线投射的渲染逻辑
    Camera *camera = scene.getCamera();
    camera->setDOF(false, aperture, focalLength);
    std::cout << "camera: " << camera->getWidth() << " " << camera->getHeight() << std::endl;
    image = new RgbImage(camera->getWidth(), camera->getHeight());
    //DynamicCastTest(image);
    assert(image!=nullptr);
    Group* baseGroup = scene.getGroup();

    //循环屏幕空间的像素
    omp_set_num_threads(threads);
#pragma omp parallel for schedule(dynamic, 1)
    for(int y = 0; y < camera->getHeight(); ++y) {
        fprintf(stderr,"\rRendering %5.2f%%",100.*y/(image->Height()-1)); 
        for(int x = 0; x < camera->getWidth(); ++x) {
        //计算当前像素(x,y)处相机出射光线camRay
        Ray camRay = scene.getCamera()->generateRay(Vector2f(x, y));
        Hit hit ;
        //判断camRay是否和场景有交点，并返回最近交点的数据，存储在hit中
        //fprintf(stderr,"\rRendering pixel (%d,%d)",x,y);
        bool isIntersect = baseGroup->intersect (camRay, hit , 0) ;
        if(isIntersect) {
            Vector3f finalColor = Vector3f ::ZERO;
            //找到交点之后，累加来自所有光源的光强影响
            for(int li = 0; li < scene.getNumLights() ; ++li) {
                Light* light = scene.getLight(li) ;
                Vector3f L, lightColor ;
                //获得光照强度
                light->getIllumination(camRay. pointAtParameter(hit .getT()) , L, lightColor) ;
                //计算局部光强
                if(hit.getMaterial() != nullptr)
                finalColor += hit.getMaterial()->Shade(camRay, hit , L, lightColor, light, depth, baseGroup) ;
            }
            //convert linear color to sRGB color
            finalColor = clamp(finalColor) ;//TODO: use tone mapping | gamma correction
            //hittimes++;
            image->SetPixel(x, camera->getHeight()-1-y, finalColor * 255 ) ;
            //image->SetPixel(x, camera->getHeight()-1-y, Vector3f(255, 0, 0) ) ;
        }else{
            //不存在交点，返回背景色
            image->SetPixel(x, camera->getHeight()-1-y, scene.getBackgroundColor() * 255 ) ;
        }
        }
    }
    std::cout<<"Rendering finished"<<std::endl;
    //std::cout << "hittimes: " << hittimes << std::endl;
}