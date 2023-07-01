#include <vecmath.h>
#include <cmath>
#include <cassert>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <memory>

#include "../include/image.hpp"
#include "../include/ray.hpp"
#include "../include/camera.hpp"
#include "../include/utils.hpp"
#include "../include/scene_parser.hpp"
#include "../include/light.hpp"
#include "../include/material.hpp"
#include "../include/object3d.hpp"
#include "../include/group.hpp"
#include "../include/triangle.hpp"
#include "../include/transform.hpp"
#include "../include/mesh.hpp"
#include "../include/curve.hpp"
#include "../include/hit.hpp"
#include "../include/render.hpp"

#define __DEBUG__

    bool smooth = false; bool useBVH = true;

void drawtangent(RgbImage* image, Curve* curve, double t, Vector3f color){
  Vector3f p = curve->evaluate(t);
  Vector3f d = curve->evaluateDerivative(t);
  d.normalize();
  //image->SetPixel((int)(p.x() * (image->Width()-1)), (int)(p.y() * (image->Height()-1)), color);
  for(float i = 0; i < 0.1; i += 0.001){
    //draw tangent line
    Vector3f p1 = p + i * d;
    if(p1.x() >= 0 && p1.x() <= 1 && p1.y() >= 0 && p1.y() <= 1)
    image->SetPixel((int)(p1.x() * (image->Width()-1)), (int)(p1.y() * (image->Height()-1)), color);
  }

}

void drawline(RgbImage* image, Vector3f start, Vector3f end, Vector3f color){
  Vector3f d = end - start;
  d.normalize();
  for(float i = 0; i < 1; i += 0.001){
    //draw tangent line
    Vector3f p1 = start + i * d;
    if(p1.x() >= 0 && p1.x() <= 1 && p1.y() >= 0 && p1.y() <= 1)
    image->SetPixel((int)(p1.x() * (image->Width()-1)), (int)(p1.y() * (image->Height()-1)), color);
  }
}

int main(int argc, char *argv[]){ 
    //parse arguments
    /*default values:
      usage: --width 1024 --height 768 --samples 100 
      --output ../output/result.png --input ../test/scene.txt
      --quality 100 --rendermode 0
      --depth 5 --threads 28 --depth-of-field 0
    */
    /* @param
      width: width of the image
      height: height of the image
      samples: number of samples per pixel
      output: output file name and path should be an image format
      input: input file name and path should be a scene file (txt)
      quality: quality of the image (0-100)
      rendermode:
      0: path tracing
      1: schostic progressive photon mapping
      2: vertex connection and merging
      3: bidirectional path tracing
      4: metropolis light transport
      5: ray casting

      depth: depth of the path tracing / iterations of the photon mapping
      threads: number of threads while rendering

      depth-of-field:
      0: off
      1: on
    */
    /* if(argc < 2){
      std::cout << "Usage: " << argv[0] << " --width 1024 --height 768 --samples 100 --output ../output/result.png --input ../test/scene.txt --quality 100 --rendermode 0 --depth 5 --threads 28 --depth-of-field 0 --aperture 1 --focus-length 5" << std::endl;
      return 0;
    } */
    int width = 1024, height = 1024, samples = 10000, quality = 100, depth = 50, threads = 1;
    std::string output = "../output/default.png", input = "../testcases/sppm.txt";
    RENDER = SPPM; bool DOF = false; float aperture = 1.0, focus_length = 5.0;
    parse_arg(argc, argv, width, height, samples, threads, depth, quality, input, output, DOF, aperture, focus_length);
    #ifdef __DEBUG__
    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;
    std::cout << "samples: " << samples << std::endl;
    std::cout << "output: " << output << std::endl;
    std::cout << "input: " << input << std::endl;
    std::cout << "quality: " << quality << std::endl;
    std::cout << "rendermode: " << RENDER << std::endl;
    std::cout << "depth: " << depth << std::endl;
    std::cout << "threads: " << threads << std::endl;
    std::cout << "depth-of-field: " << DOF << std::endl;
    std::cout << "aperture: " << aperture << std::endl;
    std::cout << "focus-length: " << focus_length << std::endl;
    #endif

    //cli arguments: x1 y1 x2 y2 
    //x1 y1: start point
    //x2 y2: end point
    //float x3 = atof(argv[1]), y3 = atof(argv[2]), x2 = atof(argv[3]), y2 = atof(argv[4]);
    //int it = atoi(argv[5]);
    //Vector3f start(x3, y3, 0), end(x2, y2, 0);
    //解析场景文件，获得场景信息
    SceneParser sceneParser(input.c_str());
    RgbImage *image ;
    RendererFactory factory;
    std::unique_ptr<Renderer> renderer;

    // 选择渲染方式并创建相应的渲染器
    try {
        renderer = factory.createRenderer();
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // 使用渲染器进行渲染
    renderer->render(sceneParser, image, samples, threads, depth, DOF, aperture, focus_length);

    
    assert(image!=nullptr);
    image->SaveImage(output.c_str());
    std::cout << "Image saved." << std::endl;
    #ifdef __PICTURE__DEBUG__
    std::string outputFile = output;
    std::string outputFileBmp = output.substr(0, output.size()-3) + "bmp";
    std::string outputFilePng = output.substr(0, output.size()-3) + "png";
    std::string outputFilePpm = output.substr(0, output.size()-3) + "ppm";
    std::string outputFileTga = output.substr(0, output.size()-3) + "tga";
    std::string outputFileJpg = output.substr(0, output.size()-3) + "jpg";
    std::string outputFileHdr = output.substr(0, output.size()-3) + "hdr";
    image->SaveImage(outputFile.c_str());
    image->SaveImage(outputFileBmp.c_str());
    image->SaveImage(outputFilePng.c_str());
    image->SaveImage(outputFilePpm.c_str());
    image->SaveImage(outputFileTga.c_str());
    image->SaveImage(outputFileJpg.c_str());
    image->SaveImage(outputFileHdr.c_str());
    #endif
    return 0;
 } 