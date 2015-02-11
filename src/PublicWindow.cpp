	
#include <iostream>
#include <string>

#include <cairo/cairo-ft.h>
#include <cairo-ps.h>

#include <cups/cups.h>

#include "Logging.h"
#include "PublicWindow.h"

using namespace std;

bool PublicWindow::Init() {
 
    if ((x_display = XOpenDisplay(NULL)) == NULL) {
        cerr << "Unable to open X11 display" << endl;
        exit(1);
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


    buffer_surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
                                                 width,
                                                 height);

    return true;
}

void PublicWindow::Invalidate(void) {
    paint();
}

void PublicWindow::LoadImage(string src) {
    Logging::instance().Log(LOGGING_INFO, "Public Window", "Loading image " + src);
    if(imgData.Load(src) > 0){
        Logging::instance().Log(LOGGING_INFO, "Public Window", "\t" + to_string(imgData.width) + "x" + to_string(imgData.height));

        int stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, imgData.width);
        
        image_surface = cairo_image_surface_create_for_data(imgData.pixbuf, CAIRO_FORMAT_RGB24,
                          imgData.width, imgData.height,
                          stride);
    }
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

    cairo_t *temp_dc = cairo_create(buffer_surface);
    if(image_surface != NULL) {
        cairo_matrix_t matrix;
        cout << "painting" << endl;
        int w = cairo_image_surface_get_width (image_surface);
        int h = cairo_image_surface_get_height (image_surface);
        float scale = h > w ? (float)this->height/h : (float)this->width/w ;
cout << "scale: " << this->height << "/" << h << " = " << scale;
        cairo_get_matrix(temp_dc, &matrix);
        cairo_scale (temp_dc, scale, scale);
cout << "\t" << (this->width - (w * scale)) << " x " << (this->height - (h * scale)) << endl;
        cairo_set_source_surface (temp_dc, image_surface, ((this->width - (w * scale)) / scale) / 2, ((this->height - (h * scale)) / scale) / 2);
        cairo_paint (temp_dc);
        cairo_set_matrix(temp_dc, &matrix);
    }else{
        cairo_set_source_rgb (temp_dc, 0, 0, 0);
        cairo_rectangle(temp_dc, 0,0, width, height);
        cairo_fill_preserve (temp_dc);
    }


if(countdown_number > 0) {
// darken bg
    cairo_set_source_rgba (temp_dc, 0, 0, 0, 0.8);
    cairo_rectangle(temp_dc, 0,0, width, height);
    cairo_fill_preserve (temp_dc);
// draw text
    char cdNum[5];
    snprintf(cdNum, 5, "%d", countdown_number);
    cairo_set_source_rgb(temp_dc, 255, 255, 255);
    cairo_set_font_face (temp_dc, font_opensans);
    cairo_set_font_size (temp_dc, 400.0);
    cairo_text_extents_t extents;
    cairo_text_extents (temp_dc, cdNum, &extents);
    float x = ((float)width / 2)-(extents.width/2 + extents.x_bearing);
    float y = ((float)height / 2)-(extents.height/2 + extents.y_bearing);
    cout << "font " << x << "x" << y << endl;
    cairo_move_to (temp_dc, x, y);
    cairo_show_text (temp_dc, cdNum);
}

cairo_set_source_surface (dc, buffer_surface, 0,0);
cairo_paint(dc);
cairo_destroy(temp_dc);

        /*cairo_set_source_rgb(v, 0, 0, 0);
        cairo_set_line_width(temp_dc, 4);
        cairo_rectangle(temp_dc, 0,0, 800, 600);
        cairo_stroke_preserve(temp_dc);*/

        XEvent ev;
        while(XPending(x_display))
            XNextEvent(x_display, &ev);
}

#define PAGE_WIDTH 280
#define PAGE_HEIGHT 420

void PublicWindow::Print() {
    cout << "Todo: Print" << endl;
    cairo_surface_t *ps;
    cairo_t *ps_dc;
    cairo_matrix_t matrix;

    ps = cairo_ps_surface_create ("output.ps", PAGE_WIDTH, PAGE_HEIGHT);
    ps_dc = cairo_create(ps);
    cairo_ps_surface_dsc_comment (ps, "%%PageOrientation: Landscape");

    cairo_translate (ps_dc, 0, PAGE_HEIGHT);
    cairo_matrix_init (&matrix, 0, -1, 1, 0, 0,  0);
    cairo_transform (ps_dc, &matrix);

    float scale = imgData.width> imgData.height ? (float)PAGE_WIDTH / (float)imgData.height : (float)PAGE_HEIGHT / (float)imgData.width;
    cout << scale << endl;
    cairo_scale (ps_dc, scale, scale);
    cairo_set_source_surface (ps_dc, image_surface, 0, 0);
    cairo_paint (ps_dc);
    cairo_surface_show_page (ps);

    cairo_destroy (ps_dc);
    cairo_surface_finish (ps);
    cairo_surface_destroy (ps);


    cups_dest_t *dests, *dest;
    int num_dests = cupsGetDests(&dests);
    if(num_dests > 0){
        cout << "Printing to : " << dests->name << endl;


        const char *ppd_filename;
        ppd_filename = cupsGetPPD(dests->name);

        int           num_options;
        cups_option_t *options;
        num_options = 0;
        options     = NULL;
        num_options = cupsAddOption("media", "Custom.100x150mm", num_options, &options);
        cupsPrintFile(dests->name, "output.ps", "cairo PS", num_options, options);
        cupsFreeOptions(num_options, options);
        cout << "Sent to printer" << endl;
    }else{
        cout << "No printers ready" << endl;
    }
}
