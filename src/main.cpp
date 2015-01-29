/* Copyright  - Lee Hazlehurst 2014 */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <pwd.h>
#include <grp.h>

#include <gtkmm.h>
#include <gtkmm/application.h>

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "camera.h"
#include "cameraservice.h"
#include "Logging.h"
#include "watchable_fd.h"
#include "webserver.h"
#include "ImageData.h"



using namespace std;


Display *dsp;
cairo_t *cairo;
cairo_surface_t *cs;
vector<watchable_fd> fds;
cairo_surface_t *image;
ImageData *data;



class Photobooth {



};

void paint()
{

    if(image != NULL) {
        cout << "paint" << endl;
        float win_width = 800;
        float win_height = 600;
        int w = cairo_image_surface_get_width (image);
        int h = cairo_image_surface_get_height (image);
        float scale = h > w ? win_width/w : win_height/h;
        float inverseScale = h > w ? w/win_width : h/win_height;

        cairo_scale (cairo, scale, scale);
        cairo_set_source_surface (cairo, image, 0, 0);
        cairo_paint (cairo);
        cairo_scale (cairo, inverseScale, inverseScale);
        cairo_set_source_rgb(cairo, 0, 0, 0);
        cairo_set_line_width(cairo, 4);
        cairo_rectangle(cairo, 0,0, win_width, win_height);
        cairo_stroke_preserve(cairo);
    }
}


void drawImage(cairo_t *cr, std::string src){
    
    if(data != NULL)
        delete(data);

//Load the image from jpg;
    data = new ImageData();
    int otherstride = data->Load(src);

    cout << data->width << "x" << data->height << endl;
    

    int stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, data->width);
    cout << "stride: " << stride << " - " << otherstride << endl;

    cout << "drawing " << src << endl;

    Logging::instance().Log(LOGGING_INFO, "Main", "Drawing image " + src);
    image = cairo_image_surface_create_for_data(data->pixbuf, CAIRO_FORMAT_RGB24,
                      data->width, data->height,
                      stride);

    //cairo_scale (cr, 0.5, 0.5);
    paint();
}


void onProcessed(string filename) {
	drawImage(cairo, "/home/lee/.photobooth/" + filename);
}


void capture_thread(watchable_fd watchable) {
    
    // throw away all input
    char out[254];
    while (read(watchable.fd, &out, 254) == 254) {}

	Context::Current().cameraService->trigger();

}

void CheckAndCreateDir(const char* path) {
    //Check working dir exists
    struct stat buffer;   
    if (stat (path, &buffer) != 0){
        //Create working dir
        mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
}

uid_t name_to_uid(char const *name)
{
  if (!name)
    return -1;
  long const buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (buflen == -1)
    return -1;
  // requires c99
  char buf[buflen];
  struct passwd pwbuf, *pwbufp;
  if (0 != getpwnam_r(name, &pwbuf, buf, buflen, &pwbufp)
      || !pwbufp)
    return -1;
  return pwbufp->pw_uid;
}

gid_t name_to_gid(char const *name)
{
  if (!name)
    return -1;
  long const buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (buflen == -1)
    return -1;
  // requires c99
  char buf[buflen];
  struct group grbuf, *grbufp;
  if (0 != getgrnam_r(name, &grbuf, buf, buflen, &grbufp)
      || !grbufp)
    return -1;
  return grbufp->gr_gid;
}





cairo_surface_t *cairo_create_x11_surface(int x, int y)
{
   
   int screen;
   cairo_surface_t *sfc;
 
   if ((dsp = XOpenDisplay(NULL)) == NULL)
      exit(1);
   screen = DefaultScreen(dsp);
   Window da = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp),
      0, 0, x, y, 0, 0, 255);



   XSelectInput(dsp, da, 
        ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask |
        ButtonPressMask | ButtonReleaseMask  | StructureNotifyMask 
        );
   XMapWindow(dsp, da);

   XFlush(dsp);

   sfc = cairo_xlib_surface_create(dsp, da,
      DefaultVisual(dsp, screen), x, y);
   cairo_xlib_surface_set_size(sfc, x, y);

   
 
   return sfc;
}







int main(int argc, char* argv[]) {
    
    Context &ctx = Context::Current();
    string username = "lee";
    string group = "www-data";
    ctx.uid = name_to_uid(username.c_str());
    ctx.gid = name_to_gid(group.c_str());

    if (getuid() != 0) {
        cerr << "This application must be run as root!" <<endl;
        return -1;
    }

    if (ctx.uid < 0) {
        cerr << "Cannot find uid of user " << username << endl;
        return -1;
    }

    if (ctx.uid == 0) {
        cerr << "Runnable user should not be root, change the config file. " << endl;
        return -1;
    }

    // We are currenly root, lets create the run folder
    string run_dir = "/var/run/photobooth";
    CheckAndCreateDir(run_dir.c_str());
    chown(run_dir.c_str(), ctx.uid, ctx.gid);

    //now lets change our running user
    setgid(ctx.gid);
    setuid(ctx.uid);
    umask(S_IWOTH);


    // Get home path
    char path[3000];
    char *home = getenv ("HOME");
    string working_dir = string(home)  + "/.photobooth/";
    

    CheckAndCreateDir(working_dir.c_str());
    

    if (chdir(working_dir.c_str()) != 0) {
        cout << "Cannot open working directory" << endl;
        return 1;
    }



    Logging::instance().Init(string(working_dir) + "photobooth.log");
    Logging::instance().Log(LOGGING_INFO, "Main", "Starting Photobooth control.");

// Init the camera service
    PBCameraService camservice;
    camservice.onProcessed.bind(&onProcessed);
    camservice.init();
    

//FD for stdin
    
    Logging::instance().Log(LOGGING_VERBOSE, "Main", "Hooking up stdin fd.");
    watchable_fd wfd;
    wfd.fd = 0;
    wfd.onReadable = &capture_thread;
    fds.push_back(wfd);
    

//FD for fcgi
    webserver web(ctx, run_dir + "/fcgi.sock");

    cs = cairo_create_x11_surface(800,600);
    cairo = cairo_create(cs);

    int Xfd = ConnectionNumber(dsp);
XEvent ev;
    fd_set watched_fds;
    while(1) {

        int max = 0;
        FD_ZERO(&watched_fds);
        for (watchable_fd x : fds) {
            FD_SET(x.fd, &watched_fds);
            if(x.fd > max) max = x.fd;
        }
        FD_SET(Xfd, &watched_fds);
        if (Xfd > max)
            max = Xfd;

        if (select(max + 1, &watched_fds, NULL, NULL, NULL)) {
            for (watchable_fd x : fds) {
                if (FD_ISSET(x.fd, &watched_fds)) {
                    x.onReadable(x);
                }
            }
            if(FD_ISSET(Xfd, &watched_fds)) {
                XNextEvent(dsp, &ev);
                switch (ev.type) {
                    case MapNotify:
                    case Expose:
                        break;
                    case ConfigureNotify:
                        paint();
                        break;
                }
            } 
        }
        while(XPending(dsp))
            XNextEvent(dsp, &ev);
    }

    Logging::instance().Log(LOGGING_INFO, "Main", "Application Exit.");
}
