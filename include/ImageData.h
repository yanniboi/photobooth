#pragma once
#include <jpeglib.h>

class ImageData{
public:
    long width;
    long height;
    unsigned char *pixbuf;

    int Load(std::string filename);

    ~ImageData(void)
    {
        delete(pixbuf);
    }
};