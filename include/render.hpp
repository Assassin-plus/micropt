#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <iostream>
#include <memory>

#include "utils.hpp"

class Image;
class RgbImage;
class SceneParser;


// 基类：渲染器
class Renderer {
public:
    virtual void render(const SceneParser& scene, RgbImage*& image, int samples, int threads, int depth, bool DOF, float aperture, float focalLength) = 0;
};

class PathTracingRenderer : public Renderer {
public:
    void render(const SceneParser& scene, RgbImage*& image, int samples, int threads, int depth, bool DOF, float aperture, float focalLength) override ;
};

class SPPMRenderer : public Renderer {
public:
    void render(const SceneParser& scene, RgbImage*& image, int samples, int threads, int depth, bool DOF, float aperture, float focalLength) override ;
};


class RayCastingRenderer : public Renderer {
public:
    void render(const SceneParser& scene, RgbImage*& image, int samples, int threads, int depth, bool DOF, float aperture, float focalLength) override ;
};

class RendererFactory {
public:
    std::unique_ptr<Renderer> createRenderer() {
        switch(RENDER){
            case PT:
                return std::make_unique<PathTracingRenderer>();
            case SPPM:
                return std::make_unique<SPPMRenderer>();
            /* case VCM:
                return std::make_unique<VCMRenderer>();
            case BDPT:
                return std::make_unique<BDPTRenderer>();
            case MLT:
                return std::make_unique<MLTRenderer>();
            case VRPT:
                return std::make_unique<VRPTRenderer>(); */
            case RC:
                return std::make_unique<RayCastingRenderer>();
            default:
                return nullptr;
        }
    }
};



#endif // __RENDERER_H__