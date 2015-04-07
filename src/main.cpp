/* Copyright  - Lee Hazlehurst 2014 */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <pwd.h>
#include <grp.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <X11/Xutil.h>

#include "camera.h"
#include "cameraservice.h"
#include "Logging.h"
#include "watchable_fd.h"
#include "webserver.h"
#include "PublicWindow.h"



using namespace std;

PublicWindow win;
Display *dsp;
vector<watchable_fd> fds;

class Photobooth {



};


void onProcessed(string filename) {
	win.LoadImage("/home/lee/.photobooth/" + filename);
}


void onPrint() {
    win.Print();
}

void onClear() {
    win.Clear();
}


void countdown() {
    win.Clear();
    win.setCountdownNumber(5);
    for(int i = 5; i >= 0; i--){
        usleep(1000000);
        win.setCountdownNumber(i);
    }
    win.Clear();
    
    Context::Current().cameraService->trigger();
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
    //setuid(ctx.uid);
    umask(S_IWOTH);


    // Get home path
    char path[3000];
    const char *home = ("/home/" + username).c_str();//getenv ("HOME");
cout << "home is " << home << endl;
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
    ctx.cameraService = &camservice;
    camservice.init();   // Comment me out for non camera debug.
    

//FD for stdin
    

// Redundant (for keyboard debug)
    Logging::instance().Log(LOGGING_VERBOSE, "Main", "Hooking up stdin fd.");
    watchable_fd wfd;
    wfd.fd = 0;
    wfd.onReadable = &capture_thread;
    fds.push_back(wfd);
    

//FD for fcgi
    webserver web(ctx, run_dir + "/fcgi.sock");
    web.onTrigger.bind(&countdown);
    web.onClear.bind(&onClear);
    web.onPrint.bind(&onPrint);
    fds.push_back(web.fd());

    win.Init();
    win.LoadImage("capt0004.jpg");  // If image exists use this rather than looking for camera


// Continuous update loop to keep the screen updating.
    int Xfd = ConnectionNumber(win.getX11Display());
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
                XNextEvent(win.getX11Display(), &ev);
                switch (ev.type) {
                    case MapNotify:
                    case Expose:
                        break;
                    case ConfigureNotify:
                        win.Invalidate();
                        break;
                }
            }  
        }
        while(XPending(win.getX11Display()))
            XNextEvent(win.getX11Display(), &ev);
    }

    Logging::instance().Log(LOGGING_INFO, "Main", "Application Exit.");
}
