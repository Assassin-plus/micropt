#pragma once
#include <vector>
#include "classical_object.hpp"
#include "triangle.hpp"
#include <vecmath.h>
#include <algorithm>

class BVHNode {
public:
    BoundingBox box;
    BVHNode* left;
    BVHNode* right;
    std::vector<Triangle*> triangles;
    BVHNode() {
        box = BoundingBox();
        left = nullptr;
        right = nullptr;
        triangles = std::vector<Triangle*>();
    }
    BVHNode(std::vector<Triangle*>& triangles) {
        
        this->box = BoundingBox();
        for (auto triangle : triangles) {
            this->box.extend(triangle->_vertices[0]);
            this->box.extend(triangle->_vertices[1]);
            this->box.extend(triangle->_vertices[2]);
        }
        if(triangles.size() <= 2000){
            //this->children = std::vector<BVHNode*>();
            this->left = nullptr;
            this->right = nullptr;
            this->triangles = triangles;
            return;
        }
        //split
        //randomly choose a axis
        int axis = rand() % 3;
        sort(triangles.begin(), triangles.end(), [axis](Triangle* a, Triangle* b) {
            return a->_vertices[0][axis] < b->_vertices[0][axis];
        });
        float mid = triangles[triangles.size() / 2]->_vertices[0][axis];
        std::vector<Triangle*> leftTriangles;
        std::vector<Triangle*> rightTriangles;
        for (auto triangle : triangles) {
            if(triangle->_vertices[0][axis] < mid && triangle->_vertices[1][axis] < mid && triangle->_vertices[2][axis] < mid) {
                leftTriangles.push_back(triangle);
            }else if(triangle->_vertices[0][axis] > mid && triangle->_vertices[1][axis] > mid && triangle->_vertices[2][axis] > mid) {
                rightTriangles.push_back(triangle);
            }else{
                leftTriangles.push_back(triangle);
                rightTriangles.push_back(triangle);
            }
        }
        if(leftTriangles.size() == 0 || rightTriangles.size() == 0) {
            this->left = nullptr;
            this->right = nullptr;
            this->triangles = triangles;
            return;
        }
        this->left = new BVHNode(leftTriangles);
        this->right = new BVHNode(rightTriangles);

        
        
        /* 
        //auto is the middle point
        sort(triangles.begin(), triangles.end(), [](Triangle* a, Triangle* b) {
            return a->_vertices[0].x() < b->_vertices[0].x();
        });
        float midX = triangles[triangles.size() / 2]->_vertices[0].x();
        sort(triangles.begin(), triangles.end(), [](Triangle* a, Triangle* b) {
            return a->_vertices[0].y() < b->_vertices[0].y();
        });
        float midY = triangles[triangles.size() / 2]->_vertices[0].y();
        sort(triangles.begin(), triangles.end(), [](Triangle* a, Triangle* b) {
            return a->_vertices[0].z() < b->_vertices[0].z();
        });
        float midZ = triangles[triangles.size() / 2]->_vertices[0].z();

        std::vector<Triangle*> childrenTriangles[8];
        for (auto triangle : triangles) {
            bool isXMin = triangle->_vertices[0].x() < midX;
            bool isYMin = triangle->_vertices[0].y() < midY;
            bool isZMin = triangle->_vertices[0].z() < midZ;
            int index = 0;
            if(isXMin) index += 1;
            if(isYMin) index += 2;
            if(isZMin) index += 4;
            if(index >= 8) std::cout << "index is " << index << std::endl;
            childrenTriangles[index].push_back(triangle);
        }
        for (int i = 0; i < 8; ++i) {
            if(childrenTriangles[i].size() == 0) continue;
            children.push_back(new BVHNode(childrenTriangles[i]));
        } */

    }
    
    bool intersect(const Ray &ray, Hit &h, float tmin) {
        if(!box.intersect(ray, h, tmin)) return false;
        if(triangles.size() > 0) {
            bool isIntersect = false;
            for (auto triangle : triangles) {
                bool isIntersectWithTriangle = triangle->intersect(ray, h, tmin);
                if(isIntersectWithTriangle) {
                    isIntersect = true;
                }
            }
            return isIntersect;
        }
        bool isIntersect = false;
        if(left != nullptr) {
            bool isIntersectWithLeft = left->intersect(ray, h, tmin);
            if(isIntersectWithLeft) {
                isIntersect = true;
            }
        }
        if(right != nullptr) {
            bool isIntersectWithRight = right->intersect(ray, h, tmin);
            if(isIntersectWithRight) {
                isIntersect = true;
            }
        }
        return isIntersect;
    }
};
