#pragma once
#include <iostream>
#include <queue>

#include "FastDelegate.h"
#include <gphoto2/gphoto2.h>

using namespace fastdelegate;

typedef FastDelegate1<std::string> onProcessedDelegate;

struct camera_queue_entry {
	CameraFilePath	path;
	int offset;
};

class PBCamera
{

private:
	int _id;
	std::queue<camera_queue_entry> _queue;
	Camera *_camera;
	GPContext *_camera_context;
	CameraWidget *_config;

public:

	PBCamera(Camera *cam, GPContext *ctx);
        ~PBCamera();

	std::string Capture();
	std::string Process();

	void SetConfigValue(std::string path, void *value);
	CameraWidget *GetConfigWidget(std::string path);
        void Wait();

	//delegates
	onProcessedDelegate onProcessed;

};
