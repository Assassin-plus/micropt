#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <Vector3f.h>
#include "object3d.hpp"
#include "texture.hpp"

struct SPPMPixel {
    // SPPMPixel Public Members
    float radius ;//radius of the circle
    Vector3f Ld = Vector3f::ZERO;//direct lighting
    bool hasHit = false;//whether the pixel has been hit
    struct VisiblePoint {
        Vector3f p;//position
        Vector3f n;//normal
        Vector3f wo;//outgoing direction
        Material *m = nullptr;//material
        Vector3f beta = Vector3f(1, 1, 1);//path throughput
        Vector3f tau = Vector3f::ZERO;//flux
        float cnt = 0;//photon count
        VisiblePoint() = default;
        VisiblePoint(const Vector3f &p, const Vector3f &n, const Vector3f &wo, Material *m, const Vector3f &beta)
            : p(p), n(n), wo(wo), m(m), beta(beta) {}
    } vp;
    Vector3f tau = Vector3f::ZERO;//flux
    float n = 0;//photon count
    SPPMPixel() = default;
    SPPMPixel(const float& r) : radius(r) {}
};
//work as a node in the photon map(like a kd-tree)
struct Photon {
    Photon() = default;
    Photon(const Vector3f &p, const Vector3f &alpha, const Vector3f &wi)
        : p(p), alpha(alpha), wi(wi) {}
    float get(int i) const {
        return p[i];
    }
    double distance(const Photon &photon) const {
        return (p - photon.p).length();
    }
    double distance(const Vector3f &point) const {
        return (p - point).length();
    }
    bool operator<(const Photon &photon) const {
        return p < photon.p;
    }
    Vector3f p;//position
    Vector3f alpha;//flux
    Vector3f wi;//incoming direction
};

class Light {
public:
    Light() = default;

    virtual ~Light() = default;

    virtual void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const = 0;
    virtual Photon emitPhotonSampler() const = 0;
};


class DirectionalLight : public Light {
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f &d, const Vector3f &c) {
        direction = d.normalized();
        color = c;
    }

    ~DirectionalLight() override = default;

    ///@param p unsed in this function
    ///@param distanceToLight not well defined because it's not a point light
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = -direction;
        col = color;
    }

    Photon emitPhotonSampler() const override {
        return Photon(Vector3f::ZERO, color, -direction);
    }

private:

    Vector3f direction;
    Vector3f color;

};

class PointLight : public Light {
public:
    PointLight() = delete;

    PointLight(const Vector3f &p, const Vector3f &c) {
        position = p;
        color = c;
    }

    ~PointLight() override = default;

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = (position - p);
        dir = dir / dir.length();
        //physically correct intensity falloff
        float distance = (position - p).length();
        col = color / (distance * distance);
        //color is the intensity at distance 1
    }

    Photon emitPhotonSampler() const override {
        //sample a point on the sphere
        float theta = 2 * M_PI * (rand() / (RAND_MAX + 1.0));
        float phi = acos(1 - 2 * (rand() / (RAND_MAX + 1.0)));
        Vector3f direction = Vector3f(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
        return Photon(position, color, direction);
    }

private:

    Vector3f position;
    Vector3f color;

};

class AreaLight : public Light {
private:
    Vector3f position;
    Vector3f color;
    Vector3f normal;
    float area;
    Texture *texture;
public:
    AreaLight() = delete;

    AreaLight(const Vector3f &p, const Vector3f &c, const Vector3f &n, float area,
              Texture *texture = nullptr) {
        position = p;
        color = c;
        normal = n;
        this->area = area;
        this->texture = texture;
    }

    ~AreaLight() override = default;

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        //TODO: calculate Illumination using Monte Carlo integration
    }

    Photon emitPhotonSampler() const override {
        //TODO: randomly sample a point on the light source
        //the shape of arealight is a circle
        //sample a point on the circle
        float theta = 2 * M_PI * (rand() / (RAND_MAX + 1.0));
        float r = sqrt((rand() / (RAND_MAX + 1.0))) * area;
        Vector3f u = Vector3f::cross(normal, Vector3f::RIGHT).normalized();
        if(u.length() < 0.0001)
            u = Vector3f::cross(normal, Vector3f::UP).normalized();
        Vector3f v = Vector3f::cross(normal, u).normalized();
        Vector3f origin = position + r * cos(theta) * u + r * sin(theta) * v;
        //uniformly sample a direction on the hemisphere
        float theta2 = 2 * M_PI * (rand() / (RAND_MAX + 1.0));
        float phi = acos(1 - (rand() / (RAND_MAX + 1.0)));
        Vector3f direction = normal * cos(phi) + sin(phi) * (sin(theta2) * u + cos(theta2) * v);
        Vector3f flux = color * area * area * cos(phi);
        return Photon(origin, flux, direction);
    }
};

class AmbientLight : public Light {
public:
    AmbientLight() = delete;

    AmbientLight(const Vector3f &c) {
        color = c;
    }

    ~AmbientLight() override = default;

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = Vector3f::ZERO;
        col = color;
    }
    Photon emitPhotonSampler() const override {
        return Photon(Vector3f::ZERO, color, Vector3f::ZERO);
    }

private:
    Vector3f color;
};

class SpotLight : public Light {
public:
    SpotLight() = delete;

    SpotLight(const Vector3f &p, const Vector3f &d, const Vector3f &c, float angle, float intensity) {
        position = p;
        direction = d.normalized();
        color = c;
        coneAngle = angle;
        lightIntensity = intensity;
    }

    ~SpotLight() override = default;

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // Calculate the direction from the hit point to the light source
        dir = (position - p).normalized();

        // Calculate the angle between the direction to the light and the spotlight direction
        float cosAngle = Vector3f::dot(dir, -direction);

        // Check if the hit point is within the spotlight cone
        if (cosAngle >= cos(coneAngle / 2.0f)) {
            // Calculate the intensity of the light at the hit point
            float intensity = cosAngle / (1.0f + lightIntensity * (1.0f - cosAngle));
            col = color * intensity;
        } else {
            // The hit point is outside the spotlight cone, so there is no illumination
            col = Vector3f::ZERO;
        }
    }

    Photon  emitPhotonSampler() const override {
        //sample a point inside the cone
        float theta = 2 * M_PI * (rand() / (RAND_MAX + 1.0));
        float phi = acos(1 - 2 * (rand() / (RAND_MAX + 1.0)));
        phi = phi * coneAngle / M_PI;
        //base vector is direction
        Vector3f direction = Vector3f(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
        //convert to world space
        Vector3f u = Vector3f::cross(direction, Vector3f::UP).normalized();
        Vector3f v = Vector3f::cross(direction, u).normalized();
        direction = direction.x() * u + direction.y() * v + direction.z() * direction;
        float intensity = cos(phi) / (1.0f + lightIntensity * (1.0f - cos(phi)));
        return Photon(position, color * intensity, direction);

    }

private:
    Vector3f position;
    Vector3f direction;
    Vector3f color;
    float coneAngle;
    float lightIntensity;
};


#endif // LIGHT_H
