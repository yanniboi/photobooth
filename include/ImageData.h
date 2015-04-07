#pragma once
#include <jpeglib.h>

class ImageData{
public:
    long width;
    long height;
    unsigned char *pixbuf;
    std::string fname;
    bool loaded;

    int Load(std::string filename);

    ~ImageData(void)
    {
        delete(pixbuf);
    }
};
