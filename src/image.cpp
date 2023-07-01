#include <cassert>
#include <vecmath.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

//#include "../include/svpng.hpp"
//#include "../include/lodepng.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"
#include "../include/image.hpp"
//a universal image class with original interface
//add interface for stb_image and stb_image_write
//add middle layer for debugging

//avoid multiple definition of stb_image and stb_image_write

GrayImage *GrayImage::LoadImage(const char *filename) {
    unsigned char* tmp = stbi_load(filename, &width, &height, NULL, 1);
    if(!tmp) {
        printf("load gray image failed\n");
        return nullptr;
    }else{
        data = new float[width * height];
        for(int i = 0; i < width * height; ++i) {
            data[i] = tmp[i];
        }
        return this;
    }
}

void GrayImage::SavePng(const char *filename) const {
    if(!stbi_write_png(filename, width, height, 1, data, width)) {
        printf("save gray image as png failed\n");
    }
}

void GrayImage::SaveBmp(const char *filename) const {
    if(!stbi_write_bmp(filename, width, height, 1, data)) {
        printf("save gray image as bmp failed\n");
    }
}
void GrayImage::SaveTga(const char *filename) const {
    if(!stbi_write_tga(filename, width, height, 1, data)) {
        printf("save gray image as tga failed\n");
    }
}
void GrayImage::SavePpm(const char *filename) const {
    if(!stbi_write_tga(filename, width, height, 1, data)) {
        printf("save gray image as ppm failed\n");
    }
}
void GrayImage::SaveJpg(const char *filename, int quality=100) const {
    if(!stbi_write_jpg(filename, width, height, 1, data, quality)) {
        printf("save gray image as jpg failed\n");
    }
}
void GrayImage::SaveHdr(const char *filename) const {
    if(!stbi_write_hdr(filename, width, height, 1, (float *)data)) {
        printf("save gray image as hdr failed\n");
    }
}
void GrayImage::SaveImage(const char *filename) const {
    std::string str(filename);
    std::string suffix = str.substr(str.find_last_of('.') + 1);
    if(suffix == "png") {
        SavePng(filename);
    } else if(suffix == "bmp") {
        SaveBmp(filename);
    } else if(suffix == "tga") {
        SaveTga(filename);
    } else if(suffix == "ppm") {
        SavePpm(filename);
    } else if(suffix == "jpg") {
        SaveJpg(filename);
    } else if(suffix == "hdr") {
        SaveHdr(filename);
    } else {
        printf("unsupported image format\n");
    }
}

GrayAlphaImage *GrayAlphaImage::LoadImage(const char *filename) {
    unsigned char *temp = stbi_load(filename, &width, &height, NULL, 2);
    if(!temp) {
        printf("load grayalpha image failed\n");
        return nullptr;
    }else{
        data = new Vector2f[width * height];
        for(int i = 0; i < width * height; ++i) {
            data[i].x() = temp[i * 2];
            data[i].y() = temp[i * 2 + 1];
        }
        return this;
    }
}

void GrayAlphaImage::SavePng(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 2];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 2] = data[i].x();
        temp[i * 2 + 1] = data[i].y();
    }
    if(!stbi_write_png(filename, width, height, 2, temp, width * 2)) {
        printf("save grayalpha image as png failed\n");
    }
    delete[] temp;
}
void GrayAlphaImage::SaveBmp(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 2];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 2] = data[i].x();
        temp[i * 2 + 1] = data[i].y();
    }
    if(!stbi_write_bmp(filename, width, height, 2, temp)) {
        printf("save grayalpha image as bmp failed\n");
    }
    delete[] temp;
}
void GrayAlphaImage::SaveTga(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 2];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 2] = data[i].x();
        temp[i * 2 + 1] = data[i].y();
    }
    if(!stbi_write_tga(filename, width, height, 2, temp)) {
        printf("save grayalpha image as tga failed\n");
    }
    delete[] temp;
}
void GrayAlphaImage::SavePpm(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 2];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 2] = data[i].x();
        temp[i * 2 + 1] = data[i].y();
    }
    if(!stbi_write_tga(filename, width, height, 2, temp)) {
        printf("save grayalpha image as ppm failed\n");
    }
    delete[] temp;
}
void GrayAlphaImage::SaveJpg(const char *filename, int quality=100) const {
    unsigned char *temp = new unsigned char[width * height * 2];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 2] = data[i].x();
        temp[i * 2 + 1] = data[i].y();
    }
    if(!stbi_write_jpg(filename, width, height, 2, temp, quality)) {
        printf("save grayalpha image as jpg failed\n");
    }
    delete[] temp;
}
void GrayAlphaImage::SaveHdr(const char *filename) const {
    float *temp = new float[width * height * 2];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 2] = data[i].x();
        temp[i * 2 + 1] = data[i].y();
    }
    if(!stbi_write_hdr(filename, width, height, 2, temp)) {
        printf("save grayalpha image as hdr failed\n");
    }
    delete[] temp;
}
void GrayAlphaImage::SaveImage(const char *filename) const {
    std::string str(filename);
    std::string suffix = str.substr(str.find_last_of('.') + 1);
    if(suffix == "png") {
        SavePng(filename);
    } else if(suffix == "bmp") {
        SaveBmp(filename);
    } else if(suffix == "tga") {
        SaveTga(filename);
    } else if(suffix == "ppm") {
        SavePpm(filename);
    } else if(suffix == "jpg") {
        SaveJpg(filename);
    } else if(suffix == "hdr") {
        SaveHdr(filename);
    } else {
        printf("unsupported image format\n");
    }
}

RgbImage *RgbImage::LoadImage(const char *filename) {
    unsigned char *temp = stbi_load(filename, &width, &height, NULL, 3);
    if(!temp) {
        printf("load rgb image failed\n");
        return nullptr;
    }else{
        data = new Vector3f[width * height];
        for(int i = 0; i < width * height; ++i) {
            data[i].x() = temp[i * 3];
            data[i].y() = temp[i * 3 + 1];
            data[i].z() = temp[i * 3 + 2];
        }
        return this;
    }
}
void RgbImage::SavePng(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 3];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 3] = data[i].x();
        temp[i * 3 + 1] = data[i].y();
        temp[i * 3 + 2] = data[i].z();
    }
    if(!stbi_write_png(filename, width, height, 3, temp, width * 3)) {
        printf("save rgb image as png failed\n");
    }
    delete[] temp;
}
void RgbImage::SaveBmp(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 3];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 3] = data[i].x();
        temp[i * 3 + 1] = data[i].y();
        temp[i * 3 + 2] = data[i].z();
    }
    if(!stbi_write_bmp(filename, width, height, 3, temp)) {
        printf("save rgb image as bmp failed\n");
    }
    delete[] temp;
}
void RgbImage::SaveTga(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 3];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 3] = data[i].x();
        temp[i * 3 + 1] = data[i].y();
        temp[i * 3 + 2] = data[i].z();
    }
    if(!stbi_write_tga(filename, width, height, 3, temp)) {
        printf("save rgb image as tga failed\n");
    }
    delete[] temp;
}
void RgbImage::SavePpm(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 3];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 3] = data[i].x();
        temp[i * 3 + 1] = data[i].y();
        temp[i * 3 + 2] = data[i].z();
    }
    if(!stbi_write_tga(filename, width, height, 3, temp)) {
        printf("save rgb image as ppm failed\n");
    }
    delete[] temp;
}
void RgbImage::SaveJpg(const char *filename, int quality=100) const {
    unsigned char *temp = new unsigned char[width * height * 3];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 3] = data[i].x();
        temp[i * 3 + 1] = data[i].y();
        temp[i * 3 + 2] = data[i].z();
    }
    if(!stbi_write_jpg(filename, width, height, 3, temp, quality)) {
        printf("save rgb image as jpg failed\n");
    }
    delete[] temp;
}
void RgbImage::SaveHdr(const char *filename) const {
    float *temp = new float[width * height * 3];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 3] = data[i].x();
        temp[i * 3 + 1] = data[i].y();
        temp[i * 3 + 2] = data[i].z();
    }
    if(!stbi_write_hdr(filename, width, height, 3, temp)) {
        printf("save rgb image as hdr failed\n");
    }
    delete[] temp;
}
void RgbImage::SaveImage(const char *filename) const {
    std::string str(filename);
    std::string suffix = str.substr(str.find_last_of('.') + 1);
    if(suffix == "png") {
        SavePng(filename);
    } else if(suffix == "bmp") {
        SaveBmp(filename);
    } else if(suffix == "tga") {
        SaveTga(filename);
    } else if(suffix == "ppm") {
        SavePpm(filename);
    } else if(suffix == "jpg") {
        SaveJpg(filename);
    } else if(suffix == "hdr") {
        SaveHdr(filename);
    } else {
        printf("unsupported image format\n");
    }
}

RgbaImage *RgbaImage::LoadImage(const char *filename) {
    unsigned char *temp = stbi_load(filename, &width, &height, NULL, 4);
    if(!temp) {
        printf("load rgba image failed\n");
        return nullptr;
    }else{
        data = new Vector4f[width * height];
        for(int i = 0; i < width * height; ++i) {
            data[i].x() = temp[i * 4];
            data[i].y() = temp[i * 4 + 1];
            data[i].z() = temp[i * 4 + 2];
            data[i].w() = temp[i * 4 + 3];
        }
        return this;
    }
}
void RgbaImage::SavePng(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 4];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 4] = data[i].x();
        temp[i * 4 + 1] = data[i].y();
        temp[i * 4 + 2] = data[i].z();
        temp[i * 4 + 3] = data[i].w();
    }
    if(!stbi_write_png(filename, width, height, 4, temp, width * 4)) {
        printf("save rgba image as png failed\n");
    }
    delete[] temp;
}
void RgbaImage::SaveBmp(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 4];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 4] = data[i].x();
        temp[i * 4 + 1] = data[i].y();
        temp[i * 4 + 2] = data[i].z();
        temp[i * 4 + 3] = data[i].w();
    }
    if(!stbi_write_bmp(filename, width, height, 4, temp)) {
        printf("save rgba image as bmp failed\n");
    }
    delete[] temp;
}
void RgbaImage::SaveTga(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 4];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 4] = data[i].x();
        temp[i * 4 + 1] = data[i].y();
        temp[i * 4 + 2] = data[i].z();
        temp[i * 4 + 3] = data[i].w();
    }
    if(!stbi_write_tga(filename, width, height, 4, temp)) {
        printf("save rgba image as tga failed\n");
    }
    delete[] temp;
}
void RgbaImage::SavePpm(const char *filename) const {
    unsigned char *temp = new unsigned char[width * height * 4];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 4] = data[i].x();
        temp[i * 4 + 1] = data[i].y();
        temp[i * 4 + 2] = data[i].z();
        temp[i * 4 + 3] = data[i].w();
    }
    if(!stbi_write_tga(filename, width, height, 4, temp)) {
        printf("save rgba image as ppm failed\n");
    }
    delete[] temp;
}
void RgbaImage::SaveJpg(const char *filename, int quality=100) const {
    unsigned char *temp = new unsigned char[width * height * 4];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 4] = data[i].x();
        temp[i * 4 + 1] = data[i].y();
        temp[i * 4 + 2] = data[i].z();
        temp[i * 4 + 3] = data[i].w();
    }
    if(!stbi_write_jpg(filename, width, height, 4, temp, quality)) {
        printf("save rgba image as jpg failed\n");
    }
    delete[] temp;
}
void RgbaImage::SaveHdr(const char *filename) const {
    float *temp = new float[width * height * 4];
    for(int i = 0; i < width * height; ++i) {
        temp[i * 4] = data[i].x();
        temp[i * 4 + 1] = data[i].y();
        temp[i * 4 + 2] = data[i].z();
        temp[i * 4 + 3] = data[i].w();
    }
    if(!stbi_write_hdr(filename, width, height, 4, temp)) {
        printf("save rgba image as hdr failed\n");
    }
    delete[] temp;
}
void RgbaImage::SaveImage(const char *filename) const {
    std::string str(filename);
    std::string suffix = str.substr(str.find_last_of('.') + 1);
    if(suffix == "png") {
        SavePng(filename);
    } else if(suffix == "bmp") {
        SaveBmp(filename);
    } else if(suffix == "tga") {
        SaveTga(filename);
    } else if(suffix == "ppm") {
        SavePpm(filename);
    } else if(suffix == "jpg") {
        SaveJpg(filename);
    } else if(suffix == "hdr") {
        SaveHdr(filename);
    } else {
        printf("unsupported image format\n");
    }
}

void DynamicCastTest(Image* image){
    RgbaImage *rgbaimg = dynamic_cast<RgbaImage *>(image);
    if(rgbaimg){
        image = rgbaimg;
    }else{
        RgbImage *rgbimg = dynamic_cast<RgbImage *>(image);
        if(rgbimg){
            image = rgbimg;
        }else{
            GrayAlphaImage *grayalphaimg = dynamic_cast<GrayAlphaImage *>(image);
            if(grayalphaimg){
                image = grayalphaimg;
            }else{
                GrayImage *grayimg = dynamic_cast<GrayImage *>(image);
                if(grayimg){
                    image = grayimg;
                }else{
                    printf("dynamic cast failed\n");
                }
            }
        }
    }
}