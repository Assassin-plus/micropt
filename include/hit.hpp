#ifndef __HIT_H__
#define __HIT_H__

#include <vecmath.h>

class ray;

class Material;

class Hit {
public:

    // constructors
    Hit() {
        material = nullptr;
        t = 1e38;
    }

    Hit(float _t, Material *m, const Vector3f &n) {
        t = _t;
        material = m;
        normal = n;
    }

    Hit(const Hit &h) {
        t = h.t;
        material = h.material;
        normal = h.normal;
    }

    // destructor
    ~Hit() = default;

    float getT() const {
        return t;
    }

    Material *getMaterial() const {
        if(material == nullptr) {
        }
        return material;
    }

    const Vector3f &getNormal() const {
        return normal;
    }

    const Vector2f &getTexCoord() const {
        return texCoord;
    }

    void set(float _t, Material *m, const Vector3f &n, const Vector2f &tC) {
        t = _t;
        material = m;
        normal = n;
        texCoord = tC;
    }

private:
    float t;
    Material *material;
    Vector3f normal;
    Vector2f texCoord;

};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
    os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
