#include "../include/curve.hpp"
#include "../include/ray.hpp"
#include "../include/hit.hpp"
#include "../include/utils.hpp"
#include "../include/material.hpp"
#include "../include/classical_object.hpp"
#include <iostream>
#include <queue>

// BezierCurve
BezierCurve::BezierCurve(const std::vector<Vector3f>& controlVector3fs, Vector3f point1, Vector3f point2) {
    this->controlVector3fs = controlVector3fs;
    this->point1 = point1;
    this->point2 = point2;
}

Vector3f BezierCurve::evaluate(double t) const {
    //de Casteljau's algorithm
    std::vector<Vector3f> controlVector3fs = this->controlVector3fs;
    int n = controlVector3fs.size() - 1;
    for (int r = 1; r <= n; r++) {
        for (int i = 0; i <= n - r; i++) {
            controlVector3fs[i] = (1 - t) * controlVector3fs[i] + t * controlVector3fs[i + 1];
        }
    }
    return controlVector3fs[0];
}

Vector3f BezierCurve::evaluateDerivative(double t) const {
    //de Casteljau's algorithm
    //default: first derivative
    std::vector<Vector3f> controlVector3fs = this->controlVector3fs;
    int n = controlVector3fs.size() - 1;
    int prefix = 1;
    int derivative = 1;
    for(int i = 0; i < derivative; i++){
        prefix *= n - i;
    }
    for (int k = 0; k < derivative; k++) {
        for (int r = 0; r <= n - derivative; r++) {
            controlVector3fs[r] =  (controlVector3fs[r + 1] - controlVector3fs[r]);
        }
    }
    for (int i = 0; i < n - derivative; i++) {
        for(int j = 0; j < n - derivative - i; j++){
            controlVector3fs[j] = ( 1 - t ) * controlVector3fs[j] + t * controlVector3fs[j + 1];
        }
    }
    return prefix * controlVector3fs[0];

}

bool BezierCurve::intersect(const Ray& ray, Hit& hit, float tmin){
    //
}


// BSplineCurve
BSplineCurve::BSplineCurve(const std::vector<Vector3f>& controlVector3fs, const std::vector<float>& knots, Vector3f point1, Vector3f point2, Material* m = nullptr) {
    this->controlVector3fs = controlVector3fs;
    this->knots = knots;
    this->point1 = point1;
    this->point2 = point2;
    this->material = m;
    box = BoundingBox();
    for(int i = 0; i < controlVector3fs.size(); i++){
        box.extend(Vector3f(controlVector3fs[i].x(), 0, controlVector3fs[i].y()));
        box.extend(Vector3f(-controlVector3fs[i].x(), 0, controlVector3fs[i].y()));
        box.extend(Vector3f(0, controlVector3fs[i].x(), controlVector3fs[i].y()));
        box.extend(Vector3f(0, -controlVector3fs[i].x(), controlVector3fs[i].y()));
    }
}

int BSplineCurve::findSpan(double t) const{
    int p = knots.size() - controlVector3fs.size() - 1;
    int n = controlVector3fs.size() - 1;
    if(t < knots[p] || t > knots[knots.size()-p])
        return p;
    //assert(t >= knots[p] && t <= knots[knots.size()-p]);
    if (t >= knots[n + 1]) {
        return n;
    }
    int low = knots.size() - controlVector3fs.size() - 1;
    int high = n + 1;
    int mid = (low + high) / 2;
    while (t < knots[mid] || t >= knots[mid + 1]) {
        if (t < knots[mid]) {
            high = mid;
        } else {
            low = mid;
        }
        mid = (low + high) / 2;
    }
    return mid;
}

Vector3f BSplineCurve::evaluate(double t) const {
    //de Boor Cox algorithm
    int k = findSpan(t);
    int p = knots.size() - controlVector3fs.size() - 1;
    assert(p >= 0);
    std::vector<Vector3f> d(p + 1);
    for (int i = 0; i <= p; i++) {
        d[i] = controlVector3fs[k - p + i];
    }
    for (int r = 1; r <= p; r++) {
        for (int i = p; i >= r; i--) {
            float alpha = (t - knots[k - p + i]) / (knots[i + k - r + 1] - knots[k - p + i]);
            d[i] = (1.0 - alpha) * d[i - 1] + alpha * d[i];
        }
    }
    return d[p];
}

Vector3f BSplineCurve::evaluateDerivative(double t) const {
    //de Boor Cox algorithm
    std::vector<Vector3f> controlVector3fs = this->controlVector3fs;
    // sum_i=1^n p(P_i - P_i-1) / (u_p+i - u_i) N_i,p-1(u)
    /* int k = findSpan(t);
    int p = knots.size() - controlVector3fs.size() - 1;
    assert(p >= 0);
    std::vector<Vector3f> d(p + 1);
    
    for (int i = 0; i <= p; i++) {
        d[i] = controlVector3fs[k - p + i];
    }
    for (int r = 1; r <= p; r++) {
        for (int i = p; i >= r; i--) {
            float alpha = (t - knots[k - p + i]) / (knots[i + k - r + 1] - knots[k - p + i]);
            d[i] = (1.0 - alpha) * d[i - 1] + alpha * d[i];
        }
    }
    return p * (d[p] - d[p - 1]) / (knots[k + p] - knots[k]); */
    std::vector<Vector3f> q(controlVector3fs.size() - 1);
    int p = knots.size() - controlVector3fs.size() - 1;
    for(int i = 0; i < q.size(); i++){
        q[i] = p * (controlVector3fs[i + 1] - controlVector3fs[i]) / (knots[i + p + 1] - knots[i + 1]);
    }
    BSplineCurve curve(q, knots, point1, point2);
    return curve.evaluate(t);
}



std::vector<double> BSplineCurve::init(Vector3f o, Vector3f d) const{
    //initialize x0
    /* std::vector<double> x ;
    int p = knots.size() - controlVector3fs.size() - 1;
    int n = controlVector3fs.size() - 1;
    std::queue<float> start , end;
    start.push(knots[p]);
    end.push(knots[n + 1]);
    while (true) {
        if(start.empty() || end.empty()){
            break;
        }
        float s = start.front();
        float e = end.front();
        start.pop();
        end.pop();
        if(s > e || fabs(s - e) < 0.001){
            break;
        }
        float mid = (s + e) / 2.0f;
        if(f(mid, o, d) * f(s, o, d) < 0){
            x.push_back((s + mid) / 2.0f);
        }else{
            start.push(s);
            end.push(mid);
        }
        if(f(mid, o, d) * f(e, o, d) < 0){
            x.push_back((e + mid) / 2.0f);
        }else{
            start.push(mid);
            end.push(e);
        }
    }
    return x; */
    //tranverse
    std::vector<double> x;
    double start = knots[knots.size() - controlVector3fs.size() - 1];
    double end = knots[controlVector3fs.size()];
    double t = start;
    //TODO: Tune the number of points
    for(;t < end; t += (end - start) / 200.0f){
        if(fabs(f(t, o, d)) < 0.01){
            x.push_back(t);
        }
    }
    return x;
}

double BSplineCurve::Newton(double t, Vector3f o, Vector3f d) const{
    Vector3f p = evaluate(t);
    Vector3f dp = evaluateDerivative(t);
    // rotate along x axis
    /* double q = p.x() - o.x();
    double f = pow((q * d.z() + o.z() * d.x()), 2) + pow((q * d.y() + o.y() * d.x()), 2) - pow(p.y() * d.x(), 2);
    double df = 2 * d.z() * dp.x() * (q * d.z() + o.z() * d.x()) + 2 * d.y() * dp.x() * (q * d.y() + o.y() * d.x()) - 2 * p.y() * dp.y() * d.x() * d.x();
    return f / df; */
    //rotate along z axis
    double q = (p.y() - o.z());
    double f = (q * d.x() + o.x() * d.z()) * (q * d.x() + o.x() * d.z()) + (q * d.y() + o.y() * d.z()) * (q * d.y() + o.y() * d.z()) - p.x() * p.x() * d.z()* d.z();
    double df = 2 * d.x() * dp.y() * (q * d.x() + o.x() * d.z()) + 2 * d.y() * dp.y() * (q * d.y() + o.y() * d.z()) - 2 * p.x() * dp.x() * d.z() * d.z();
    //two-dimensional
    //double f = d.y() * p.x() - d.x() * p.y() - o.x() * d.y() + o.y() * d.x();
    //double df = d.y() * dp.x() - d.x() * dp.y();
    return f / df;
}

double BSplineCurve::f(double t, Vector3f o, Vector3f d) const{
    Vector3f p = evaluate(t);
    // rotate along x axis
    //double q = p.x() - o.x();
    //return pow((q * d.z() + o.z() * d.x()), 2) + pow((q * d.y() + o.y() * d.x()), 2) - pow(p.y() * d.x(), 2);
    // rotate along z axis
    double q = (p.y() - o.z()) ;
    return pow((q * d.x() + o.x() * d.z()), 2) + pow((q * d.y() + o.y() * d.z()), 2) - pow(p.x() * d.z(), 2);
    //return (q * d.x() + o.x() * d.z()) * (q * d.x() + o.x() * d.z()) + (q * d.y() + o.y() * d.z()) * (q * d.y() + o.y() * d.z()) - p.x() * p.x() * d.z()* d.z();
    //two-dimensional
    //return d.y() * p.x() - d.x() * p.y() - o.x() * d.y() + o.y() * d.x();
}

bool BSplineCurve::intersect(const Ray& ray, Hit& hit, float tmin){
    //Newton's Method
    //acceleration: bounding box
    //create rotate matrix to let line [point1, point2] to be z axis
    Vector3f rz = point2 - point1;
    rz.normalize();
    Vector3f rx = Vector3f::cross(Vector3f(0,0,1), rz);
    if(rx.length() < EPS){
        rx = Vector3f::cross(Vector3f(0,1,0), rz);
    }
    rx.normalize();
    Vector3f ry = Vector3f::cross(rz, rx);
    ry.normalize();
    Matrix3f rotate = Matrix3f(rx, ry, rz);
    Matrix3f rotateInverse = rotate.inverse();
    Vector3f o = rotateInverse * (ray.getOrigin() - point1);
    Vector3f d = rotateInverse * ray.getDirection();
    //std::cout << "o is " << o << std::endl;
    //std::cout << "d is " << d << std::endl;

    
    if(!box.intersect(Ray(o,d), hit, tmin)){
        return false;
    }
    bool isIntersect = false;

    int k = knots.size() - controlVector3fs.size() - 1;
    //temporarily set the middle point of the curve
    //double mid = (knots[k] + knots[knots.size()-k]) / 2.0f;
    //Vector3f o = ray.getOrigin();
    //Vector3f d = ray.getDirection();
    std::vector<double> x = init(o, d);

    for(int i = 0; i < x.size(); i++){
        double x0 = x[i], x1 = 0;    
        int time = 0;
        //TODO: Tune the depth of Newton's Method
        while(time < 20){
            //use special case of rotate surface
            if(x0 < knots[k] || x0 > knots[controlVector3fs.size()]){
                break;
            }
            x1 = x0;
            x0 = x1 - Newton(x1, o, d);
            time ++;
            if(fabs(x1 - x0) < EPS)
                break;
        }
        if(x0 < knots[k] || x0 > knots[controlVector3fs.size()]){
            continue;
        }
        if(fabs(x1 - x0) > EPS)
            continue;

        if(fabs(f(x0, o, d)) < EPS){
            Vector3f p = evaluate(x0);
            if(fabs(d.z()) > EPS){
                double tr = (p.y() - o.z()) / d.z();
                if((p.y() - (o.z() + tr * d.z())) > EPS){
                    //std::cout << p.y() - (o.z() + tr * d.z()) << std::endl;
                    //continue;
                }
                if(tr < hit.getT()){
                    //PhongMaterial* m = new PhongMaterial(Vector3f(0.5,0.5,0.5), Vector3f(0.5,0.5,0.5), 1);
                    double sint = asin(map((tr * d.y() + o.y())/p.x()));
                    double cost = acos(map((tr * d.x() + o.x())/p.x()));
                    //sint: -pi/2 ~ pi/2
                    //cost: 0 ~ pi
                    //theta: -pi ~ pi
                    double theta = sint >= 0 ? cost : -cost;
                    if(fabs(sint) < EPS){
                        theta = cost >= 0 ? 0 : PI;
                    }
                    if(fabs(cost) < EPS){
                        theta = sint >= 0 ? PI / 2 : -PI / 2;
                    }
                    Vector3f dp = evaluateDerivative(x0);
                    Vector3f normal = Vector3f::cross(Vector3f(dp.x() * cos(theta), dp.x() * sin(theta), dp.y()), Vector3f(-sin(theta),cos(theta),0));
                    
                    normal = rotate * normal;
                    normal.normalize();
                    Vector2f texCoord = Vector2f((x0 - knots[k])/(knots[knots.size()-k] - knots[k]), (theta + PI) / (2 * PI));
                    hit.set(tr, this->material, normal, texCoord);
                    isIntersect = true;
                }
            }else{
                //std::cout << d.z() << std::endl;

                float a = d.x() * d.x() + d.y() * d.y();
                float b = 2 * (d.x() * o.x() + d.y() * o.y());
                float c = o.x() * o.x() + o.y() * o.y() - p.x() * p.x();
                float delta = b * b - 4 * a * c;
                if(delta < 0){
                    continue;
                }
                float t1 = (-b + sqrt(delta)) / (2 * a);
                float t2 = (-b - sqrt(delta)) / (2 * a);
                float tr = 0;

                if(t1 > tmin && t2 > tmin){
                    tr = std::min(t1, t2);
                }else if(t1 > tmin){
                    tr = t1;
                }else if(t2 > tmin){
                    tr = t2;
                }else{
                    continue;
                }
                if(tr < hit.getT()){
                    //PhongMaterial* m = new PhongMaterial(Vector3f(0.5,0.5,0.5), Vector3f(0.5,0.5,0.5), 1);
                    double sint = asin(map((tr * d.y() + o.y())/p.x()));
                    double cost = acos(map((tr * d.x() + o.x())/p.x()));
                    //sint: -pi/2 ~ pi/2
                    //cost: 0 ~ pi
                    //theta: -pi ~ pi
                    double theta = sint >= 0 ? cost : -cost;
                    if(fabs(sint) < EPS){
                        theta = cost >= 0 ? 0 : PI;
                    }
                    if(fabs(cost) < EPS){
                        theta = sint >= 0 ? PI / 2 : -PI / 2;
                    }
                    if((p.y() - (o.z() + tr * d.z())) > EPS){
                        //std::cout << p.y() - (o.z() + tr * d.z()) << std::endl;
                        //continue;
                    }
                    Vector3f dp = evaluateDerivative(x0);
                    Vector3f normal = Vector3f::cross(Vector3f(dp.x() * cos(theta), dp.x() * sin(theta), dp.y()), Vector3f(-sin(theta),cos(theta),0));
                    normal = rotate * normal;
                    normal.normalize();
                    Vector2f texCoord = Vector2f((x0 - knots[k])/(knots[knots.size()-k] - knots[k]), (theta + PI) / (2 * PI));
                    hit.set(tr, this->material, normal, texCoord);
                    isIntersect = true;
                }
                
            }
        }
    }
    return isIntersect;
}

// CatmullRomCurve

CatmullRomCurve::CatmullRomCurve(const std::vector<Vector3f>& controlVector3fs, float tension, Vector3f point1, Vector3f point2) {
    this->controlVector3fs = controlVector3fs;
    this->tension = tension;
    this->point1 = point1;
    this->point2 = point2;
}

Vector3f CatmullRomCurve::evaluate(double t) const {
    return Vector3f();
}

Vector3f CatmullRomCurve::evaluateDerivative(double t) const {
    return Vector3f();
}

bool CatmullRomCurve::intersect(const Ray& ray, Hit& hit, float tmin){
    //
    return true;
}

// BezierSurface

BezierSurface::BezierSurface(const std::vector<std::vector<Vector3f>>& controlVector3fs) {
    this->controlVector3fs = controlVector3fs;
}

Vector3f BezierSurface::evaluate(double u, double v) const {
    return Vector3f();
}

Vector3f BezierSurface::evaluateDerivative(double t) const {
    return Vector3f();
}

Vector3f BezierSurface::evaluatePartialDerivativeU(double u, double v) const {
    return Vector3f();
}

Vector3f BezierSurface::evaluatePartialDerivativeV(double u, double v) const {
    return Vector3f();
}

bool BezierSurface::intersect(const Ray& ray, Hit& hit, float tmin){
    //
    return true;
}

// BSplineSurface

BSplineSurface::BSplineSurface(const std::vector<std::vector<Vector3f>>& controlVector3fs, const std::vector<float>& knotsU, const std::vector<float>& knotsV) {
    this->controlVector3fs = controlVector3fs;
    this->knotsU = knotsU;
    this->knotsV = knotsV;
}

Vector3f BSplineSurface::evaluate(double u, double v) const {
    return Vector3f();
}

Vector3f BSplineSurface::evaluateDerivative(double t) const {
    return Vector3f();
}

Vector3f BSplineSurface::evaluatePartialDerivativeU(double u, double v) const {
    return Vector3f();
}

Vector3f BSplineSurface::evaluatePartialDerivativeV(double u, double v) const {
    return Vector3f();
}

bool BSplineSurface::intersect(const Ray& ray, Hit& hit, float tmin){
    //
    return true;
}
