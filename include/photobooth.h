#pragma once

#include <vector>
#include "camera.h"
#include "cameraservice.h"

class Context {

private:
    Context(){};
    ~Context(){};

public:
    int uid;
    int gid;
    std::vector<PBCamera *> cams;
    PBCameraService *cameraService;


    static Context &Current(){
        static Context c;
        return c;
    }
};