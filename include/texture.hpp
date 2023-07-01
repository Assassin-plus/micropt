#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <vecmath.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "image.hpp"

class Texture {
    public:
    Texture() = default;

    virtual ~Texture() = default;

    virtual Vector3f getColor(const Vector2f &uv) const = 0;

    virtual Vector3f getNormal(const Vector2f &uv) const = 0;

    //TODO: mipmap Tri-linear interpolation
    //virtual Vector3f getColor(const Vector2f &uv) const = 0;
    

};

class EmpiricalImageTexture : public Texture {
public:
    EmpiricalImageTexture(const std::string &imagePath) {
        // default image: diffuse
        diffuseTexture = loadImage(imagePath);
        ambientTexture = nullptr;
        specularTexture = nullptr;
        specularHighlightTexture = nullptr;
        bumpTexture = nullptr;
        bumpMultiplier = 1;
        alphaTexture = nullptr;
        displacementTexture = nullptr;
    }

    bool loadAmbientTexture(const std::string &imagePath) {
        ambientTexture = loadImage(imagePath);
        return true;
    }

    bool loadDiffuseTexture(const std::string &imagePath) {
        diffuseTexture = loadImage(imagePath);
        return true;
    }

    bool loadSpecularTexture(const std::string &imagePath) {
        //specular texture should be gray
        specularTexture = new GrayImage(imagePath);
        return true;
    }

    bool loadSpecularHighlightTexture(const std::string &imagePath) {
        //specular highlight texture should be gray
        specularHighlightTexture = new GrayImage(imagePath);
        return true;
    }

    bool loadBumpTexture(const std::string &imagePath, const double& bumpMultiplier) {
        bumpTexture = new GrayImage(imagePath);
        this->bumpMultiplier = bumpMultiplier;
        return true;
    }

    bool loadAlphaTexture(const std::string &imagePath) {
        alphaTexture = new GrayImage(imagePath);
        return true;
    }

    bool loadDisplacementTexture(const std::string &imagePath) { 
        displacementTexture = new GrayImage(imagePath);
        //TODO: edit mesh in obj file?
        return true;
    }

    Vector3f getColor(const Vector2f &uv) const override {
        //std::cout<<"get color: "<<uv.x()<<" "<<uv.y()<<std::endl;
        
        //bilinear interpolation
        RgbImage* img = static_cast<RgbImage*>(diffuseTexture);
        float u = uv.x() * img->Width() - 0.5;
        float v = (1 - uv.y()) * img->Height() - 0.5;

        Vector2f uv00 = mapToImage(Vector2f(floor(u), floor(v)), img->Width(), img->Height());
        Vector2f uv01 = mapToImage(Vector2f(floor(u), ceil(v)), img->Width(), img->Height());
        Vector2f uv10 = mapToImage(Vector2f(ceil(u), floor(v)), img->Width(), img->Height());
        Vector2f uv11 = mapToImage(Vector2f(ceil(u), ceil(v)), img->Width(), img->Height());
        Vector3f color00 = img->GetPixel(uv00.x(), uv00.y());
        Vector3f color01 = img->GetPixel(uv01.x(), uv01.y());
        Vector3f color10 = img->GetPixel(uv10.x(), uv10.y());
        Vector3f color11 = img->GetPixel(uv11.x(), uv11.y());
        Vector3f color0 = lerp(color00, color01, v - floor(v));
        Vector3f color1 = lerp(color10, color11, v - floor(v));
        Vector3f color = lerp(color0, color1, u - floor(u));
        return color/255.0;
        
        /* int x = uv.x() * diffuseTexture->Width();
        int y = (1 - uv.y()) * diffuseTexture->Height();
        while(x < 0) x += diffuseTexture->Width();
        while(y < 0) y += diffuseTexture->Height();
        x %= diffuseTexture->Width();
        y %= diffuseTexture->Height();
        return static_cast<RgbImage*>(diffuseTexture)->GetPixel(x, y)/255.0; */
    }

    float getSpecular(const Vector2f &uv) const {
        int x = uv.x() * specularTexture->Width();
        int y = (1 - uv.y()) * specularTexture->Height();
        while(x < 0) x += specularTexture->Width();
        while(y < 0) y += specularTexture->Height();
        x %= specularTexture->Width();
        y %= specularTexture->Height();
        if(dynamic_cast<RgbImage*>(specularTexture)) {
            return static_cast<RgbImage*>(specularTexture)->GetPixel(x, y).x()/255.0;
        } else {
            return static_cast<GrayImage*>(specularTexture)->GetPixel(x, y)/255.0;
        }
    }

    Vector3f getNormal(const Vector2f &uv) const override {
        //throughput
        return Vector3f::ZERO;
    }
    std::pair<float, float> getBump(const Vector2f &uv) {
        GrayImage* img = dynamic_cast<GrayImage*>(bumpTexture);
        if(!img){
            //std::cout<<"bump texture is not gray image"<<std::endl;
            return std::make_pair(0, 0);
        }
        float u = uv.x() * img->Width() - 0.5;
        float v = (1 - uv.y()) * img->Height() - 0.5;

        Vector2f uv00 = mapToImage(Vector2f(floor(u), floor(v)), img->Width(), img->Height());
        Vector2f uv01 = mapToImage(Vector2f(floor(u), ceil(v)), img->Width(), img->Height());
        Vector2f uv10 = mapToImage(Vector2f(ceil(u), floor(v)), img->Width(), img->Height());
        Vector2f uv11 = mapToImage(Vector2f(ceil(u), ceil(v)), img->Width(), img->Height());
        float bump00 = img->GetPixel(uv00.x(), uv00.y());
        float bump01 = img->GetPixel(uv01.x(), uv01.y());
        float bump10 = img->GetPixel(uv10.x(), uv10.y());
        float bump11 = img->GetPixel(uv11.x(), uv11.y());
        float bump0 = bump00 * (1 - (v - floor(v))) + bump01 * (v - floor(v));
        float bump1 = bump10 * (1 - (v - floor(v))) + bump11 * (v - floor(v));
        float bump = bump0 * (1 - (u - floor(u))) + bump1 * (u - floor(u));
        
        return std::make_pair(bump, bumpMultiplier);
    }
    bool hasSpecular() const { return specularTexture != nullptr; }

private:
    Image* loadImage(const std::string &imagePath) {
        std::cout<<"load image: "<<imagePath<<std::endl;
        Image* image = new RgbImage(imagePath);
        assert(image!=nullptr);
        return image;
    }
    Image* ambientTexture;
    Image* diffuseTexture;
    Image* specularTexture;
    Image* specularHighlightTexture;
    Image* bumpTexture;
    double bumpMultiplier;
    Image* alphaTexture;
    Image* displacementTexture;
};

class PBRTexture : public Texture {
public:
    PBRTexture() = delete;
    PBRTexture(const std::string &imagePath) {
        //default image: roughness
        roughnessTexture = new GrayImage(imagePath);
        metallicTexture = nullptr;
        sheenTexture = nullptr;
        normalTexture = nullptr;
        emissiveTexture = nullptr;
    }
    bool loadEmissiveTexture(const std::string &imagePath) {
        emissiveTexture = new RgbImage(imagePath);
        return true;
    }
    bool loadRoughnessTexture(const std::string &imagePath) {
        roughnessTexture = new GrayImage(imagePath);
        return true;
    }
    bool loadMetallicTexture(const std::string &imagePath) {
        metallicTexture = new GrayImage(imagePath);
        return true;
    }
    bool loadSheenTexture(const std::string &imagePath) {
        sheenTexture = new GrayImage(imagePath);
        return true;
    }
    bool loadNormalTexture(const std::string &imagePath) {
        normalTexture = new RgbImage(imagePath);
        return true;
    }

    Vector3f getColor(const Vector2f &uv) const override {
        //compute from all textures
        return Vector3f(0.5,0.5,0.5);
    }

    Vector3f getNormal(const Vector2f &uv) const override {
        //TODO: normal map
        int x = uv.x() * normalTexture->Width();
        int y = (1 - uv.y()) * normalTexture->Height();
        while(x < 0) x += normalTexture->Width();
        while(y < 0) y += normalTexture->Height();
        x%=normalTexture->Width();
        y%=normalTexture->Height();
        
        GrayImage* img = dynamic_cast<GrayImage*>(normalTexture);
        if(!img){
            std::cout<<"bump texture is not gray image"<<std::endl;
            return Vector3f::ZERO;
        }
        int x1 = (x + 1) % normalTexture->Width();
        int y1 = (y + 1) % normalTexture->Height();
        int x_ = (x - 1 + normalTexture->Width()) % normalTexture->Width();
        int y_ = (y - 1 + normalTexture->Height()) % normalTexture->Height();
        
        float dx = img->GetPixel(x1, y) - img->GetPixel(x_, y);
        float dy = img->GetPixel(x, y1) - img->GetPixel(x, y_);
        return Vector3f(dx, dy, 1);
    }

private:
    Image* emissiveTexture;
    Image* roughnessTexture;
    Image* metallicTexture;
    Image* sheenTexture;
    Image* normalTexture;
};

class ProceduralTexture : public Texture {
public:
    ProceduralTexture() = default;

    Vector3f getColor(const Vector2f &uv) const override {
        return Vector3f(0.5,0.5,0.5);
    }

    Vector3f getNormal(const Vector2f &uv) const override {
        return Vector3f::UP;
    }

    float getRoughness(const Vector2f &uv) const {
        return 0;
    }

    float getMetallic(const Vector2f &uv) const {
        return 0;
    }

    float getAmbientOcclusion(const Vector2f &uv) const {
        return 0;
    }
    private:
    // 实现省略
};

#endif //__TEXTURE_H__