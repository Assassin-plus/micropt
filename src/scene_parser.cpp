#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "scene_parser.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "material.hpp"
#include "object3d.hpp"
#include "mesh.hpp"
#include "classical_object.hpp"
#include "transform.hpp"
#include "group.hpp"
//#include "sphere.hpp"
//#include "plane.hpp"

#include "curve.hpp"
#include "texture.hpp"
// support PBR pipeline
// diffuse, specular, roughness, metallic, normal, ao, displacement


#define DegreesToRadians(x) ((M_PI * x) / 180.0f)

SceneParser::SceneParser(const char *filename) {

    // initialize some reasonable default values
    group = nullptr;
    camera = nullptr;
    background_color = Vector3f(0.5, 0.5, 0.5);
    num_lights = 0;
    lights = std::vector<Light *>();
    num_materials = 0;
    materials = std::vector<Material *>();
    current_material = nullptr;
    num_curves = 0;
    curves = std::vector<Curve *>();
    num_textures = 0;
    textures = std::vector<Texture *>();

    // parse the file
    assert(filename != nullptr);
    const char *ext = &filename[strlen(filename) - 4];

    if (strcmp(ext, ".txt") != 0) {
        printf("wrong file name extension %s\n", ext);
        exit(0);
    }
    file = fopen(filename, "r");

    if (file == nullptr) {
        printf("cannot open scene file %s\n", filename);
        exit(0);
    }
    parseFile();
    fclose(file);
    file = nullptr;

    if (num_lights == 0) {
        printf("WARNING:    No lights specified\n");
    }

    #ifdef __DEBUG__
    std::cout << "background_color: " << background_color << std::endl;
    std::cout << "num_lights: " << num_lights << std::endl;
    std::cout << "num_materials: " << num_materials << std::endl;
    std::cout << "num_curves: " << num_curves << std::endl;
    std::cout << "num_textures: " << num_textures << std::endl;
    #endif
}

SceneParser::~SceneParser() {

    delete group;
    delete camera;

    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    materials.clear();
    for (i = 0; i < num_lights; i++) {
        delete lights[i];
    }
    lights.clear();
}

// ====================================================================
// ====================================================================

void SceneParser::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        } else if (!strcmp(token, "Background")) {
            parseBackground();
        } else if (!strcmp(token, "Lights")) {
            parseLights();
        } else if (!strcmp(token, "Materials")) {
            parseMaterials();
        } else if (!strcmp(token, "Group")) {
            group = parseGroup();
        } else if (!strcmp(token, "Textures")) {
            parseTextures();
        } else{
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert (!strcmp(token, "up"));
    Vector3f up = readVector3f();
    getToken(token);
    assert (!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token);
    assert (!strcmp(token, "width"));
    int width = readInt();
    getToken(token);
    assert (!strcmp(token, "height"));
    int height = readInt();
    getToken(token);
    assert (!strcmp(token, "}"));
    camera = new PerspectiveCamera(center, direction, up, width, height, angle_radians);
}

void SceneParser::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        } else if (!strcmp(token, "color")) {
            background_color = readVector3f();
        } else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = std::vector<Light *>(num_lights);
    // read in the objects
    int count = 0;
    while (num_lights > count) {
        getToken(token);
        if (strcmp(token, "DirectionalLight") == 0) {
            lights[count] = parseDirectionalLight();
        } else if (strcmp(token, "PointLight") == 0) {
            lights[count] = parsePointLight();
        } else if (strcmp(token, "AreaLight") == 0) {
            lights[count] = parseAreaLight();
        } else if (strcmp(token, "SpotLight") == 0) {
            lights[count] = parseSpotLight();
        } else {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}

Light *SceneParser::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}

Light *SceneParser::parsePointLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "position"));
    Vector3f position = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new PointLight(position, color);
}

Light *SceneParser::parseAreaLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "position"));
    Vector3f position = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "normal"));
    Vector3f normal = readVector3f();
    getToken(token);
    assert (!strcmp(token, "area"));
    float area = readFloat();
    getToken(token);
    if(!strcmp(token, "texture")) {
        getToken(token);
        assert (!strcmp(token, "filename"));
        char filename[MAX_PARSER_TOKEN_LENGTH];
        getToken(filename);
        Texture *texture = new EmpiricalImageTexture(filename);
        getToken(token);
        assert (!strcmp(token, "}"));
        return new AreaLight(position, color, normal, area, texture);
    }else if(!strcmp(token, "}")){
        return new AreaLight(position, color, normal, area);
    }
}

Light *SceneParser::parseSpotLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "position"));
    Vector3f position = readVector3f();
    getToken(token);
    assert (!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "angle"));
    float angle = readFloat();
    getToken(token);
    assert (!strcmp(token, "intensity"));
    float intensity = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new SpotLight(position, direction, color, angle, intensity);
}

// ====================================================================
// ====================================================================

void SceneParser::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = std::vector<Material *>(num_materials);
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if (!strcmp(token, "Material") ||
            !strcmp(token, "PhongMaterial")) {
            materials[count] = parseMaterial();
        } else if (!strcmp(token, "GIMaterial")){
            materials[count] = parseGIMaterial();
        } else {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}

Material *SceneParser::parseGIMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vector3f diffuseColor(1, 1, 1), specularColor(0, 0, 0), emissionColor(0, 0, 0);
    BRDFType type = BRDFType::DIFFUSE;
    getToken(token);
    assert (!strcmp(token, "{"));
    while(true) {
        getToken(token);
        if(!strcmp(token, "diffuseColor")) {
            diffuseColor = readVector3f();
        } else if(!strcmp(token, "specularColor")) {
            specularColor = readVector3f();
        } else if(!strcmp(token, "emissionColor")) {
            emissionColor = readVector3f();
        } else if(!strcmp(token, "DIFFUSE")){
            type = BRDFType::DIFFUSE;
        } else if(!strcmp(token, "SPECULAR")){
            type = BRDFType::SPECULAR;
        } else if(!strcmp(token, "REFRACTION")){
            type = BRDFType::REFRACTION;
        } else if(!strcmp(token, "EMISSION")){
            type = BRDFType::EMISSION;
        } else if(!strcmp(token, "}")){
            break;
        } else {
            printf("Unknown token in parseGIMaterial: '%s'\n", token);
            exit(0);
        }
    }
    auto *answer = new DiscreteMaterial(diffuseColor, specularColor, emissionColor, type);
    return answer;
}

Material *SceneParser::parseMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    filename[0] = 0;
    Vector3f diffuseColor(1, 1, 1), specularColor(0, 0, 0);
    float shininess = 0;
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "diffuseColor") == 0) {
            diffuseColor = readVector3f();
        } else if (strcmp(token, "specularColor") == 0) {
            specularColor = readVector3f();
        } else if (strcmp(token, "shininess") == 0) {
            shininess = readFloat();
        } else if (strcmp(token, "texture") == 0) {
            // Optional: read in texture and draw it.
            getToken(filename);
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    auto *answer = new PhongMaterial(diffuseColor, specularColor, shininess);
    return answer;
}



// ====================================================================
// ====================================================================

Object3D *SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D *answer = nullptr;
    if (!strcmp(token, "Group")) {
        answer = (Object3D *) parseGroup();
    } else if (!strcmp(token, "Sphere")) {
        answer = (Object3D *) parseSphere();
    } else if (!strcmp(token, "Plane")) {
        answer = (Object3D *) parsePlane();
    } else if (!strcmp(token, "Triangle")) {
        answer = (Object3D *) parseTriangle();
    } else if (!strcmp(token, "TriangleMesh")) {
        answer = (Object3D *) parseTriangleMesh();
    } else if (!strcmp(token, "Transform")) {
        answer = (Object3D *) parseTransform();
    } else if (!strcmp(token, "Curves")) {
        answer = (Object3D *) parseCurves();
    } else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group *SceneParser::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));

    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numObjects"));
    int num_objects = readInt();

    auto *answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert (index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        } else {
            Object3D *object = parseObject(token);
            assert (object != nullptr);
            if(dynamic_cast<Curve*>(object) != nullptr) {
                for(int i = 0; i < num_curves; i++) {
                    object = curves[i];
                    answer->addObject(count, object);
                    count++;
                }
            }
            answer->addObject(count, object);

            count++;
        }
    }
    getToken(token);
    assert (!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere *SceneParser::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Sphere(center, radius, current_material);
}


Plane *SceneParser::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "normal"));
    Vector3f normal = readVector3f();
    getToken(token);
    assert (!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Plane(normal, offset, current_material);
}


Triangle *SceneParser::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "vertex0"));
    Vector3f v0 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "vertex1"));
    Vector3f v1 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "vertex2"));
    Vector3f v2 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Triangle(v0, v1, v2, Vector2f::ZERO, Vector2f::ZERO, Vector2f::ZERO, current_material);
}

Mesh *SceneParser::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "obj_file"));
    getToken(filename);
    getToken(token);
    assert (!strcmp(token, "}"));
    const char *ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));
    Mesh *answer = new Mesh(filename, current_material);

    return answer;
}


Transform *SceneParser::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix4f matrix = Matrix4f::identity();
    Object3D *object = nullptr;
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);

    while (true) {
        if (!strcmp(token, "Scale")) {
            Vector3f s = readVector3f();
            matrix = matrix * Matrix4f::scaling(s[0], s[1], s[2]);
        } else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix = matrix * Matrix4f::uniformScaling(s);
        } else if (!strcmp(token, "Translate")) {
            matrix = matrix * Matrix4f::translation(readVector3f());
        } else if (!strcmp(token, "XRotate")) {
            matrix = matrix * Matrix4f::rotateX(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "YRotate")) {
            matrix = matrix * Matrix4f::rotateY(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "ZRotate")) {
            matrix = matrix * Matrix4f::rotateZ(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "Rotate")) {
            getToken(token);
            assert (!strcmp(token, "{"));
            Vector3f axis = readVector3f();
            float degrees = readFloat();
            float radians = DegreesToRadians(degrees);
            matrix = matrix * Matrix4f::rotation(axis, radians);
            getToken(token);
            assert (!strcmp(token, "}"));
        } else if (!strcmp(token, "Matrix4f")) {
            Matrix4f matrix2 = Matrix4f::identity();
            getToken(token);
            assert (!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2(i, j) = v;
                }
            }
            getToken(token);
            assert (!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        } else {
            // otherwise this must be an object,
            // and there are no more transformations
            object = parseObject(token);
            break;
        }
        getToken(token);
    }

    assert(object != nullptr);
    getToken(token);
    assert (!strcmp(token, "}"));
    return new Transform(matrix, object);
}
// ====================================================================
// ====================================================================

Curve* SceneParser::parseCurves(){
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numCurves"));
    num_curves = readInt();
    curves = std::vector<Curve *>(num_curves);
    // read in the objects
    int count = 0;
    while (num_curves > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert (index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        } else {
            if (!strcmp(token, "BezierCurve")) {
                curves[count] = parseBezierCurve();
            } else if (!strcmp(token, "BSplineCurve")) {
                curves[count] = parseBSplineCurve();
            } else if (!strcmp(token, "CatmullRomCurve")) {
                curves[count] = parseCatmullRomCurve();
            } else if (!strcmp(token, "BSplineSurface")) {
                curves[count] = parseBSplineSurface();
            } else if (!strcmp(token, "BezierSurface")) {
                curves[count] = parseBezierSurface();
            } else {
                printf("Unknown token in parseCurve: '%s'\n", token);
                exit(0);
            }
            count++;
        }
    }
    getToken(token);
    assert (!strcmp(token, "}"));
    return curves.size() > 0 ? curves[0] : nullptr;
}

Curve *SceneParser::parseBezierCurve(){
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector<Vector3f> controls;
    getToken(token);
    assert(!strcmp(token, "["));
    while (true) {
        if(!strcmp(token, "]")){
            break;
        }
        Vector3f control = readVector3f();
        controls.push_back(control);
    }
    getToken(token);
    assert (!strcmp(token, "axis"));
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "point1"));
    Vector3f point1 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "point2"));
    Vector3f point2 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    getToken(token);
    assert (!strcmp(token, "}"));

    return new BezierCurve(controls, point1, point2);
}

Curve *SceneParser::parseBSplineCurve(){
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector<Vector3f> controls;
    getToken(token);
    assert(!strcmp(token, "["));
    while (true) {
        Vector3f control = readVector3f();
        if(control.x() != control.x()){
            break;
        }
        controls.push_back(control);
    }
    getToken(token);
    assert (!strcmp(token, "]"));
    getToken(token);
    assert (!strcmp(token, "knots"));
    std::vector<float> knots;
    getToken(token);
    assert(!strcmp(token, "["));
    while (true) {
        float knot = readFloat();
        if(knot != knot){
            break;
        }
        knots.push_back(knot);
    }
    getToken(token);
    assert (!strcmp(token, "]"));
    getToken(token);
    assert (!strcmp(token, "axis"));
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "point1"));
    Vector3f point1 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "point2"));
    Vector3f point2 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    getToken(token);
    assert (!strcmp(token, "}"));

    return new BSplineCurve(controls, knots, point1, point2, current_material);

}

Curve *SceneParser::parseCatmullRomCurve(){
    // params: controls, tau, axis
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector<Vector3f> controls;
    getToken(token);
    assert(!strcmp(token, "["));
    while (true) {
        if(!strcmp(token, "]")){
            break;
        }
        Vector3f control = readVector3f();
        controls.push_back(control);
    }
    getToken(token);
    assert (!strcmp(token, "tension"));
    float tension = readFloat();
    getToken(token);
    assert (!strcmp(token, "axis"));
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "point1"));
    Vector3f point1 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "point2"));
    Vector3f point2 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    getToken(token);
    assert (!strcmp(token, "}"));

    return new CatmullRomCurve(controls, tension, point1, point2);
}

Surface *SceneParser::parseBezierSurface(){
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector<std::vector<Vector3f>> controls;
    getToken(token);
    assert(!strcmp(token, "["));
    while (true) {
        if(!strcmp(token, "]")){
            break;
        }
        std::vector<Vector3f> row;
        getToken(token);
        assert(!strcmp(token, "["));
        while (true) {
            if(!strcmp(token, "]")){
                break;
            }
            Vector3f control = readVector3f();
            row.push_back(control);
        }
        controls.push_back(row);
    }
    getToken(token);
    assert (!strcmp(token, "}"));

    return new BezierSurface(controls);
}

Surface *SceneParser::parseBSplineSurface(){
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector<std::vector<Vector3f>> controls;
    getToken(token);
    assert(!strcmp(token, "["));
    while (true) {
        if(!strcmp(token, "]")){
            break;
        }
        std::vector<Vector3f> row;
        getToken(token);
        assert(!strcmp(token, "["));
        while (true) {
            if(!strcmp(token, "]")){
                break;
            }
            Vector3f control = readVector3f();
            row.push_back(control);
        }
        controls.push_back(row);
    }
    //get knots
    getToken(token);
    assert (!strcmp(token, "uknots"));
    std::vector<float> uknots;
    getToken(token);
    assert(!strcmp(token, "["));
    while (true) {
        if(!strcmp(token, "]")){
            break;
        }
        float knot = readFloat();
        uknots.push_back(knot);
    }
    getToken(token);
    assert (!strcmp(token, "vknots"));
    std::vector<float> vknots;
    getToken(token);
    assert(!strcmp(token, "["));
    while (true) {
        if(!strcmp(token, "]")){
            break;
        }
        float knot = readFloat();
        vknots.push_back(knot);
    }
    getToken(token);
    assert (!strcmp(token, "}"));
    
    return new BSplineSurface(controls, uknots, vknots);
}

// ====================================================================
// ====================================================================

void SceneParser::parseTextures(){
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numTextures"));
    num_textures = readInt();
    textures = std::vector<Texture *>(num_textures);
    // read in the objects
    int count = 0;
    while (num_textures > count) {
        getToken(token);
        if (!strcmp(token, "Texture")) {
            textures[count] = parseTexture();
        } else {
            printf("Unknown token in parseTexture: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}

Texture *SceneParser::parseTexture(){
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    filename[0] = 0;
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "type"));
    getToken(token);
    assert (!strcmp(token, "image"));
    getToken(token);
    assert (!strcmp(token, "filename"));
    getToken(filename);
    getToken(token);
    assert (!strcmp(token, "}"));
    Texture *answer = new EmpiricalImageTexture(filename);

    return answer;
}

// ====================================================================
// ====================================================================

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert (file != nullptr);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vector3f SceneParser::readVector3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vector3f\n");
        return Vector3f(nan(""), nan(""), nan(""));
        assert (0);
    }
    return Vector3f(x, y, z);
}


float SceneParser::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        return nan("");
        assert (0);
    }
    return answer;
}


int SceneParser::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert (0);
    }
    return answer;
}
