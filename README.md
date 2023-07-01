# How to use
```bash
mkdir build
cd build
cmake ..
make -j8
./SPPM --input ../testcases/testcasename.txt --output ../output/testcasename.png --samples 1000 --threads 8 --rendermode 1 --dof 1 --depth-of-field 1 --aperture 3 --focus-length 250 --depth 5
```
# Points

分布式路径追踪: Distributed Path Tracing

* 反射
* 折射
* 纹理
  * 纹理贴图
  * 高光贴图
* 三角网格：obj文件读取
  * wavefront mtl材质支持
  * BVH加速
  * PBR材质支持
* 光源
  * 点光源
  * 面光源采样
  * > 重要性采样,包括cosine采样与BRDF重要性采样
    >
  * 软阴影
  * 抗锯齿
  * 景深
* 参数曲面解析法求交和渲染

SPPM

* 焦散
* > 因为渲染时间不足，所以只使用了简单球体的焦散
  >

# Results

![1687685267763](image/report/1687685267763.png)

Bspline旋转参数曲面

![1687685419996](image/report/1687685419996.png)![1687685441703](image/report/1687685441703.png)![1687685600173](image/report/1687685600173.png)反射、折射、软阴影、抗锯齿、景深、Color Bleeding

![1687685634737](image/report/1687685634737.png)

obj，纹理贴图、法线插值

![1687685759159](image/report/1687685759159.png)

复杂网格曲面、纹理贴图、高光贴图


SPPM:焦散

# Structure & Honor Code

> This project is developed based on the framework provided by the course. Mainly, I have implemented the following parts:

## Codes developed by myself

### bvh.hpp

BVH加速, 用于加速三角网格的求交

> 利用Bounding Box和递归的思想, 将三角网格分割成一系列层级的Bounding Box, 从而加速求交

### camera.hpp

相机, 用于生成光线

> 基于PA1提供的透视投影的相机，实现了景深
> 代码如下：

```cpp
randomly sample a point on the lens
//Generate ray from lens point
Vector3f rayOrigin = center + x * horizontal + y * up;
Vector3f rayDir = Vector3f((point.x() - cx) / fx, (point.y() - cy) / fy, 1).normalized();
// transform the ray to world space
Matrix3f R(horizontal, up, direction);
rayDir = R * rayDir;
//Compute focal point
Vector3f focalPoint = center + focalLength * rayDir;
rayDir = (focalPoint - rayOrigin).normalized();
return Ray(rayOrigin, rayDir);
```

### classical_object.hpp

经典物体，包括球体和平面，复用了PA1的本人代码

### curve.hpp

参数曲面，用于求交和渲染

> 实现了BezierCurve的渲染，与BSplineCurve旋转曲面的求交和渲染
> 使用de Casteljau算法求曲线上的点
> 使用Newton法求交

### light.hpp

光源，包括点光源、面光源、环境光源、聚光灯光源和方向光源

> 为支持SPPM，实现了Photon的采样与发射
> 同时支持原有框架中的getIllumination函数，用于光源的采样

### material.hpp

材质，包括Lambertian材质、Phong材质与wavefront mtl材质，同时在类中包含Texture类，用于贴图。

> 实现了BRDF重要性采样，采用Cook-Torrance BRDF，包括Fresnel项、几何项和法线分布项
>
>> Smith's shadowing and masking function
>> Beckmann distribution
>>

### mesh.hpp

三角网格，用于求交和渲染

> 实现了wavefront obj文件的读取，包括顶点、纹理坐标、法线、材质、光滑组等信息, 并支持mtl材质，包括纹理贴图、高光贴图、凹凸贴图
> 基于tiny_obj_loader包装
> 利用BVH加速求交
> 实现了法线插值

### render.hpp

渲染器，用于渲染场景

> 基于工厂模式，实现了多种渲染器，包括Ray Casting渲染器、Path Tracing渲染器、SPPM渲染器
> Path Tracing基于smallpt
> SPPM渲染器基于PBRT-V4,并未实现HashGrid加速
> 实现了抗锯齿、openmp多线程渲染、Sampler & Filter等功能，其中景深和抗锯齿基于多次 采样求平均值
> 抗锯齿代码如下：

```cpp
for(int sy = 0; sy < 2; ++sy) {
    for(int sx = 0; sx < 2; ++sx) {
        Sample camera at (sx + x, sy + y)
        render the generated ray
    }
    Merge sample contribution into pixel
}
```

### scene_parser.hpp

基于原有框架，实现了场景解析器，用于解析场景文件，新增了对参数曲面、光源、材质、纹理等等新参数的解析

### sppm.hpp

SPPMIntegrator，用于SPPM渲染。基于PBRT-V4，实现了SPPMPixel与Photon类，未实现HashGrid加速。

> SPPMPixel类用于存储摄像机可见点，存储了光通量、位置、统计半径、有贡献的光子数量等信息
> Photon类用于存储光子，存储了光通量、位置、入射方向、入射光线等信息

### image.hpp

封装了开源库stb_image, stb_image_write, 用于读取和写入图片

### texture.hpp

纹理，用于纹理贴图、高光贴图、凹凸贴图

> 实现了mipmap采样
> 支持获取纹理颜色、法线、高光系数等信息

### utils.hpp

工具类，包括各种enum变量、预定义的常量、gamma校正、KDTree等

## Codes provided by the course

group.hpp
hit.hpp
object3d.hpp
ray.hpp
transform.hpp
triangle.hpp

## Opensource Library Used

Vecmath库，用于向量、矩阵运算

Models downloaded from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data)

miloyip(2017)[svpng v0.1.1](https://github.com/miloyip/svpng)

Ivandeve(2018)[LodePNG](https://github.com/lvandeve/lodepng)

nothings(2016)[stb](https://github.com/nothings/stb)
