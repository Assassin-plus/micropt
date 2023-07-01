#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Plane() {
        // default x-y plane at the center
        this->normal = Vector3f(0, 0, 1);
        this->d = 0;
        //default material is gray
        Material *default_material = new TraditionalMaterial(Vector3f(0.5, 0.5, 0.5));
        this->material = default_material;
    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->normal = normal;
        this->d = d;
        this->material = m;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float t = (d - Vector3f::dot(normal, r.getOrigin())) / Vector3f::dot(normal, r.getDirection());
        if (t < tmin) {
            return false;
        }
        if (t < h.getT()) {
            h.set(t, material, normal, Vector2f::ZERO);
            return true;
        }
        return false;
    }

protected:
    Vector3f normal;
    float d;

};

#endif //PLANE_H
		

