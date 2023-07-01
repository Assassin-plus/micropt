#include "../include/utils.hpp"

RenderMode RENDER;
SamplerType SAMPLER;
FilterType FILTER;

void parse_arg(int argc, char *argv[], int& width, int& height, int& samples, int& threads, int& depth, int& quality, std::string& input, std::string& output, bool& DOF, float& aperture, float& focus_length){
    for(int i = 1; i < argc; i++){
      if(std::string(argv[i]) == "--width"){
        width = atoi(argv[i+1]);
      }
      else if(std::string(argv[i]) == "--height"){
        height = atoi(argv[i+1]);
      }
      else if(std::string(argv[i]) == "--samples"){
        samples = atoi(argv[i+1]);
      }
      else if(std::string(argv[i]) == "--output"){
        output = argv[i+1];
      }
      else if(std::string(argv[i]) == "--input"){
        input = argv[i+1];
      }
      else if(std::string(argv[i]) == "--quality"){
        quality = atoi(argv[i+1]);
      }
      else if(std::string(argv[i]) == "--rendermode"){
        int mode = atoi(argv[i+1]);
        switch(mode){
          case 0:
            RENDER = PT;
            break;
          case 1:
            RENDER = SPPM;
            break;
          case 2:
            RENDER = VCM;
            break;
          case 3:
            RENDER = BDPT;
            break;
          case 4:
            RENDER = MLT;
            break;
          case 5:
            RENDER = RC;
            break;
          default:
            std::cout << "Invalid render mode" << std::endl;
        }
      }
      else if(std::string(argv[i]) == "--depth"){
        depth = atoi(argv[i+1]);
      }
      else if(std::string(argv[i]) == "--threads"){
        threads = atoi(argv[i+1]);
      }
      else if(std::string(argv[i]) == "--depth-of-field"){
        int dof = atoi(argv[i+1]);
        switch(dof){
          case 0:
            DOF = false;
            break;
          case 1:
            DOF = true;
            break;
          default:
            std::cout << "Invalid depth of field mode" << std::endl;
        }
      }
      else if(std::string(argv[i]) == "--sampler"){
        int sampler = atoi(argv[i+1]);
        switch(sampler){
          case 0:
            SAMPLER = RANDOM;
            break;
          case 1:
            SAMPLER = STRATIFIED;
            break;
          case 2:
            SAMPLER = HALTON;
            break;
          case 3:
            SAMPLER = SOBOL;
            break;
          default:
            std::cout << "Invalid sampler mode" << std::endl;
        }
      }
      else if(std::string(argv[i]) == "--filter"){
        int filter = atoi(argv[i+1]);
        switch(filter){
          case 0:
            FILTER = BOX;
            break;
          case 1:
            FILTER = GAUSSIAN;
            break;
          case 2:
            FILTER = MITCHELL;
            break;
          case 3:
            FILTER = LANCZOS;
            break;
          default:
            std::cout << "Invalid filter mode" << std::endl;
        }
      }
      else if(std::string(argv[i]) == "--aperture"){
        aperture = atof(argv[i+1]);
      }
      else if(std::string(argv[i]) == "--focus-length"){
        focus_length = atof(argv[i+1]);
      }
      else if(std::string(argv[i]) == "--smooth"){
        int s = atoi(argv[i+1]);
        if(s == 0) smooth = false;
        else smooth = true;
      }
      else if(std::string(argv[i]) == "--bvh"){
        int s = atoi(argv[i+1]);
        if(s == 0) useBVH = false;
        else useBVH = true;
      }
    }
}

void mapToImage(int& x, int& y, int width, int height){
  while (x < 0) x += width;
  while (y < 0) y += height;
  while (x >= width) x -= width;
  while (y >= height) y -= height;
}

Vector2f mapToImage(Vector2f p, int width, int height){
  while (p.x() < 0) p.x() += width;
  while (p.y() < 0) p.y() += height;
  while (p.x() >= width) p.x() -= width;
  while (p.y() >= height) p.y() -= height;
  return p;
}