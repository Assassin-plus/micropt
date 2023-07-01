#ifndef SPPM_HPP
#define SPPM_HPP
//follow the style of PBRT-v4 naming
#include "utils.hpp"
#include "light.hpp"
#include <vecmath.h>
#include <vector>

class Material;
class Group;
class RgbImage;
class SceneParser;
class SPPMIntegrator {
public:
    SPPMIntegrator() = default;
    SPPMIntegrator(const int& width, const int& height, const int &photons, const int &iterations, const float &radius = 5.0f, const float &alpha = 0.7f) : photonCount(photons), iteration(iterations), sharedRadius(radius), alpha(alpha) {
        photonTotal = photonCount * iteration;
        PixelMap.reserve(width * height);
        for(auto& pixel : PixelMap) {
            pixel = SPPMPixel();
        }
        photonMap = std::make_unique<KDTree<Photon>>();
        photonMap->reserve(photonCount);
    }

    void render(const SceneParser& scene, RgbImage *&image);

private:
    int photonCount;                   //单pass有效光子数
    int photonTotal;                   //全部发射光子数
    int iteration;                     //pass次数
    float sharedRadius;                     //初始半径
    float alpha;                            //衰减系数
    std::vector<SPPMPixel> PixelMap;        //像素map
    std::unique_ptr<KDTree<Photon>> photonMap; //光子map
};




#endif //SPPM_HPP