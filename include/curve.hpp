#ifndef __CURVE_H__
#define __CURVE_H__

#include <vector>
#include <vecmath.h>
#include <iostream>
#include "object3d.hpp"
#include "classical_object.hpp"

class Ray;
class Hit;

class Curve : public Object3D{
public:
    virtual Vector3f evaluate(double t) const = 0;
    virtual Vector3f evaluateDerivative(double t) const = 0;
    virtual bool intersect(const Ray& ray, Hit& hit, float tmin) = 0;
};

//Revolution Surface
class BezierCurve : public Curve {
public:
    BezierCurve(const std::vector<Vector3f>& controlVector3fs, Vector3f point1, Vector3f point2);
    Vector3f evaluate(double t) const override;
    Vector3f evaluateDerivative(double t) const override;
    bool intersect(const Ray& ray, Hit& hit, float tmin) override;

private:
    std::vector<Vector3f> controlVector3fs;
    Vector3f point1;
    Vector3f point2;
};

class BSplineCurve : public Curve {
public:
    BSplineCurve(const std::vector<Vector3f>& controlVector3fs, const std::vector<float>& knots, Vector3f point1, Vector3f point2, Material* m);
    Vector3f evaluate(double t) const override;
    Vector3f evaluateDerivative(double t) const override;
    bool intersect(const Ray& ray, Hit& hit, float tmin)  override;
    double f(double t, Vector3f o, Vector3f d) const;
    double Newton(double t, Vector3f o, Vector3f d) const;
    std::vector<double> init(Vector3f o, Vector3f d) const;

private:
    std::vector<Vector3f> controlVector3fs;
    std::vector<float> knots;
    Vector3f point1;
    Vector3f point2;
    BoundingBox box;
    int findSpan(double t) const;
};

class CatmullRomCurve : public Curve {
public:
    CatmullRomCurve(const std::vector<Vector3f>& controlVector3fs, float tension, Vector3f point1, Vector3f point2);
    Vector3f evaluate(double t) const override;
    Vector3f evaluateDerivative(double t) const override;
    bool intersect(const Ray& ray, Hit& hit, float tmin)  override;

private:
    std::vector<Vector3f> controlVector3fs;
    float tension;
    Vector3f point1;
    Vector3f point2;
};

class Surface : public Curve{
public:
    Vector3f evaluate(double t) const override{
        return this->evaluate(t, t);
    };
    virtual Vector3f evaluate(double u, double v) const = 0;
    virtual Vector3f evaluateDerivative(double t) const = 0;
    virtual Vector3f evaluatePartialDerivativeU(double u, double v) const = 0;
    virtual Vector3f evaluatePartialDerivativeV(double u, double v) const = 0;
};

class BezierSurface : public Surface {
public:
    BezierSurface(const std::vector<std::vector<Vector3f>>& controlVector3fs);
    Vector3f evaluate(double u, double v) const override;
    Vector3f evaluateDerivative(double t) const override;
    Vector3f evaluatePartialDerivativeU(double u, double v) const override;
    Vector3f evaluatePartialDerivativeV(double u, double v) const override;
    bool intersect(const Ray& ray, Hit& hit, float tmin)  override;

private:
    std::vector<std::vector<Vector3f>> controlVector3fs;
};

class BSplineSurface : public Surface {
public:
    BSplineSurface(const std::vector<std::vector<Vector3f>>& controlVector3fs, const std::vector<float>& knotsU, const std::vector<float>& knotsV);
    Vector3f evaluate(double u, double v) const override;
    Vector3f evaluateDerivative(double t) const override;
    Vector3f evaluatePartialDerivativeU(double u, double v) const override;
    Vector3f evaluatePartialDerivativeV(double u, double v) const override;
    bool intersect(const Ray& ray, Hit& hit, float tmin)  override;

private:
    std::vector<std::vector<Vector3f>> controlVector3fs;
    std::vector<float> knotsU;
    std::vector<float> knotsV;
};

// 添加其他类型的参数曲线和曲面的类实现




#endif //__CURVE_H__