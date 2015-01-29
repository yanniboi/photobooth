#include <chrono>         // std::chrono::seconds
#include <string>         // std::string
#include <thread>         // std::this_thread::sleep_for

#include "cameraservice.h"
#include "photobooth.h"
#include "Logging.h"


static GPPortInfoList       *portinfolist;
static CameraAbilitiesList  *abilities;


void PBCameraService::_processed(std::string filename) {
    onProcessed(filename);
}


int PBCameraService::open_camera (Camera ** camera, const char *model, const char *port, GPContext *context) {
    int     ret, m, p;
    CameraAbilities a;
    GPPortInfo  pi;

    ret = gp_camera_new (camera);
    if (ret < GP_OK) return ret;

    if (!abilities) {
        /* Load all the camera drivers we have... */
        ret = gp_abilities_list_new (&abilities);
        if (ret < GP_OK) return ret;
        ret = gp_abilities_list_load (abilities, context);
        if (ret < GP_OK) return ret;
    }

    /* First lookup the model / driver */
        m = gp_abilities_list_lookup_model (abilities, model);
    if (m < GP_OK) return ret;
        ret = gp_abilities_list_get_abilities (abilities, m, &a);
    if (ret < GP_OK) return ret;
        ret = gp_camera_set_abilities (*camera, a);
    if (ret < GP_OK) return ret;

    if (!portinfolist) {
        /* Load all the port drivers we have... */
        ret = gp_port_info_list_new (&portinfolist);
        if (ret < GP_OK) return ret;
        ret = gp_port_info_list_load (portinfolist);
        if (ret < 0) return ret;
        ret = gp_port_info_list_count (portinfolist);
        if (ret < 0) return ret;
    }

    /* Then associate the camera with the specified port */
        p = gp_port_info_list_lookup_path (portinfolist, port);
        switch (p) {
        case GP_ERROR_UNKNOWN_PORT:
                fprintf (stderr, "The port you specified "
                        "('%s') can not be found. Please "
                        "specify one of the ports found by "
                        "'gphoto2 --list-ports' and make "
                        "sure the spelling is correct "
                        "(i.e. with prefix 'serial:' or 'usb:').",
                                port);
                break;
        default:
                break;
        }
        if (p < GP_OK) return p;

        ret = gp_port_info_list_get_info (portinfolist, p, &pi);
        if (ret < GP_OK) return ret;
        ret = gp_camera_set_port_info (*camera, pi);
        if (ret < GP_OK) return ret;
    return GP_OK;
}


std::string PBCameraService::trigger(void) {
    int i;
    std::string fname = "";
    for (i=0; i < Context::Current().cams.size(); i++) {
        fname = Context::Current().cams[i]->Capture();
    }
    return fname;
}

void PBCameraService::init() {

    GPContext *cam_context;
    cam_context = gp_context_new(); // Create Context


    CameraList *list;
    gp_list_new (&list);
    Logging::instance().Log(LOGGING_DEBUG, "Cam Service", "Waiting for camera to be detected.");
    while (gp_list_count(list) == 0) {
        std::this_thread::sleep_for (std::chrono::seconds(1));
        gp_camera_autodetect (list, cam_context);
    }
    Logging::instance().Log(LOGGING_INFO, "Cam Service", "Found at least one camera:");
    int i;
    for (i=0; i < gp_list_count(list); i++) {
        const char *name, *value;
        gp_list_get_name (list, i, &name);
        gp_list_get_value (list, i, &value);
        Logging::instance().Log(LOGGING_INFO, "Cam Service", "\t" + std::string(name));

        Camera *cam;
        gp_camera_new(&cam);
        
        open_camera(&cam, name, value, cam_context);

        PBCamera *camera = new PBCamera(cam, cam_context);
        Context::Current().cams.push_back(camera);
        camera->onProcessed.bind(this, &PBCameraService::_processed);
    }

    Logging::instance().Log(LOGGING_DEBUG, "Cam Service", "Finished cam service init.");

}