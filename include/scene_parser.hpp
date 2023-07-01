#ifndef __SCENE_PARSER_H__
#define __SCENE_PARSER_H__

#include <cassert>
#include <vecmath.h>

class Camera;
class Light;
class Material;
class Object3D;
class Group;
class Sphere;
class Plane;
class Triangle;
class Transform;
class Mesh;
class Curve;
class Surface;
class Texture;

#define MAX_PARSER_TOKEN_LENGTH 1024

class SceneParser {
public:

    SceneParser() = delete;
    SceneParser(const char *filename);

    ~SceneParser();

    Camera *getCamera() const {
        return camera;
    }

    Vector3f getBackgroundColor() const {
        return background_color;
    }

    int getNumLights() const {
        return num_lights;
    }

    Light *getLight(int i) const {
        assert(i >= 0 && i < num_lights);
        return lights[i];
    }

    std::vector<Light *> getLights() const {
        return lights;
    }

    int getNumMaterials() const {
        return num_materials;
    }

    Material *getMaterial(int i) const {
        assert(i >= 0 && i < num_materials);
        return materials[i];
    }

    Group *getGroup() const {
        return group;
    }

private:

    void parseFile();
    void parsePerspectiveCamera();
    void parseBackground();
    void parseLights();
    Light *parsePointLight();
    Light *parseDirectionalLight();
    Light *parseAreaLight();
    Light *parseSpotLight();
    void parseMaterials();
    Material *parseMaterial();
    Material *parseGIMaterial();
    Object3D *parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    Group *parseGroup();
    Sphere *parseSphere();
    Plane *parsePlane();
    Triangle *parseTriangle();
    Mesh *parseTriangleMesh();
    Transform *parseTransform();
    Curve *parseCurves();
    Curve *parseBezierCurve();
    Curve *parseBSplineCurve();
    Curve *parseCatmullRomCurve();
    Surface *parseBezierSurface();
    Surface *parseBSplineSurface();
    void parseTextures();
    Texture *parseTexture();

    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);

    Vector3f readVector3f();

    float readFloat();
    int readInt();

    FILE *file;
    Camera *camera;
    Vector3f background_color;
    int num_lights;
    std::vector<Light *> lights;
    int num_materials;
    std::vector<Material *> materials;
    Material *current_material;
    Group *group;
    int num_curves;
    std::vector<Curve *> curves;
    int num_textures;
    std::vector<Texture *> textures;
};

#endif // SCENE_PARSER_H
