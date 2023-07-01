#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>
#include "../include/texture.hpp"
#include "../include/utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

/* static void PrintInfo(const tinyobj::attrib_t& attrib,
                      const std::vector<tinyobj::shape_t>& shapes,
                      const std::vector<tinyobj::material_t>& materials) {
  std::cout << "# of vertices  : " << (attrib.vertices.size() / 3) << std::endl;
  std::cout << "# of normals   : " << (attrib.normals.size() / 3) << std::endl;
  std::cout << "# of texcoords : " << (attrib.texcoords.size() / 2)
            << std::endl;

  std::cout << "# of shapes    : " << shapes.size() << std::endl;
  std::cout << "# of materials : " << materials.size() << std::endl;

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].mesh.indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));
    printf("Size of shape[%ld].lines.indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].lines.indices.size()));
    printf("Size of shape[%ld].points.indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].points.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.smoothing_group_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);
      printf("  face[%ld].smoothing_group_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.smoothing_group_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("    bump_multiplier = %f\n", static_cast<const double>(materials[i].bump_texopt.bump_multiplier));
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", static_cast<const double>(materials[i].roughness));
    printf("  material.Pm     = %f\n", static_cast<const double>(materials[i].metallic));
    printf("  material.Ps     = %f\n", static_cast<const double>(materials[i].sheen));
    printf("  material.Pc     = %f\n", static_cast<const double>(materials[i].clearcoat_thickness));
    printf("  material.Pcr    = %f\n", static_cast<const double>(materials[i].clearcoat_thickness));
    printf("  material.aniso  = %f\n", static_cast<const double>(materials[i].anisotropy));
    printf("  material.anisor = %f\n", static_cast<const double>(materials[i].anisotropy_rotation));
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}
 */
/* static bool TestLoadObj(const char* filename, const char* basepath = NULL,
                        bool triangulate = true) {
  std::cout << "Loading " << filename << std::endl;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename,
                              basepath, triangulate);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  if (!ret) {
    printf("Failed to load/parse .obj.\n");
    return false;
  }

  PrintInfo(attrib, shapes, materials);

  return true;
}
 */

Mesh::Mesh(const char *filename, Material *material) : Object3D(material) {
    std::cout << "Mesh constructor" << std::endl;
    std::cout << "Loading " << filename << std::endl;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    bool triangulate = true;

    std::string warn;
    std::string err;
    std::string inputfile = filename;
    
    string::size_type iPos = (inputfile .find_last_of('\\') + 1) == 0 ?  inputfile .find_last_of('/') + 1: inputfile .find_last_of('\\') + 1 ;
    string basepath = inputfile.substr(0,iPos);//获取文件路径

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename,
                                basepath.c_str(), triangulate);

    if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
    }

    if (!ret) {
    printf("Failed to load/parse .obj.\n");
    }
    #ifdef __DEBUG__
    PrintInfo(attrib, shapes, materials);
    #endif
    //parse attrib, shapes and materials from tinyobjloader to mesh
    //attrib
    for(size_t i = 0; i < attrib.vertices.size(); i+=3){
        Vector3f vertex(attrib.vertices[i], attrib.vertices[i+1], attrib.vertices[i+2]);
        _v.push_back(vertex);
    }

    for(size_t i = 0; i < attrib.normals.size(); i+=3){
        Vector3f normal(attrib.normals[i], attrib.normals[i+1], attrib.normals[i+2]);
        _n.push_back(normal);
    }

    for(size_t i = 0; i < attrib.texcoords.size(); i+=2){
        Vector2f texcoord(attrib.texcoords[i], attrib.texcoords[i+1]);
        _uv.push_back(texcoord);
    }

    std::cout << "vertices: " << _v.size() << std::endl;
    std::cout << "normals: " << _n.size() << std::endl;
    std::cout << "texcoords: " << _uv.size() << std::endl;


    //shapes
    for(size_t i = 0; i < shapes.size(); i++) {
        Shape shape;
        shape._name = shapes[i].name;
        size_t index_offset = 0;

        assert(shapes[i].mesh.num_face_vertices.size() ==
            shapes[i].mesh.material_ids.size());

        assert(shapes[i].mesh.num_face_vertices.size() ==
            shapes[i].mesh.smoothing_group_ids.size());
        
        for(size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++){
            Face face;
            size_t fnum = shapes[i].mesh.num_face_vertices[f];
            //std::cout << "face[" << f << "].fnum = " << fnum << std::endl;
            for(size_t v = 0; v < fnum; v++){
                Vertex vertex;
                tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
                vertex._vertex_index = idx.vertex_index;
                vertex._normal_index = idx.normal_index;
                vertex._texcoord_index = idx.texcoord_index;
                face._vertexes.push_back(vertex);
            }
            index_offset += fnum;
            shape._faces.push_back(face);
        }

        shape._material_ids.swap(shapes[i].mesh.material_ids);

        _shapes.push_back(shape);
    }
    if(smooth){
        if(_n.size() == 0 || _n.size() != _v.size()){
            std::cout << "No/In appropriate normals in obj file, use normal of vertex interpolation" << std::endl;
            _n = std::vector<Vector3f>(_v.size(), Vector3f::ZERO);
            for(size_t i = 0; i < _shapes.size(); i++){
                for(size_t j = 0; j < _shapes[i]._faces.size(); j++){
                    Face face = _shapes[i]._faces[j];
                    assert(face._vertexes.size() == 3);
                    Vector3f normal = Vector3f::cross(_v[face._vertexes[1]._vertex_index] - _v[face._vertexes[0]._vertex_index], _v[face._vertexes[2]._vertex_index] - _v[face._vertexes[0]._vertex_index]);
                    _n[face._vertexes[0]._vertex_index] += normal;
                    face._vertexes[0]._normal_index = face._vertexes[0]._vertex_index;
                    //std::cout<<face._vertexes[0]._normal_index<<std::endl;
                    _n[face._vertexes[1]._vertex_index] += normal;
                    face._vertexes[1]._normal_index = face._vertexes[1]._vertex_index;
                    _n[face._vertexes[2]._vertex_index] += normal;
                    face._vertexes[2]._normal_index = face._vertexes[2]._vertex_index;
                }
            }
            for(size_t i = 0; i < _n.size(); i++){
                _n[i].normalize();
            }
        }
    }
    //no need to parse tags
    std::cout<<"materials size:"<<materials.size()<<std::endl;

    //materials
    for(size_t i = 0; i < materials.size(); i++) {
        Material *m;
        //default material: EmpiricalMaterial
        m = new EmpiricalMaterial(
            Vector3f(materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]),
            Vector3f(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]),
            Vector3f(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]),
            Vector3f(materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]),
            Vector3f(materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]),
            materials[i].shininess,
            materials[i].ior,
            materials[i].dissolve,
            materials[i].illum);
        if(materials[i].diffuse_texname != ""){
            EmpiricalImageTexture *texture = new EmpiricalImageTexture(basepath + materials[i].diffuse_texname);
            if(materials[i].ambient_texname != ""){
                texture->loadAmbientTexture(materials[i].ambient_texname);
            }
            if(materials[i].specular_texname != ""){
                texture->loadSpecularTexture(materials[i].specular_texname);
            }
            if(materials[i].specular_highlight_texname != ""){
                texture->loadSpecularHighlightTexture(materials[i].specular_highlight_texname);
            }
            if(materials[i].bump_texname != ""){
                texture->loadBumpTexture(materials[i].bump_texname, materials[i].bump_texopt.bump_multiplier);
            }
            if(materials[i].alpha_texname != ""){
                texture->loadAlphaTexture(materials[i].alpha_texname);
            }
            if(materials[i].displacement_texname != ""){
                texture->loadDisplacementTexture(materials[i].displacement_texname);
            }

            dynamic_cast<EmpiricalMaterial *>(m)->setTexture(texture);
        }

        //PBR pipeline
        if(materials[i].roughness != 0.0f || materials[i].metallic != 0.0f || materials[i].sheen != 0.0f || materials[i].clearcoat_thickness != 0.0f || materials[i].clearcoat_roughness != 0.0f || materials[i].anisotropy != 0.0f || materials[i].anisotropy_rotation != 0.0f){
            m = new PBRMaterial(static_cast<const double>(materials[i].roughness),
                                static_cast<const double>(materials[i].metallic),
                                static_cast<const double>(materials[i].sheen),
                                static_cast<const double>(materials[i].clearcoat_thickness),
                                static_cast<const double>(materials[i].clearcoat_roughness),
                                static_cast<const double>(materials[i].anisotropy),
                                static_cast<const double>(materials[i].anisotropy_rotation));
        }
        if(materials[i].roughness_texname != ""){
            PBRTexture* texture = new PBRTexture(materials[i].roughness_texname);
            if(materials[i].emissive_texname != ""){
                texture->loadEmissiveTexture(materials[i].emissive_texname);
            }
            if(materials[i].metallic_texname != ""){
                texture->loadMetallicTexture(materials[i].metallic_texname);
            }
            if(materials[i].sheen_texname != ""){
                texture->loadSheenTexture(materials[i].sheen_texname);
            }
            if(materials[i].normal_texname != ""){
                texture->loadNormalTexture(materials[i].normal_texname);
            }
        }
        _materials.push_back(m);
    }
    
    #ifdef __DEBUG__
    std::cout << "shapes: " << _shapes.size() << std::endl;
    std::cout << "materials: " << _materials.size() << std::endl;
    std::cout << "vertexes: " << _v.size() << std::endl;
    std::cout << "normals: " << _n.size() << std::endl;
    std::cout << "texcoords: " << _uv.size() << std::endl;
    #endif
    //output material info
    
    
    if(useBVH){
    //construct all triangles
    std::vector<Triangle*> _triangles;
    for(auto shape : _shapes){
        size_t index_offset = 0;
        
        for(auto face : shape._faces){
            Material *m = this->material;
            //material overlap
            if(shape._material_ids[index_offset]!=-1){
                m = _materials[shape._material_ids[index_offset]];
            }

            if(_uv.size() <= 0){
                Triangle* triangle = new Triangle
                        (_v[face._vertexes[0]._vertex_index],
                        _v[face._vertexes[1]._vertex_index],
                        _v[face._vertexes[2]._vertex_index],
                        Vector2f(), Vector2f(), Vector2f(),
                        m);
                if(_n.size() > 0){
                    //std::cout << face._vertexes[0]._normal_index << std::endl;
                    if(face._vertexes[0]._normal_index != -1){
                        triangle->setNormals(_n[face._vertexes[0]._normal_index],
                                            _n[face._vertexes[1]._normal_index],
                                            _n[face._vertexes[2]._normal_index]);
                    }else {
                        triangle->setNormals(_n[face._vertexes[0]._vertex_index],
                                            _n[face._vertexes[1]._vertex_index],
                                            _n[face._vertexes[2]._vertex_index]);
                    }
                }
                _triangles.push_back(triangle);
            }else{
                Triangle* triangle = new Triangle
                        (_v[face._vertexes[0]._vertex_index],
                        _v[face._vertexes[1]._vertex_index],
                        _v[face._vertexes[2]._vertex_index],
                        _uv[face._vertexes[0]._texcoord_index],
                        _uv[face._vertexes[1]._texcoord_index],
                        _uv[face._vertexes[2]._texcoord_index],
                        m);
                if(_n.size() > 0){
                    if(face._vertexes[0]._normal_index != -1){
                        triangle->setNormals(_n[face._vertexes[0]._normal_index],
                                            _n[face._vertexes[1]._normal_index],
                                            _n[face._vertexes[2]._normal_index]);
                    }else {
                        triangle->setNormals(_n[face._vertexes[0]._vertex_index],
                                            _n[face._vertexes[1]._vertex_index],
                                            _n[face._vertexes[2]._vertex_index]);
                    }
                }
                _triangles.push_back(triangle);
            }

        }
        index_offset += shape._faces.size();

    }
    //construct BVH
    _root = new BVHNode(_triangles);
    }
    
    std::cout << "Constructing Finished" << std::endl;
}

bool Mesh::intersect(const Ray &r, Hit &h, float tmin) {
    if(useBVH){
        //std::cout<<"BVH intersect"<<std::endl;
        return _root->intersect(r, h, tmin);

    }
    //std::cout << "Mesh intersect" << std::endl;
    //std::cout << "Hit: "<< h.getT() << h.getTexCoord() << std::endl;
    bool result = false;
    for(size_t i = 0; i < _shapes.size(); i++){
        #ifdef __DEBUG__
        std::cout << "shape[" << i << "]" << std::endl;
        //std::cout << _shapes[i] << std::endl;
        #endif
        for(size_t j = 0; j < _shapes[i]._faces.size(); j++){
            #ifdef __DEBUG__
            std::cout << "face[" << j << "]" << std::endl;
            #endif
            Face face = _shapes[i]._faces[j];
            //std::cout << face << std::endl;
            assert(face._vertexes.size() == 3);

            #ifdef __DEBUG__
            std::cout<<"vertexes:"<<std::endl;
            std::cout<< _v[face._vertexes[0]._vertex_index] << std::endl;
            std::cout<< _v[face._vertexes[1]._vertex_index] << std::endl;
            std::cout<< _v[face._vertexes[2]._vertex_index] << std::endl;
            std::cout<<"material:"<<std::endl;
            std::cout<< _shapes[i]._material_ids[j] << std::endl;
            std::cout<< _materials.size() << std::endl;
            #endif
            Material *m = this->material;
            //material overlap
            assert(j < _shapes[i]._material_ids.size());
            if(_shapes[i]._material_ids[j]!=-1){
                m = _materials[_shapes[i]._material_ids[j]];
            }
            assert(m != nullptr);
            if(_uv.size() <= 0){
                Triangle triangle(_v[face._vertexes[0]._vertex_index],
                                  _v[face._vertexes[1]._vertex_index],
                                  _v[face._vertexes[2]._vertex_index],
                                  Vector2f(), Vector2f(), Vector2f(),
                                  m);
                if(_n.size() > 0){
                    //std::cout << face._vertexes[0]._normal_index << std::endl;
                    if(face._vertexes[0]._normal_index != -1){
                        triangle.setNormals(_n[face._vertexes[0]._normal_index],
                                            _n[face._vertexes[1]._normal_index],
                                            _n[face._vertexes[2]._normal_index]);
                    }else {
                        triangle.setNormals(_n[face._vertexes[0]._vertex_index],
                                            _n[face._vertexes[1]._vertex_index],
                                            _n[face._vertexes[2]._vertex_index]);
                    }
                }
                result |= triangle.intersect(r, h, tmin);
            }else{
                Triangle triangle(_v[face._vertexes[0]._vertex_index],
                                _v[face._vertexes[1]._vertex_index],
                                _v[face._vertexes[2]._vertex_index],
                                _uv[face._vertexes[0]._texcoord_index],
                                _uv[face._vertexes[1]._texcoord_index],
                                _uv[face._vertexes[2]._texcoord_index],
                                m);
                if(_n.size() > 0){
                    if(face._vertexes[0]._normal_index != -1){
                        triangle.setNormals(_n[face._vertexes[0]._normal_index],
                                            _n[face._vertexes[1]._normal_index],
                                            _n[face._vertexes[2]._normal_index]);
                    }else {
                        triangle.setNormals(_n[face._vertexes[0]._vertex_index],
                                            _n[face._vertexes[1]._vertex_index],
                                            _n[face._vertexes[2]._vertex_index]);
                    }
                }
                result |= triangle.intersect(r, h, tmin);
            }
        }
    }
    //std::cout << "Hit: "<< h.getT() << std::endl;
    //std::cout << "texCoord after all intersect: " << h.getTexCoord() << std::endl;
    return result;
}

TraditionalMesh::TraditionalMesh(const char *filename, Material *m) {
    material = m;

    std::cout << "Loading " << filename << std::endl;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    bool triangulate = true;

    std::string warn;
    std::string err;
    std::string inputfile = filename;
    
    string::size_type iPos = (inputfile .find_last_of('\\') + 1) == 0 ?  inputfile .find_last_of('/') + 1: inputfile .find_last_of('\\') + 1 ;
    string basepath = inputfile.substr(0,iPos);//获取文件路径

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename,
                                basepath.c_str(), triangulate);

    if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
    }

    if (!ret) {
    printf("Failed to load/parse .obj.\n");
    }

    //parse attrib, shapes and materials from tinyobjloader to mesh
    //attrib
    for(int i = 0; i < attrib.vertices.size(); i+=3){
        Vector3f vertex(attrib.vertices[i], attrib.vertices[i+1], attrib.vertices[i+2]);
        v.push_back(vertex);
    }
    //no normal
    //computed in computeNormal()
    //no texcoord

    //shapes
    //only parse triangle indexes
    for(int i = 0; i < shapes.size(); i++){
        for(int j = 0; j < shapes[i].mesh.indices.size(); j+=3){
            TriangleIndex triIndex;
            triIndex[0] = shapes[i].mesh.indices[j].vertex_index;
            triIndex[1] = shapes[i].mesh.indices[j+1].vertex_index;
            triIndex[2] = shapes[i].mesh.indices[j+2].vertex_index;
            t.push_back(triIndex);
        }
    }
    computeNormal();

}

bool TraditionalMesh::intersect(const Ray &r, Hit &h, float tmin) {

    // Optional: Change this brute force method into a faster one.
    bool result = false;
    for (int triId = 0; triId < (int) t.size(); ++triId) {
        TriangleIndex& triIndex = t[triId];
        Triangle triangle(v[triIndex[0]], v[triIndex[1]], v[triIndex[2]],
                            Vector2f(), Vector2f(), Vector2f(), material);
        triangle.normal = n[triId];
        result |= triangle.intersect(r, h, tmin);
    }
    return result;
}

void TraditionalMesh::computeNormal() {
    n.resize(t.size());
    for (int triId = 0; triId < (int) t.size(); ++triId) {
        TriangleIndex& triIndex = t[triId];
        Vector3f a = v[triIndex[1]] - v[triIndex[0]];
        Vector3f b = v[triIndex[2]] - v[triIndex[0]];
        b = Vector3f::cross(a, b);
        n[triId] = b / b.length();
    }
}
