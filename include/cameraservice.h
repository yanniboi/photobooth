#pragma once

#include "camera.h"

class PBCameraService {
private:
//    GPContext *cam_context;
    void _processed(std::string filename);
    void findCamera();
public:
    void init();
    int open_camera (Camera ** camera, const char *model, const char *port, GPContext *context);
    std::string trigger(void);
    onProcessedDelegate onProcessed;
};
