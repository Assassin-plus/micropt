#ifndef SPHERE_H
#define SPHERE_H

#include <vecmath.h>
#include <cmath>

class Object3D;
class Material;

class Sphere : public Object3D {
public:
    Sphere() {
        // unit ball at the center
        this->center = Vector3f(0, 0, 0);
        this->radius = 1;
        //default material is gray
        Material *default_material = new TraditionalMaterial(Vector3f(0.5, 0.5, 0.5));
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        // 
        this->center = center;
        this->radius = radius;
        this->material = material;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        //geometry method
        /* Vector3f l = center - r.getOrigin();
        //compute l2-r2
        float l2 = Vector3f::dot(l, l);
        float r2 = radius * radius;
        float l2_r2 = l2 - r2;
        float tca = Vector3f::dot(l, r.getDirection());
        float eps = radius > 1e4 ? 1e-6 * r2 : 1e-3 * radius ; 
        if(fabs(l2_r2) < eps )//ray origin is on the sphere
        {
            return false;
        }else if(l2_r2 > 0)//ray origin is outside the sphere
        {
            if(tca < 0)
            {
                return false;
            }else{
                float d2 = l2 - tca * tca;
                if(d2 < r2)
                {
                    float thc = sqrt(r2 - d2);
                    float t0 = tca - thc;
                    if(t0 < h.getT()){
                        h.set(t0, material, (r.pointAtParameter(t0) - center) / radius, Vector2f::ZERO);
                        return true;
                    }else{
                        return false;
                    }
                }else{
                    return false;
                }
            }
        }else //ray origin is inside the sphere
        {
            float d2 = l2 - tca * tca;
            float thc = sqrt(r2 - d2);
            float t0 = tca + thc;
            if(t0 < h.getT()){
                h.set(t0, material, (r.pointAtParameter(t0) - center) / radius, Vector2f::ZERO);
                return true;
            }else{
                return false;
            }
        } */
        Vector3f op = center - r.getOrigin();
        double t, eps = radius > 1e4 ? 1e-6 * radius : 1e-3 * radius;
        double b = Vector3f::dot(op, r.getDirection());
        double det = b * b - Vector3f::dot(op, op) + radius * radius;
        if (det < 0) {
            return false;
        } else {
            det = sqrt(det);
        }

        if((t=b-det)>eps) {
            if(t<h.getT()) {
                Vector3f normal = (r.pointAtParameter(t) - center) / radius;
                h.set(t, material, normal, Vector2f::ZERO);
                return true;
            }
        } else if((t=b+det)>eps) {
            if(t<h.getT()) {
                Vector3f normal = (r.pointAtParameter(t) - center) / radius;
                h.set(t, material, normal, Vector2f::ZERO);
                return true;
            }
        }
        return false;

        //traditional method
        /* Vector3f oc = r.getOrigin() - center;
        float a = Vector3f::dot(r.getDirection(), r.getDirection());
        float b = 2 * Vector3f::dot(oc, r.getDirection());
        float c = Vector3f::dot(oc, oc) - radius * radius;
        float delta = b * b - 4 * a * c;
        if (delta < 0) {
            return false;
        }
        float t = (-b - sqrt(delta)) / (2 * a);
        if (t < tmin * radius ) {
            t = (-b + sqrt(delta)) / (2 * a);
        }
        if (t < tmin * radius ) {
            return false;
        }
        if (t < h.getT()) {
            Vector3f normal = (r.pointAtParameter(t) - center) / radius;
            h.set(t, material, normal, Vector2f::ZERO);
            return true;
        }   
        return false; */
    }

protected:
    Vector3f center;
    float radius;
    Material *material;
};


#endif
