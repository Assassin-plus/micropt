#include <cassert>
#include <iostream>
#include <vecmath.h>
#include "../include/material.hpp"
#include "../include/texture.hpp"
#include "../include/ray.hpp"
#include "../include/hit.hpp"
#include "../include/light.hpp"
#include "../include/group.hpp"
#include "../include/utils.hpp"

Vector3f GouraudMaterial::Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const {
    Vector3f shadedColor = Vector3f::ZERO;
    Vector3f normal = hit.getNormal();
    float LN = Vector3f::dot(normal, dirToLight);
    if (LN > 0) {
        shadedColor += lightColor * diffuseColor * LN;
    }
    return shadedColor;
}

Vector3f LambertianMaterial::Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const {
    Vector3f shadedColor = Vector3f::ZERO;
    Vector3f normal = hit.getNormal();
    float LN = Vector3f::dot(normal, dirToLight);
    if (LN > 0) {
        shadedColor += lightColor * diffuseColor * LN;
    }
    return shadedColor;
}

Vector3f PhongMaterial::Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const {
    Vector3f shadedColor = Vector3f::ZERO;
    Vector3f normal = hit.getNormal();
    Vector3f view = -ray.getDirection();
    Vector3f reflect = 2 * Vector3f::dot(dirToLight, normal) * normal - dirToLight;
    reflect.normalize();
    float LN = Vector3f::dot(normal, dirToLight);
    float RV = Vector3f::dot(reflect, view);
    if (LN > 0) {
        shadedColor += lightColor * diffuseColor * LN;
    }
    if (RV > 0) {
        shadedColor += lightColor * specularColor * pow(RV, shininess);
    }
    return shadedColor;
}

Vector3f EmpiricalMaterial::getDiffuseColor(const Vector2f& texCoord) const {
    if(_texture == nullptr) {
        return diffuseColor;
    } else {
        return _texture->getColor(texCoord);
    }
}

std::pair<float, float> EmpiricalMaterial::getBump(const Vector2f& texCoord) const {
    if(_texture == nullptr) {
        return std::make_pair(0.0f, 0.0f);
    } else {
        EmpiricalImageTexture* _t = dynamic_cast<EmpiricalImageTexture*>(_texture);
        assert(_t != nullptr);
        return _t->getBump(texCoord);
    }
}

Vector3f EmpiricalMaterial::getSpecularColor(const Vector2f& texCoord) const {
    if(_texture == nullptr) {
        return specularColor;
    } else {
        EmpiricalImageTexture* _t = dynamic_cast<EmpiricalImageTexture*>(_texture);
        assert(_t != nullptr);
        return _t->getSpecular(texCoord);
    }
}

Vector3f EmpiricalMaterial::sampleBRDF(const Vector3f& dirToView, const Vector3f& normal) const{
    unsigned short Xi[3] = {0, 0, 8192};
    double r1 = erand48(Xi);
    //decide diffuse or specular
    double diffuseRatio = diffuseColor.length() / (diffuseColor.length() + specularColor.length());
    if(r1 < diffuseRatio){
        //diffuse
        double r2 = erand48(Xi);
        double r3 = erand48(Xi);double r3s = sqrt(r3);
        double theta = 2 * M_PI * r2;
        double x = r3s * cos(theta);
        double y = r3s * sin(theta);
        double z = sqrt(1 - r3);
        //generate orthonormal basis (w, u, v) according to normal
        Vector3f w = normal, u = (Vector3f::cross((fabs(w.x()) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0)),w)).normalized(), v = Vector3f::cross(w,u);
        //generate random reflection ray direction
        Vector3f dir = (u * x + v * y + w * z).normalized();
        return dir;
    }else{
        //specular sample according to microfacet distribution
        //current: Beckmann distribution
        //theta_h = atan(sqrt(-alpha^2 * log(1 - r1)))
        //phi_h = 2 * pi * r2
        //w_h = (sin(theta_h) * cos(phi_h), sin(theta_h) * sin(phi_h), cos(theta_h))
        //w_o = reflect(-w_i, w_h) = 2 * dot(w_i, w_h) * w_h - w_i
        double r2 = erand48(Xi);
        double r3 = erand48(Xi);
        double theta_h = atan(sqrt(-pow(shininess, 2) * log(1 - r1)));
        double phi_h = 2 * M_PI * r2;
        double x = sin(theta_h) * cos(phi_h);
        double y = sin(theta_h) * sin(phi_h);
        double z = cos(theta_h);
        Vector3f w_h = Vector3f(x, y, z);
        Vector3f w_o = 2 * Vector3f::dot(dirToView, w_h) * w_h - dirToView;
        return w_o;
    }
}


Vector3f EmpiricalMaterial::evalBRDF(const Vector3f& dirToLight, const Vector3f& dirToView, const Vector3f& normal) const{
    Vector3f w_i = dirToLight;
    Vector3f w_o = dirToView;
    Vector3f w_h = (w_i + w_o).normalized();
    //Diffuse term
    Vector3f diffuse = diffuseColor / M_PI;
    //Specular term
    float ks = 1 - std::max(diffuseColor.x(), std::max(diffuseColor.y(), diffuseColor.z()));
    double theta_h = acos(Vector3f::dot(w_h, normal));
    double theta_i = acos(Vector3f::dot(w_i, normal));
    double theta_o = acos(Vector3f::dot(w_o, normal));
    //D(w_h), Beckmann distribution
    double alpha = pow(shininess, 2);
    double D = exp(-pow(tan(theta_h), 2) / alpha) / (M_PI * alpha * pow(cos(theta_h), 4));
    //F(w_h * w_i) , Fresnel term, use Schlick approximation
    double F0 = pow((IOR - 1) / (IOR + 1), 2);
    double F = F0 + (1 - F0) * pow(1 - Vector3f::dot(w_h, w_i), 5);
    //G(w_h, w_i, w_o), Smith's shadowing and masking function
    //compute G1(w_i, w_h)
    double b_i = 1 / (alpha * pow(tan(theta_i), 2));
    double G1_i;
    if(b_i < 1.6){
        G1_i = (3.535 * b_i + 2.181 * pow(b_i, 2)) / (1 + 2.276 * b_i + 2.577 * pow(b_i, 2));
    }else{
        G1_i = 1;
    }
    if(Vector3f::dot(w_i, w_h) * Vector3f::dot(w_i, normal) <= 0){
        G1_i = 0;
    }
    //compute G1(w_o, w_h)
    double b_o = 1 / (alpha * pow(tan(theta_o), 2));
    double G1_o;
    if(b_o < 1.6){
        G1_o = (3.535 * b_o + 2.181 * pow(b_o, 2)) / (1 + 2.276 * b_o + 2.577 * pow(b_o, 2));
    }else{
        G1_o = 1;
    }
    if(Vector3f::dot(w_o, w_h) * Vector3f::dot(w_o, normal) <= 0){
        G1_o = 0;
    }
    double G = G1_i * G1_o;

    Vector3f specular = ks * D * F * G / (4 * Vector3f::dot(w_i, normal) * Vector3f::dot(w_o, normal));
    return diffuse + specular;
}

Vector3f EmpiricalMaterial::Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const {
    
    switch(illum){
        case 0:// constant color illumination model
            {
                return _texture == nullptr ? diffuseColor : _texture->getColor(hit.getTexCoord());
            }
        case 1:// Lambertian shading
            {
                Vector3f shadedColor = Vector3f::ZERO;
                if( dynamic_cast<AmbientLight*>(light) != nullptr ){
                    shadedColor += lightColor * ambientColor;
                    return shadedColor;
                }
        
                Vector3f normal = hit.getNormal();
                float LN = Vector3f::dot(normal, dirToLight);
                
                if (LN > 0) {
                    if(_texture != nullptr){
                        shadedColor += lightColor * _texture->getColor(hit.getTexCoord()) * LN;
                    }else{
                        shadedColor += lightColor * diffuseColor * LN;
                    }
                }
                return shadedColor;
            }
        case 2://Blinn-Phong shading
            {
                Vector3f shadedColor = Vector3f::ZERO;
                if( dynamic_cast<AmbientLight*>(light) != nullptr ){
                    shadedColor += lightColor * ambientColor;
                    return shadedColor;
                }

                Vector3f normal = hit.getNormal();
                //use bisector instead of reflect
                Vector3f bisector = dirToLight + -ray.getDirection();
                bisector.normalize();

                float LN = Vector3f::dot(normal, dirToLight);
                if (LN > 0) {
                    if(_texture != nullptr){
                        shadedColor += lightColor * _texture->getColor(hit.getTexCoord()) * LN;
                    }else{
                        shadedColor += lightColor * diffuseColor * LN;
                    }
                }

                if(shininess > 0){
                    float BN = Vector3f::dot(normal, bisector);
                    if(BN > 0){
                        if(dynamic_cast<EmpiricalImageTexture*>(_texture) ->hasSpecular()){
                            shadedColor += lightColor * dynamic_cast<EmpiricalImageTexture*>(_texture)->getSpecular(hit.getTexCoord()) * pow(BN, shininess);
                        }else{
                            shadedColor += lightColor * specularColor * pow(BN, shininess);
                        }
                    }
                }
                return shadedColor;
            }
        case 3://Whitted illumination model, Reflection on and Ray trace on 
            {
                Vector3f shadedColor = Vector3f::ZERO;
                if( dynamic_cast<AmbientLight*>(light) != nullptr ){
                    shadedColor += lightColor * ambientColor;
                    return shadedColor;
                }

                Vector3f normal = hit.getNormal();
                //use bisector instead of reflect
                Vector3f bisector = dirToLight + -ray.getDirection();
                bisector.normalize();

                float LN = Vector3f::dot(normal, dirToLight);
                if (LN > 0) {
                    if(_texture != nullptr){
                        shadedColor += lightColor * _texture->getColor(hit.getTexCoord()) * LN;
                    }else{
                        shadedColor += lightColor * diffuseColor * LN;
                    }
                }

                if(shininess > 0){
                    float BN = Vector3f::dot(normal, bisector);
                    if(BN > 0){
                        if(dynamic_cast<EmpiricalImageTexture*>(_texture) ->hasSpecular()){
                            shadedColor += lightColor * dynamic_cast<EmpiricalImageTexture*>(_texture)->getSpecular(hit.getTexCoord()) * pow(BN, shininess);
                        }else{
                            shadedColor += lightColor * specularColor * pow(BN, shininess);
                        }
                    }
                }
                //a reflection term similar to Whitted illumination model
                //this is a recursive call
                if(specularColor != Vector3f::ZERO && depth < 5){
                    Vector3f reflect = 2 * Vector3f::dot(dirToLight, normal) * normal - dirToLight;
                    reflect.normalize();
                    Vector3f intersectPoint = ray.pointAtParameter(hit.getT());
                    Ray reflectRay = Ray(intersectPoint, reflect);
                    Hit reflectHit = Hit();
                    Vector3f reflectL, reflectLightColor;
                    
                    if(baseGroup->intersect(reflectRay, reflectHit, 0.0001)){
                        light->getIllumination(reflectRay.pointAtParameter(reflectHit.getT()), reflectL, reflectLightColor);
                        
                        Vector3f reflectColor = reflectHit.getMaterial()->Shade(reflectRay, reflectHit, dirToLight, lightColor, light, depth + 1, baseGroup);
                        shadedColor += reflectColor * specularColor;
                    }
                }
                return shadedColor;
            }
        case 4://Transparency: Glass on
            {
                Vector3f shadedColor = Vector3f::ZERO;
                if( dynamic_cast<AmbientLight*>(light) != nullptr ){
                    shadedColor += lightColor * ambientColor;
                    return shadedColor;
                }

                Vector3f normal = hit.getNormal();
                //use bisector instead of reflect
                Vector3f bisector = dirToLight + -ray.getDirection();
                bisector.normalize();

                float LN = Vector3f::dot(normal, dirToLight);
                if (LN > 0) {
                    if(_texture != nullptr){
                        shadedColor += lightColor * _texture->getColor(hit.getTexCoord()) * LN;
                    }else{
                        shadedColor += lightColor * diffuseColor * LN;
                    }
                }

                if(shininess > 0){
                    float BN = Vector3f::dot(normal, bisector);
                    if(BN > 0){
                        EmpiricalImageTexture* texture = dynamic_cast<EmpiricalImageTexture*>(_texture);
                        if(texture != nullptr && texture->hasSpecular()){
                            shadedColor += lightColor * texture->getSpecular(hit.getTexCoord()) * pow(BN, shininess);
                        }else{
                            shadedColor += lightColor * specularColor * pow(BN, shininess);
                        }
                        if(specularColor != Vector3f::ZERO && depth < 5){
                            Vector3f reflect = 2 * Vector3f::dot(dirToLight, normal) * normal - dirToLight;
                            reflect.normalize();
                            Vector3f intersectPoint = ray.pointAtParameter(hit.getT());
                            Ray reflectRay = Ray(intersectPoint, reflect);
                            Hit reflectHit = Hit();
                            Vector3f reflectL, reflectLightColor;
                            
                            if(baseGroup->intersect(reflectRay, reflectHit, 0.0001)){
                                light->getIllumination(reflectRay.pointAtParameter(reflectHit.getT()), reflectL, reflectLightColor);

                                Vector3f reflectColor = reflectHit.getMaterial()->Shade(reflectRay, reflectHit, dirToLight, lightColor, light, depth + 1, baseGroup);
                                shadedColor += reflectColor * specularColor;
                            }
                        }
                    }
                }
                
                return shadedColor;
            }
        case 5://Fresnel reflection
            {
                //TODO
                /*This is a diffuse and specular shading models similar to 
                illumination model 3, except that reflection due to Fresnel effects is 
                introduced into the equation.  Fresnel reflection results from light 
                striking a diffuse surface at a grazing or glancing angle.  When light 
                reflects at a grazing angle, the Ks value approaches 1.0 for all color 
                samples*/
                Vector3f shadedColor = Vector3f::ZERO;
                if( dynamic_cast<AmbientLight*>(light) != nullptr ){
                    shadedColor += lightColor * ambientColor;
                    return shadedColor;
                }

                Vector3f normal = hit.getNormal();
                //use bisector instead of reflect
                Vector3f H = dirToLight + -ray.getDirection();
                H.normalize();

                float LN = Vector3f::dot(normal, dirToLight);
                if (LN > 0) {
                    if(_texture != nullptr){
                        shadedColor += lightColor * _texture->getColor(hit.getTexCoord()) * LN;
                    }else{
                        shadedColor += lightColor * diffuseColor * LN;
                    }
                }

                //Fresnel reflection
                //color += Ks( (H*Hj)^shininess ) * lightColor * Fresnel( Lj*Hj, Ks, shininess) 
                //+ Fresnel( N*V, Ks, shininess) * lightColor * Ks
                if(shininess > 0){
                    float HN = Vector3f::dot(H, normal);
                    if(HN > 0){
                        if(dynamic_cast<EmpiricalImageTexture*>(_texture) ->hasSpecular()){
                            shadedColor += lightColor * dynamic_cast<EmpiricalImageTexture*>(_texture)->getSpecular(hit.getTexCoord()) * pow(HN, shininess);
                        }else{
                            shadedColor += lightColor * specularColor * pow(HN, shininess);
                        }
                        if(specularColor != Vector3f::ZERO && depth < 5){
                            Vector3f reflect = 2 * Vector3f::dot(dirToLight, normal) * normal - dirToLight;
                            reflect.normalize();
                            Vector3f intersectPoint = ray.pointAtParameter(hit.getT());
                            Ray reflectRay = Ray(intersectPoint, reflect);
                            Hit reflectHit = Hit();
                            Vector3f reflectL, reflectLightColor;
                            
                            if(baseGroup->intersect(reflectRay, reflectHit, 0.0001)){
                                light->getIllumination(reflectRay.pointAtParameter(reflectHit.getT()), reflectL, reflectLightColor);

                                Vector3f reflectColor = reflectHit.getMaterial()->Shade(reflectRay, reflectHit, dirToLight, lightColor, light, depth + 1, baseGroup);
                                shadedColor += reflectColor * specularColor;
                            }
                        }
                    }
                }
                //a reflection term similar to Whitted illumination model
                //this is a recursive call
                
                return shadedColor;
            }
        case 6:
            {
                /*color = KaIa
                        + Kd { SUM j=1..ls, (N*Lj)Ij }
                        + Ks ({ SUM j=1..ls, ((H*Hj)^Ns)Ij } + Ir)
                        + (1.0 - Ks) TfIt*/
            }
        case 7:
            {
                /*color = KaIa
                        + Kd { SUM j=1..ls, (N*Lj)Ij }
                        + Ks ({ SUM j=1..ls, ((H*Hj)^Ns)Ij Fr(Lj*Hj,Ks,Ns)Ij} + 
                    Fr(N*V,Ks,Ns)Ir})
                    
                        + (1.0 - Kx)Ft (N*V,(1.0-Ks),Ns)TfIt
                */
            }
        case 8:
            {
                Vector3f shadedColor = Vector3f::ZERO;
                if( dynamic_cast<AmbientLight*>(light) != nullptr ){
                    shadedColor += lightColor * ambientColor;
                    return shadedColor;
                }

                Vector3f normal = hit.getNormal();
                //use bisector instead of reflect
                Vector3f bisector = dirToLight + -ray.getDirection();
                bisector.normalize();

                float LN = Vector3f::dot(normal, dirToLight);
                if (LN > 0) {
                    if(_texture != nullptr){
                        shadedColor += lightColor * _texture->getColor(hit.getTexCoord()) * LN;
                    }else{
                        shadedColor += lightColor * diffuseColor * LN;
                    }
                }

                if(shininess > 0){
                    float BN = Vector3f::dot(normal, bisector);
                    if(BN > 0){
                        if(dynamic_cast<EmpiricalImageTexture*>(_texture) ->hasSpecular()){
                            shadedColor += lightColor * dynamic_cast<EmpiricalImageTexture*>(_texture)->getSpecular(hit.getTexCoord()) * pow(BN, shininess);
                        }else{
                            shadedColor += lightColor * specularColor * pow(BN, shininess);
                        }
                    }
                }
                //a reflection term similar to Whitted illumination model
                //this is a recursive call
                //use reflaction map
                //currently not supported, skip
                return shadedColor;
            }
        case 9:
            {
                Vector3f shadedColor = Vector3f::ZERO;
                if( dynamic_cast<AmbientLight*>(light) != nullptr ){
                    shadedColor += lightColor * ambientColor;
                    return shadedColor;
                }

                Vector3f normal = hit.getNormal();
                //use bisector instead of reflect
                Vector3f bisector = dirToLight + -ray.getDirection();
                bisector.normalize();

                float LN = Vector3f::dot(normal, dirToLight);
                if (LN > 0) {
                    if(_texture != nullptr){
                        shadedColor += lightColor * _texture->getColor(hit.getTexCoord()) * LN;
                    }else{
                        shadedColor += lightColor * diffuseColor * LN;
                    }
                }

                if(shininess > 0){
                    float BN = Vector3f::dot(normal, bisector);
                    if(BN > 0){
                        if(dynamic_cast<EmpiricalImageTexture*>(_texture) ->hasSpecular()){
                            shadedColor += lightColor * dynamic_cast<EmpiricalImageTexture*>(_texture)->getSpecular(hit.getTexCoord()) * pow(BN, shininess);
                        }else{
                            shadedColor += lightColor * specularColor * pow(BN, shininess);
                        }
                    }
                }
                //a reflection term similar to Whitted illumination model
                //this is a recursive call
                //use reflaction map
                //currently not supported, skip
                return shadedColor;
            }
        case 10://TODO: add shadow rendering
            {

            }
        default:
            std::cout<<"illumination model not implemented"<<std::endl;
            return Vector3f::ZERO;
    }

}

Vector3f PBRMaterial::Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const {
    std::cout<<"PBRMaterial::Shade not implemented"<<std::endl;
    return Vector3f::ZERO;
}