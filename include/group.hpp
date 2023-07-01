#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>


class Group : public Object3D {
public:

    Group() {
        objects = std::vector<Object3D*>();
    }

    explicit Group (int num_objects) {
        objects = std::vector<Object3D*>(num_objects);
    }

    ~Group() override {
        if(objects.size() > 0) {
            for (int i = 0; i < objects.size(); ++i) {
                objects[i]->~Object3D();
                objects.clear();
            }
        }
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        bool isIntersect = false;
        for (int i = 0; i < objects.size(); i++) {
            //std::cout << "texCoord before intersect with object "<< i << " is " << h.getTexCoord() << std::endl;
            //std::cout << "Hit t(before) is " << h.getT() << std::endl;
            bool isIntersectWithObject = objects[i]->intersect(r, h, tmin);
            //std::cout << "Hit t(after) is " << h.getT() << std::endl;
            //std::cout << "texCoord after intersect with object "<< i << " is " << h.getTexCoord() << std::endl;
            if(isIntersectWithObject) {
                isIntersect = true;
            }
        }
        return isIntersect;
    }

    void addObject(int index, Object3D *obj) {
        objects[index] = obj;
    }

    int getGroupSize() {
        return objects.size();
    }
    void addObject(Object3D *obj) {
        objects.push_back(obj);
    }

    Object3D* getObject(int index) {
        return objects[index];
    }

private:
    std::vector<Object3D*> objects;
};

#endif
	
