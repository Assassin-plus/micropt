#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include "object3d.hpp"
#include "triangle.hpp"
#include "bvh.hpp"
#include "Vector2f.h"
#include "Vector3f.h"

class Material;

class Vertex{
    // each vertex stores its position, normal and texcoord as index
public:
    int _vertex_index;
    int _normal_index;
    int _texcoord_index;
};

class Face{
public:
    friend ostream& operator <<(ostream& os, const Face& f){
        os << "Face: " << endl;
        for(int i = 0; i < f._vertexes.size(); i++){
            os << f._vertexes[i]._vertex_index << " ";
        }
        os << endl;
        return os;
    };
    std::vector<Vertex> _vertexes;
};

struct num_tags{
    int int_num;
    int float_num;
    int string_num;
};

class Shape{
public:
    friend ostream& operator <<(ostream& os, const Shape& s){
        os << "Shape: " << endl;
        os << "num of faces: " << s._faces.size() << endl;
        for(int i = 0; i < s._faces.size(); i++){
            os << s._faces[i];
        }
        return os;
    };
    std::vector<Face> _faces;
    std::vector<int> _material_ids;

    //something maybe not used
    /* 
    std::vector<int> _mesh_indices;
    std::vector<int> _lines_indices;
    std::vector<int> _points_indices;
    std::vector<unsigned int> _smooth_group_ids; */
    //num_tags _tags;
    std::string _name;
};

class Mesh : public Object3D {

public:
    Mesh():Object3D(nullptr){};
    Mesh(const char *filename, Material *m);

    ~Mesh() {};

    bool intersect(const Ray &r, Hit &h, float tmin) override;

private:
    std::vector<Vector3f> _v;//attrib.vertices
    std::vector<Vector3f> _n;//attrib.normals
    std::vector<Vector2f> _uv;//attrib.texcoords
    BVHNode* _root;

    std::vector<Shape> _shapes;
    std::vector<Material *> _materials;
};

class TraditionalMesh : public Mesh {

public:
    TraditionalMesh(const char *filename, Material *m);
    ~TraditionalMesh(){};

    struct TriangleIndex {
        TriangleIndex() {
            x[0] = 0; x[1] = 0; x[2] = 0;
        }
        int &operator[](const int i) { return x[i]; }
        // By Computer Graphics convention, counterclockwise winding is front face
        int x[3]{};
    };

    std::vector<Vector3f> v;
    std::vector<TriangleIndex> t;
    std::vector<Vector3f> n;
    bool intersect(const Ray &r, Hit &h, float tmin) override;

private:
    // Normal can be used for light estimation
    void computeNormal();
};

#endif
