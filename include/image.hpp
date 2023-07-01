#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <string>

class Image {
public:
    virtual ~Image() {}
    virtual int Width()=0;
    virtual int Height()=0;
    virtual Image *LoadImage(const char *filename) = 0;
    virtual void SavePng(const char *filename) const = 0;
    virtual void SaveBmp(const char *filename) const = 0;
    virtual void SaveTga(const char *filename) const = 0;
    virtual void SavePpm(const char *filename) const = 0;
    virtual void SaveJpg(const char *filename, int quality) const = 0;
    virtual void SaveHdr(const char *filename) const = 0;
    virtual void SaveImage(const char *filename) const = 0;
    
protected:
    int width;
    int height;
};

class GrayImage : public Image {
public:
    ~GrayImage() {
        if(data) delete[] data;
    }
    GrayImage(int w, int h) {
        width = w;
        height = h;
        data = new float[width * height];
    }
    GrayImage(const char *filename) {
        LoadImage(filename);
    }
    GrayImage(const std::string& filename) {
        LoadImage(filename.c_str());
    }
    int Width() {return width;}
    int Height() {return height;}
    float GetPixel(int x, int y) const {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        return data[y * width + x];
    }
    GrayImage(const Image &image) = delete;
    GrayImage(Image *image) {
        //convert to gray image
        GrayImage *img = dynamic_cast<GrayImage *>(image);
        if(!img) printf("convert to gray image failed\n");
        assert(img!=nullptr);
        width = img->Width();
        height = img->Height();
        data = new float[width * height];
        for(int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x){
                data[y * width + x] = img->GetPixel(x,y);
            }
        }
    }
    void SetPixel(int x, int y, unsigned char color) {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        data[y * width + x] = color;
    }
    void SetAllPixels(unsigned char color) {
        for (int i = 0; i < width * height; ++i) {
            data[i] = color;
        }
    }

    GrayImage *LoadImage(const char *filename) ;
    void SavePng(const char *filename) const ;
    void SaveBmp(const char *filename) const ;
    void SaveTga(const char *filename) const ;
    void SavePpm(const char *filename) const ;
    void SaveJpg(const char *filename, int quality ) const ;
    void SaveHdr(const char *filename) const ;
    void SaveImage(const char *filename) const ;
    
private:
    float *data;
};

class GrayAlphaImage : public Image {
public:
    ~GrayAlphaImage() {
        if(data) delete[] data;
    }
    GrayAlphaImage(int w, int h) {
        width = w;
        height = h;
        data = new Vector2f[width * height];
    }
    GrayAlphaImage(const char *filename) {
        LoadImage(filename);
    }
    GrayAlphaImage(const std::string& filename) {
        LoadImage(filename.c_str());
    }
    int Width() {return width;}
    int Height() {return height;}
    Vector2f GetPixel(int x, int y) const {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        return data[y * width + x];
    }
    GrayAlphaImage(const Image &image) = delete;
    GrayAlphaImage(Image *image) {
        //convert to grayalpha image
        GrayAlphaImage *img = dynamic_cast<GrayAlphaImage *>(image);
        if(!img) printf("convert to grayalpha image failed\n");
        assert(img!=nullptr);
        width = img->Width();
        height = img->Height();
        data = new Vector2f[width * height];
        for(int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x){
                data[y * width + x] = img->GetPixel(x,y);
            }
        }
    }
    
    void SetPixel(int x, int y, Vector2f color) {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        data[y * width + x] = color;
    }
    void SetAllPixels(Vector2f color) {
        for (int i = 0; i < width * height; ++i) {
            data[i] = color;
        }
    }
    GrayAlphaImage *LoadImage(const char *filename) ;
    void SavePng(const char *filename) const ;
    void SaveBmp(const char *filename) const ;
    void SaveTga(const char *filename) const ;
    void SavePpm(const char *filename) const ;
    void SaveJpg(const char *filename, int quality ) const ;
    void SaveHdr(const char *filename) const ;
    void SaveImage(const char *filename) const ;
    
private:
    Vector2f *data;
};

class RgbImage : public Image {
public:
    ~RgbImage() {
        if(data) delete[] data;
    }
    RgbImage(int w, int h) {
        width = w;
        height = h;
        data = new Vector3f[width * height];
    }
    RgbImage(const char *filename) {
        LoadImage(filename);
    }
    RgbImage(const std::string& filename) {
        LoadImage(filename.c_str());
    }
    int Width() {return width;}
    int Height() {return height;}
    Vector3f GetPixel(int x, int y) const {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        return data[y * width + x];
    }
    RgbImage(const Image &image) = delete;
    RgbImage(Image *image) {
        //convert to rgb image
        RgbImage *img = dynamic_cast<RgbImage *>(image);
        if(!img) printf("convert to rgb image failed\n");
        assert(img!=nullptr);
        width = img->Width();
        height = img->Height();
        data = new Vector3f[width * height];
        for(int y = 0; y < height; ++y) {
            for(int x = 0; x < width; ++x){
                data[y * width + x] = img->GetPixel(x,y);
            }
        }
    }
    
    void SetPixel(int x, int y, Vector3f color) {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        data[y * width + x] = color;
    }
    void SetAllPixels(Vector3f color) {
        for (int i = 0; i < width * height; ++i) {
            data[i] = color;
        }
    }
    RgbImage *LoadImage(const char *filename) ;
    void SavePng(const char *filename) const ;
    void SaveBmp(const char *filename) const ;
    void SaveTga(const char *filename) const ;
    void SavePpm(const char *filename) const ;
    void SaveJpg(const char *filename, int quality ) const ;
    void SaveHdr(const char *filename) const ;
    void SaveImage(const char *filename) const ;

private:
    Vector3f *data;
};

class RgbaImage : public Image {
public:
    ~RgbaImage() {
        if(data) delete[] data;
    }
    RgbaImage(int w, int h) {
        width = w;
        height = h;
        data = new Vector4f[width * height];
    }
    RgbaImage(const char *filename) {
        LoadImage(filename);
    }
    RgbaImage(const std::string& filename) {
        LoadImage(filename.c_str());
    }
    int Width() {return width;}
    int Height() {return height;}
    Vector4f GetPixel(int x, int y) const {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        return data[y * width + x];
    }
    RgbaImage(const Image &image) = delete;
    RgbaImage(Image *image) {
        //convert to rgba image
        RgbaImage *img = dynamic_cast<RgbaImage *>(image);
        if(!img){
            printf("convert to rgba image failed\n");
            RgbImage *img_t = dynamic_cast<RgbImage *>(image);
            if(!img_t) printf("convert to rgb image failed\n");
            else{
                assert(img_t!=nullptr);
                printf("convert to rgba image from rgb image\n");
                width = img_t->Width();
                height = img_t->Height();
                data = new Vector4f[width * height];
                for(int y = 0; y < height; ++y) {
                    for(int x = 0; x < width; ++x){
                        data[y * width + x].x() = img_t->GetPixel(x,y).x();
                        data[y * width + x].y() = img_t->GetPixel(x,y).y();
                        data[y * width + x].z() = img_t->GetPixel(x,y).z();
                        data[y * width + x].w() = 1;
                    }
                }
            }
        }else{
            assert(img!=nullptr);
            width = img->Width();
            height = img->Height();
            data = new Vector4f[width * height];
            for(int y = 0; y < height; ++y) {
                for(int x = 0; x < width; ++x){
                    data[y * width + x] = img->GetPixel(x,y);
                }
            }
        }
    }
    
    void SetPixel(int x, int y, Vector4f color) {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        data[y * width + x] = color;
    }
    void SetAllPixels(Vector4f color) {
        for (int i = 0; i < width * height; ++i) {
            data[i] = color;
        }
    }
    RgbaImage *LoadImage(const char *filename) ;
    void SavePng(const char *filename) const ;
    void SaveBmp(const char *filename) const ;
    void SaveTga(const char *filename) const ;
    void SavePpm(const char *filename) const ;
    void SaveJpg(const char *filename, int quality ) const ;
    void SaveHdr(const char *filename) const ;
    void SaveImage(const char *filename) const ;
private:
    Vector4f *data;
};

void DynamicCastTest(Image*& image);


#endif // IMAGE_H
