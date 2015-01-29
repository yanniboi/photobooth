#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <queue>
#include <string>

#include "camera.h"
#include "Logging.h"
#include <gphoto2/gphoto2.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::queue;


static int cameracount;

static void saveImage(const char *fname, CameraFilePath *path, Camera *cam, GPContext *ctx)
{
    Logging::instance().Log(LOGGING_VERBOSE, "PBCamera-Saving", "Saving image from camera " + string(path->name));


    char *cwd = get_current_dir_name();
    

    // Do the saving
    CameraFile *file;
    
    int fd = open(string(fname).c_str(),  O_WRONLY | O_CREAT, 0644);
    gp_file_new_from_fd(&file, fd);
    gp_camera_file_get(cam, path->folder, path->name,
        GP_FILE_TYPE_NORMAL, file, ctx);
    gp_camera_file_delete(cam, path->folder, path->name, ctx);
    gp_file_free(file);
    close(fd);
    Logging::instance().Log(LOGGING_VERBOSE, "PBCamera-Processing", "Saved image as: " + string(cwd) + "/" + string(fname));
}





PBCamera::PBCamera(Camera *cam, GPContext *ctx) {

    _id = ++cameracount;

    if (cam == NULL) {
        Logging::instance().Log(LOGGING_WARNING, "PBCamera", "Camera is not open");
        return;
    }

    this->_camera = cam;
    this->_camera_context = ctx;

    if (gp_camera_get_config (_camera, &_config, _camera_context) < GP_OK) {
        _config = NULL;
    }
}


std::string PBCamera::Capture() {
    if (this->_camera == NULL || this->_camera_context == NULL) {
        cerr << "Cannot trigger an uninitialised camera." << endl;
    }

    //int retval = gp_camera_trigger_capture (this->_camera, this->_camera_context);

    CameraFilePath path;
    int retval = gp_camera_trigger_capture   (this->_camera,
        this->_camera_context
    );
    std::string fname = "";

    if (retval != GP_OK) {
        cerr << "Error triggering camera." << endl;
    }else{
        //Logging::instance().Log(LOGGING_VERBOSE, "PBCamera", "Captured: " + string(path.name));
        //saveImage(&path, _camera, _camera_context);
        fname = Process();
    }
    //PBCamera::Process();

    return fname;
}



std::string PBCamera::Process() {
    const int waittime = 2000;
    CameraEventType evtype;
    CameraFilePath *path;
    void *data;
    int retval;
    bool keepgoing = true;
    Logging::instance().Log(LOGGING_VERBOSE, "PBCamera-Processing", "Waiting for events");

    std::string jpgfname = "";

    while (keepgoing) {
        retval = gp_camera_wait_for_event(this->_camera,
            waittime, &evtype,
            &data, this->_camera_context);

        if (retval != GP_OK) {
            fprintf(stderr, "Error in getting camera event. %d\n", retval);
            return NULL;
        }
        switch (evtype) {
            case GP_EVENT_CAPTURE_COMPLETE:
                Logging::instance().Log(LOGGING_VERBOSE, "PBCamera-Processing", "Capture Complete");
                keepgoing = false;
                break;
            case GP_EVENT_UNKNOWN:
                //fprintf(stderr, "Unknown event\n");
                break;
            case GP_EVENT_TIMEOUT:
                keepgoing = false;
                Logging::instance().Log(LOGGING_VERBOSE, "PBCamera-Processing", "Timed out");
                break;
            case GP_EVENT_FOLDER_ADDED:
                fprintf(stderr, "wait for event FOLDER_ADDED\n");
                break;
            case GP_EVENT_FILE_ADDED:
                path = reinterpret_cast<CameraFilePath *>(data);                
                //Get the time for filename
                const long sysTime = time(0);

                //Get the extension for filename
                std::string extension;
                std::string filename(path->name);
                std::string::size_type idx;
                idx = filename.rfind('.');
                if (idx != std::string::npos) {
                     extension = filename.substr(idx+1);
                } else {
                    extension = "jpg";
                }

                if(extension == "jpg")
                    jpgfname = (std::to_string(_id) + "-" + std::to_string(sysTime) + "." + extension);

                saveImage(path->name, path, _camera, _camera_context);
                if(extension == "jpg")
                    onProcessed(path->name);
                break;
        }
    }
    return jpgfname;
}



static void split(queue<string> & theStringQueue,  /* Altered/returned value */
       const  string  & theString,
       const  string  & theDelimiter) {
    size_t  start = 0, end = 0;

    while (end != string::npos) {
        end = theString.find(theDelimiter, start);

        // If at end, use length=maxLength.  Else use length=end-start.
        string s = theString.substr( start, (end == string::npos) ? string::npos : end - start);
        if (s.length() > 0)
            theStringQueue.push(s);


        // If at end, use start=maxSize.  Else use start=end+delimiter.
        start = ((end > (string::npos - theDelimiter.size()) ) ?  string::npos  :  end + theDelimiter.size());
    }
}


static bool _loop_find_widget(queue<string> *path, CameraWidget *parent, CameraWidget **out) {
    CameraWidget *child;
    int retval = gp_widget_get_child_by_name(parent, path->front().c_str() , &child);
    if (retval < GP_OK) {
        return false;
    }
    path->pop();
    if (path->size() > 0) {
        return _loop_find_widget(path, child, out);
    } else {
        *out = child;
        return true;
    }
}

CameraWidget *PBCamera::GetConfigWidget(string path) {
    if (_config == NULL)
        return NULL;

    queue<string> splitpath;
    split(splitpath, path, "/");

    CameraWidget *w;
    if (_loop_find_widget(&splitpath, _config, &w)) {
        return w;
    }
    return NULL;
}


void PBCamera::SetConfigValue(string path, void *value) {
    CameraWidget * widget = GetConfigWidget(path);
    if (widget != NULL) {
        gp_widget_set_value(widget, value);
    }
}
