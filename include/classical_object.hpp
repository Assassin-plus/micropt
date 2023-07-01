//implement classical 3d object
//including sphere, plane, triangle
// Path: include/classical_object.hpp
// bezier curve is in include/curve.hpp
#ifndef CLASSICAL_OBJECT_H
#define CLASSICAL_OBJECT_H

class Object3D;
class Material;

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

class BoundingBox : public Object3D {
public:
    BoundingBox(){
        min = Vector3f(0, 0, 0);
        max = Vector3f(0, 0, 0);
        material = nullptr;
    }

    BoundingBox(const Vector3f& min, const Vector3f& max) : min(min), max(max) {}

    // 扩展包围盒以包含给定点
    void extend(const Vector3f& point) {
        min = Vector3f(std::min(min.x(), point.x()), std::min(min.y(), point.y()), std::min(min.z(), point.z()));
        max = Vector3f(std::max(max.x(), point.x()), std::max(max.y(), point.y()), std::max(max.z(), point.z()));
    }

    // 扩展包围盒以包含另一个包围盒
    void extend(const BoundingBox& bbox) {
        extend(bbox.min);
        extend(bbox.max);
    }
    bool intersect(const Ray &ray, Hit &h, float tmin) override {
        float tmax = h.getT();
        Vector3f d = ray.getDirection();
        Vector3f o = ray.getOrigin();
        for (int i = 0; i < 3; ++i) {
            float invRayDir = 1.0f / d[i];
            float tNear = (min[i] - o[i]) * invRayDir;
            float tFar = (max[i] - o[i]) * invRayDir;
            if (tNear > tFar) std::swap(tNear, tFar);
            float minT = std::max(tNear, tmin);
            float maxT = std::min(tFar, h.getT());
            if (minT > maxT) return false;
            tmax = std::min(tmax, maxT);
        }
        return true;
    }

    Vector3f min;
    Vector3f max;

protected:
};

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



#endif // CLASSICAL_OBJECT_H