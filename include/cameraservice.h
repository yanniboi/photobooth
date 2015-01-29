#pragma once

#include "camera.h"

class PBCameraService {
private:
    void _processed(std::string filename);
public:
    void init();
    int open_camera (Camera ** camera, const char *model, const char *port, GPContext *context);
    std::string trigger(void);
    onProcessedDelegate onProcessed;
};