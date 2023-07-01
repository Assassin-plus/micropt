#ifndef __UTILS_H__
#define __UTILS_H__

#include <bits/stdc++.h>
#include <omp.h>
#include <vecmath.h>
#include <cassert>
const double EPS = 1e-6;
const double PI = acos(-1.0);
const double INF = 1e300;
const double EPS2 = 1e-12;

extern bool smooth;
extern bool useBVH;

enum BRDFType { DIFFUSE, SPECULAR, REFRACTION, EMISSION, SUBSURFACE, MEDIA, MICROFACET, EMPIRICAL, PBR, NONE };

enum LightType { POINT, AREA, SPOT, ABSORB, NONE_LIGHT };

enum RayType { PRIMARY, SHADOW, REFLECTED, REFRACTED, NONE_RAY };

enum RenderMode { PT, SPPM, VCM, BDPT, MLT, VRPT, RC, NONE_RENDER };
extern RenderMode RENDER;

enum AcceleratorType { BVH, KDTREE, OCTREE, HASHGRID, NONE_ACCELERATOR };
extern AcceleratorType ACCELERATOR;

enum CameraType { PERSPECTIVE, ORTHOGRAPHIC, NONE_CAMERA };

enum SamplerType { RANDOM, STRATIFIED, HALTON, SOBOL, NONE_SAMPLER };
extern SamplerType SAMPLER;

enum FilterType { BOX, GAUSSIAN, MITCHELL, LANCZOS, NONE_FILTER };
extern FilterType FILTER;

void parse_arg(int argc, char *argv[], int& width, int& height, int& samples, int& threads, int& depth, int& quality, std::string& input, std::string& output, bool& DOF, float& aperture, float& focus_length);

inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }
inline float clamp(float x){ return x<0 ? 0 : x>1 ? 1 : x; }
inline int gammaCorrection(double x){ return int(pow(clamp(x),1/2.2)*255+.5); }

inline Vector3f clamp(const Vector3f& v) {
    return Vector3f(clamp(v.x()), clamp(v.y()), clamp(v.z()));
}
inline Vector3f gammaCorrection(const Vector3f& v) {
    return Vector3f(gammaCorrection(v.x()), gammaCorrection(v.y()), gammaCorrection(v.z()));
}

inline float Fresnel(const Vector3f& I, const Vector3f& N, float ior) {
    float cosi = Vector3f::dot(I, N);
    cosi = cosi > 1 ? 1 : cosi < -1 ? -1 : cosi;
    float etai = 1, etat = ior;
    if (cosi > 0) std::swap(etai, etat);
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) return 1;
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        return (Rs * Rs + Rp * Rp) / 2;
    }
}

inline Vector3f lerp(const Vector3f& a, const Vector3f& b, float t) {
    assert(t >= 0 && t <= 1);
    return a * (1 - t) + b * t;
}

inline double map(double x){
    return x > 1 ? 1 : x < -1 ? -1 : x;
}

void mapToImage(int& x, int& y, int width, int height);

Vector2f mapToImage(Vector2f p, int width, int height);

//use template to write kd-tree
//must contain k-NNSearch
//TODO: validate the correctness of kd-tree

template <typename T>
class KDNode {
public:
    KDNode() = default;
    KDNode(const T& data) : data(data) {
        left = nullptr;
        right = nullptr;
    }
    KDNode(const T& data, KDNode* left, KDNode* right) : data(data), left(left), right(right) {}
    KDNode(const KDNode& node) {
        data = node.data;
        left = node.left;
        right = node.right;
    }
    KDNode& operator=(const KDNode& node) {
        data = node.data;
        left = node.left;
        right = node.right;
        return *this;
    }
    KDNode(KDNode&& node) {
        data = std::move(node.data);
        left = node.left;
        right = node.right;
    }
    KDNode& operator=(KDNode&& node) {
        data = std::move(node.data);
        left = node.left;
        right = node.right;
        return *this;
    }

    ~KDNode() {
        delete left;
        delete right;
    }

    T data;
    KDNode<T>* left;
    KDNode<T>* right;
};

template <typename T>
class KDTree {
public:
    KDTree() = default;
    KDTree(const std::vector<T>& data, int dim) : data(data), dim(dim) {
        root = build(0, data.size() - 1, 0);
    }

    ~KDTree() {
        delete root;
    }

    void build(const std::vector<T>& data, int dim) {
        this->data = data;
        this->dim = dim;
        root = build(0, data.size() - 1, 0);
    }

    T* kNNSearch(const T& target, int k, float* dist) {
        //a priority queue ordered by distance, the first element in the queue
        std::priority_queue<std::pair<double, T> > pq;
        kNNSearch(root, target, k, pq);
        if(pq.empty()) return nullptr;
        T* res = new T[k];
        *dist = (float)pq.top().first;
        for (int i = 0; i < k; i++) {
            res[i] = pq.top().second;
            //std::cout << "final:"<< pq.top().second << std::endl;
            pq.pop();
        }
        return res;
    }

    std::vector<T> rangeSearch(const T& target, float radius){
        std::queue<KDNode<T>*> q;
        q.push(root);
        std::vector<T> res;
        while (!q.empty()) {
            KDNode<T>* node = q.front();
            q.pop();
            if(node == nullptr) continue;
            double dist = node->data.distance(target);
            if(dist <= radius){
                res.push_back(node->data);
            }
            double split = node->data.get(dim);
            double targetSplit = target.get(dim);
            if (targetSplit < split) {
                q.push(node->left);
                if (fabs(targetSplit - split) < radius) {
                    q.push(node->right);
                }
            } else {
                q.push(node->right);
                if (fabs(targetSplit - split) < radius) {
                    q.push(node->left);
                }
            }
        }
        if(res.empty()) return std::vector<T>();
        return res;
    }

    std::vector<T> rangeSearch(const Vector3f& target, float radius){
        std::queue<KDNode<T>*> q;
        q.push(root);
        std::vector<T> res;
        while (!q.empty()) {
            KDNode<T>* node = q.front();
            q.pop();
            if(node == nullptr) continue;
            double dist = node->data.distance(target);
            if(dist <= radius){
                res.push_back(node->data);
            }
            double split = node->data.get(dim);
            double targetSplit = target.get(dim);
            if (targetSplit < split) {
                q.push(node->left);
                if (fabs(targetSplit - split) < radius) {
                    q.push(node->right);
                }
            } else {
                q.push(node->right);
                if (fabs(targetSplit - split) < radius) {
                    q.push(node->left);
                }
            }
        }
        if(res.empty()) return std::vector<T>();
        return res;
    }

    void print() {
        std::queue<KDNode<T>*> q;
        q.push(root);
        while (!q.empty()) {
            KDNode<T>* node = q.front();
            q.pop();
            std::cout << node->data << std::endl;
            if (node->left != nullptr) q.push(node->left);
            if (node->right != nullptr) q.push(node->right);
        }
    }

    void reserve(uint32_t size) {
        data.reserve(size);
    }

private:
    void kNNSearch(KDNode<T>* node, const T& target, int k, std::priority_queue<std::pair<double, T>>& pq) {
        if (node == nullptr) return;
        double dist = node->data.distance(target);
        //std::cout << "push " << node->data << " " << dist << std::endl;
        pq.push(std::make_pair(dist, node->data));
        if (pq.size() > k) pq.pop();
        double split = node->data.get(dim);
        double targetSplit = target.get(dim);
        if (targetSplit < split) {
            kNNSearch(node->left, target, k, pq);
            if (pq.size() < k || fabs(targetSplit - split) < pq.top().first) {
                kNNSearch(node->right, target, k, pq);
            }
        } else {
            kNNSearch(node->right, target, k, pq);
            if (pq.size() < k || fabs(targetSplit - split) < pq.top().first) {
                kNNSearch(node->left, target, k, pq);
            }
        }
    }

    KDNode<T>* build(int l, int r, int d) {
        if (l > r) return nullptr;
        int mid = (l + r) >> 1;
        std::nth_element(data.begin() + l, data.begin() + mid, data.begin() + r + 1, [&](const T& a, const T& b) {
            return a.get(d) < b.get(d);
        });
        KDNode<T>* node = new KDNode<T>(data[mid]);
        node->left = build(l, mid - 1, (d + 1) % dim);
        node->right = build(mid + 1, r, (d + 1) % dim);
        return node;
    }

    KDNode<T>* root;
    std::vector<T> data;
    int dim;

};
#endif // __UTILS_H__