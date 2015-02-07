#pragma once

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "ImageData.h"

class PublicWindow{

    int x_screen;
    Display *x_display;
    Window x_win;
    cairo_surface_t *main_surface;
    cairo_t *dc;

    //Fonts 
    bool hasFonts;
    FT_Library  library;
    FT_Face     fontface_opensans;
    cairo_font_face_t *font_opensans;

    // main image
    ImageData imgData;
    cairo_surface_t *image_surface;
    cairo_surface_t *buffer_surface;

    // countdown
    int countdown_number;

    void paint(void);

public:

    int width; // Width of display
    int height; // Height of display

    bool Init();
    cairo_surface_t *getCairoSurface(){
        return main_surface;
    }

    void Invalidate(void);
    void LoadImage(std::string src);
    void Clear(void);
    void Print();

    Display *getX11Display(){
        return x_display;
    }

    void setCountdownNumber(int n) {
        countdown_number = n;
        paint();
    }
};