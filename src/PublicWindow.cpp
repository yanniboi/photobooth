
#include <iostream>
#include <string>

#include <cairo/cairo-ft.h>

#include "Logging.h"
#include "PublicWindow.h"

using namespace std;

bool PublicWindow::Init() {
 
    if ((x_display = XOpenDisplay(NULL)) == NULL) {
        cerr << "Unable to open X11 display" << endl;
        return false;
    }

    x_screen = DefaultScreen(x_display);

    Screen *scr = DefaultScreenOfDisplay(x_display);    
    cout << "screen: " << WidthOfScreen(scr) << " x " << HeightOfScreen(scr) << endl;
    width = WidthOfScreen(scr);
    height = HeightOfScreen(scr);

    x_win = XCreateSimpleWindow(x_display, DefaultRootWindow(x_display),
      0, 0, width, height, 0, 0, 255);

    XSelectInput(x_display, x_win, 
        ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask |
        ButtonPressMask | ButtonReleaseMask  | StructureNotifyMask 
        );

    XMapWindow(x_display, x_win);

    XFlush(x_display);

    main_surface = cairo_xlib_surface_create(x_display, x_win,
        DefaultVisual(x_display, x_screen), width, height);

    cairo_xlib_surface_set_size(main_surface, width, height);
    dc = cairo_create(main_surface);


    if(!FT_Init_FreeType( &library ))
        hasFonts = true;

    FT_New_Face( library,
        "OpenSans-Light.ttf",
        0,
        &fontface_opensans);
    font_opensans = cairo_ft_font_face_create_for_ft_face(fontface_opensans,
        0);

    return true;
}

void PublicWindow::Invalidate(void) {
    paint();
}

void PublicWindow::LoadImage(string src) {
    Logging::instance().Log(LOGGING_INFO, "Public Window", "Loading image " + src);
    imgData.Load(src);

    Logging::instance().Log(LOGGING_INFO, "Public Window", "\t" + to_string(imgData.width) + "x" + to_string(imgData.height));

    int stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, imgData.width);
    
    image_surface = cairo_image_surface_create_for_data(imgData.pixbuf, CAIRO_FORMAT_RGB24,
                      imgData.width, imgData.height,
                      stride);
    paint();
}

void PublicWindow::Clear(void) {
    cairo_surface_destroy(image_surface);
    image_surface = NULL;
    cairo_set_source_rgb (dc, 0, 0, 0);
    cairo_rectangle(dc, 0,0, width, height);
    cairo_fill_preserve (dc);
    paint();
}

void PublicWindow::paint(void){

    if(image_surface != NULL) {
        cairo_matrix_t matrix;
        cout << "painting" << endl;
        int w = cairo_image_surface_get_width (image_surface);
        int h = cairo_image_surface_get_height (image_surface);
        float scale = h > w ? (float)this->height/h : (float)this->width/w ;
cout << "scale: " << this->height << "/" << h << " = " << scale;
        cairo_get_matrix(dc, &matrix);
        cairo_scale (dc, scale, scale);
cout << "\t" << (this->width - (w * scale)) << " x " << (this->height - (h * scale)) << endl;
        cairo_set_source_surface (dc, image_surface, ((this->width - (w * scale)) / scale) / 2, ((this->height - (h * scale)) / scale) / 2);
        cairo_paint (dc);
        cairo_set_matrix(dc, &matrix);
    }else{
        cairo_set_source_rgb (dc, 0, 0, 0);
        cairo_rectangle(dc, 0,0, width, height);
        cairo_fill_preserve (dc);
    }


if(countdown_number > 0) {
// darken bg
    cairo_set_source_rgba (dc, 0, 0, 0, 0.5);
    cairo_rectangle(dc, 0,0, width, height);
    cairo_fill_preserve (dc);
// draw text
    char cdNum[5];
    snprintf(cdNum, 5, "%d", countdown_number);
    cairo_set_source_rgb(dc, 0, 255, 0);
    cairo_set_font_face (dc, font_opensans);
    cairo_set_font_size (dc, 400.0);
    cairo_text_extents_t extents;
    cairo_text_extents (dc, cdNum, &extents);
    float x = ((float)width / 2)-(extents.width/2 + extents.x_bearing);
    float y = ((float)height / 2)-(extents.height/2 + extents.y_bearing);
    cout << "font " << x << "x" << y << endl;
    cairo_move_to (dc, x, y);
    cairo_show_text (dc, cdNum);
}

        /*cairo_set_source_rgb(dc, 0, 0, 0);
        cairo_set_line_width(dc, 4);
        cairo_rectangle(dc, 0,0, 800, 600);
        cairo_stroke_preserve(dc);*/

        XEvent ev;
        while(XPending(x_display))
            XNextEvent(x_display, &ev);
}