#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>

//Original Framework
class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up).normalized();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual ~Camera() = default;
    void setDOF(bool dof, float aperture, float focalLength) {
        this->dof = dof;
        this->aperture = aperture;
        this->focalLength = focalLength;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
    // Depth of field parameters
    bool dof;
    float aperture;
    float focalLength;
};

// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {
protected:
    float angle;// means vertical angle
    float fx,fy;
    float cx,cy;
public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
            const Vector3f &up, int imgW, int imgH, float angle) : Camera(center, direction, up, imgW, imgH) {
        // angle is in radian.
        this->angle = angle;
        this->fy = imgH / (2 * tan(angle / 2));
        this->fx = this->fy;
        this->cx = imgW / 2;
        this->cy = imgH / 2;
    }

    Ray generateRay(const Vector2f &point) override {
        // generate Ray in camera space
        if(!dof) {
            //Origin is the center of the camera
            Vector3f rayDir = Vector3f((point.x() - cx) / fx, (point.y() - cy) / fy, 1).normalized();
            // transform the ray to world space
            Matrix3f R(horizontal, up, direction);
            rayDir = R * rayDir;
            return Ray(center, rayDir);
        }else {
            //Randomly sample a point on the lens
            /* float r = (float)rand() / RAND_MAX;
            float theta = (float)rand() / RAND_MAX * 2 * M_PI;
            float x = sqrt(r) * cos(theta) * aperture ;
            float y = sqrt(r) * sin(theta) * aperture ; */
            unsigned short Xi[3] = {0, 0, 8192};
            double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
            double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
            float x = dx * aperture;
            float y = dy * aperture;
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
        }
    }
};

#endif //CAMERA_H
