#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <string>
#include <vecmath.h>
#include "ray.hpp"
#include "hit.hpp"
#include "utils.hpp"

#include <iostream>

class Texture;
class Light;
class Group;

class Material {
public:
    virtual Vector3f Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const = 0;
    virtual bool hasTexture() = 0;
    virtual BRDFType getMaterialType() const = 0;
    virtual Vector3f getDiffuseColor() const = 0;
    std::string name;
};

class GouraudMaterial : public Material {
public:
    explicit GouraudMaterial(const Vector3f& d_color) : diffuseColor(d_color) {

    }

    virtual ~GouraudMaterial() = default;

    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }

    virtual bool hasTexture() {
        return false;
    }

    virtual BRDFType getMaterialType() const {
        return BRDFType::DIFFUSE;
    }

    Vector3f Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const override;
private:
    Vector3f diffuseColor;
};

class LambertianMaterial : public Material {
public:
    explicit LambertianMaterial(const Vector3f& d_color) : diffuseColor(d_color) {

    }

    virtual ~LambertianMaterial() = default;

    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }

    virtual bool hasTexture() {
        return false;
    }

    virtual BRDFType getMaterialType() const {
        return BRDFType::DIFFUSE;
    }

    Vector3f Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const override;
private:
    Vector3f diffuseColor;  
};

class PhongMaterial : public Material {
public:
    PhongMaterial(const Vector3f& d_color, const Vector3f& s_color, float sh) :
            diffuseColor(d_color), specularColor(s_color), shininess(sh) {

    }
    Vector3f Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const override ;
    virtual bool hasTexture() {
        return false;
    }
    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }
    virtual BRDFType getMaterialType() const {
        return BRDFType::DIFFUSE;
    }
private:
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
};

class DiscreteMaterial : public Material {
public:
    DiscreteMaterial(const Vector3f& d_color, const Vector3f& s_color, const Vector3f& e_color, BRDFType type) :
            diffuseColor(d_color), specularColor(s_color), emissionColor(e_color), type(type) {  
                
              }
    ~DiscreteMaterial() = default;
    Vector3f Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const override{
        return Vector3f::ZERO;
    }
    virtual bool hasTexture() {
        return false;
    }
    void setDiffuseColor(const Vector3f& d_color) {
        diffuseColor = d_color;
    }
    void setSpecularColor(const Vector3f& s_color) {
        specularColor = s_color;
    }
    void setEmissionColor(const Vector3f& e_color) {
        emissionColor = e_color;
    }
    Vector3f getDiffuseColor() const {
        return diffuseColor;
    }
    Vector3f getSpecularColor() const {
        return specularColor;
    }
    Vector3f getEmissionColor() const {
        return emissionColor;
    }
    virtual BRDFType getMaterialType() const {
        return type;
    }
private:
    Vector3f diffuseColor;
    Vector3f specularColor;
    Vector3f emissionColor;
    BRDFType type;
};

class EmpiricalMaterial : public Material {
public:
    EmpiricalMaterial(const Vector3f& a_color, const Vector3f& d_color, const Vector3f& s_color, const Vector3f& t_color, const Vector3f& e_color, double sh, double ior, double dis, int il) :
            ambientColor(a_color), diffuseColor(d_color), specularColor(s_color), transmittance(t_color), emissionColor(e_color), shininess(sh), IOR(ior), dissolve(dis), illum(il), _texture(nullptr) {

    }

    virtual ~EmpiricalMaterial() = default;
    void setTexture(Texture* texture) {
        _texture = texture;
    }
    virtual bool hasTexture() {
        return _texture != nullptr;
    }
    std::pair<float, float> getBump(const Vector2f& texCoord) const;
    Vector3f getDiffuseColor() const {
        return diffuseColor;
    }
    Vector3f getDiffuseColor(const Vector2f& texCoord) const;
    Vector3f getAmbientColor() const {
        return ambientColor;
    }
    Vector3f getSpecularColor() const {
        return specularColor;
    }
    Vector3f getSpecularColor(const Vector2f& texCoord) const;
    Vector3f getTransmittance() const {
        return transmittance;
    }
    Vector3f getEmissionColor() const {
        return emissionColor;
    }
    double getShininess() const {
        return shininess;
    }
    double getIOR() const {
        return IOR;
    }
    double getDissolve() const {
        return dissolve;
    }
    BRDFType getMaterialType() const {
        unsigned short Xi[3] = {0, 0, illum * illum * illum};
        double p = erand48(Xi);
        
        //using dissolve to control the probability of each type
        if(p < dissolve) {
            return BRDFType::REFRACTION;
        } else {
            return BRDFType::MICROFACET;
        }
    }
    Vector3f sampleBRDF(const Vector3f& dirToView, const Vector3f& normal) const;
    Vector3f evalBRDF(const Vector3f& dirToLight, const Vector3f& dirToView, const Vector3f& normal) const;
    Vector3f Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const override;
private:
    Vector3f ambientColor;
    Vector3f diffuseColor;
    Vector3f specularColor;
    Vector3f transmittance;
    Vector3f emissionColor;
    double shininess;
    double IOR = 1.0;
    double dissolve = 0.0;
    int illum;
    Texture* _texture;
};


class PBRMaterial : public Material {
public:
    PBRMaterial() = delete;
    PBRMaterial(const PBRMaterial& other) {
        roughness = other.roughness;
        metallic = other.metallic;
        sheen = other.sheen;
        clearcoatThick = other.clearcoatThick;
        clearcoatRough = other.clearcoatRough;
        anisotropic = other.anisotropic;
        anisotropicRotation = other.anisotropicRotation;
        texture = other.texture;
    }

    PBRMaterial(double roughness, double metallic, double sheen, double clearcoatThick, double clearcoatRough, double anisotropic, double anisotropicRotation) :
            roughness(roughness), metallic(metallic), sheen(sheen), clearcoatThick(clearcoatThick), clearcoatRough(clearcoatRough), anisotropic(anisotropic), anisotropicRotation(anisotropicRotation), texture(nullptr) {
    }

    virtual ~PBRMaterial() = default;
    virtual bool hasTexture() {
        return texture != nullptr;
    }
    virtual Vector3f getDiffuseColor() const {
        return Vector3f::ZERO;
    }
    virtual BRDFType getMaterialType() const {
        return BRDFType::DIFFUSE;
    }
    Vector3f Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const override;
private:
    double roughness;
    double metallic;
    double sheen;
    double clearcoatThick;
    double clearcoatRough;
    double anisotropic;
    double anisotropicRotation;
    Texture* texture;
};

//===================================================================================================================
//Material used in original framework

class TraditionalMaterial : public Material {
public:

    explicit TraditionalMaterial(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0) :
            diffuseColor(d_color), specularColor(s_color), shininess(s) {

    }

    virtual ~TraditionalMaterial() = default;

    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }
    Vector3f getSpecularColor() const {
        return specularColor;
    }
    float getShininess() const {
        return shininess;
    }
    virtual bool hasTexture() {
        return false;
    }
    virtual BRDFType getMaterialType() const {
        return BRDFType::DIFFUSE;
    }

    Vector3f Shade(const Ray &ray, const Hit &hit,const Vector3f &dirToLight, const Vector3f &lightColor, Light* light, int depth, Group* baseGroup) const override {
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

protected:
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
};


#endif // MATERIAL_H
