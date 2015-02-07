/* Copyright Lee Hazlehurst 2014 */

#include "fastcgi.h"

#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <functional>

#include "Logging.h"
#include "photobooth.h"
#include "webserver.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;


static void onRead(watchable_fd watchable) {
    cout << "Reading webserver stuffs" << endl;
    webserver wb = *(reinterpret_cast<webserver *>(watchable.obj));
    wb.ExecuteRequest();
}

webserver::webserver(Context &ctx, string socket) {
    int retval;
    retval = FCGX_Init();
    if (retval != 0) {
        //cerr << "Cannot start fastcgi server." << endl;
        Logging::instance().Log(LOGGING_ERROR, "Webserver", "Cannot init fastcgi.");
        return;
    }

    Logging::instance().Log(LOGGING_INFO, "Webserver", "Create webserver socket " + socket);
    try {
        int sock = FCGX_OpenSocket(socket.c_str(), 1024);
        _fd.fd = sock;
        _fd.onReadable = &onRead;
        _fd.obj = this;
        FCGX_InitRequest(&_request, sock, 0);
    } catch(int errNo) {
        Logging::instance().Log(LOGGING_ERROR, "Webserver", "Cannot connect to fastcgi socket");
    }


    mRouteTable["/photobooth/(.*)"] = std::bind(&webserver::request_basic, this, std::placeholders::_1);

}


void webserver::_thread(void) {
    while(true)
    {
        ExecuteRequest();
    }
}

watchable_fd webserver::fd() {
    return _fd;
}


void webserver::request_basic(webrequest &req) {
    cout <<"HAHA here we are" << endl;
}










void webserver::ExecuteRequest() {
    
    if (FCGX_Accept_r(&_request) == 0) {
        webrequest req;
        req.url = string(FCGX_GetParam("REQUEST_URI", _request.envp));
        req.method = string(FCGX_GetParam("REQUEST_METHOD", _request.envp));

        Logging::instance().Log(LOGGING_INFO, "Webserver", req.method + " request made to " + req.url);
        
        std::string fname  = "";

        if (req.url == "/photobooth/click") {
            fname = Context::Current().cameraService->trigger();
            FCGX_FPrintF(_request.out, ("\n\n" + fname).c_str());
        }else if (req.url == "/photobooth/go") {
            onTrigger();
            FCGX_FPrintF(_request.out, ("\n\n" + fname).c_str());
        }else if (req.url == "/photobooth/clear") {
            onClear();
            FCGX_FPrintF(_request.out, ("\n\n" + fname).c_str());
        }else if (req.url == "/photobooth/print") {
            onPrint();
            FCGX_FPrintF(_request.out, ("\n\n" + fname).c_str());
        }

        FCGX_Finish_r(&_request);
    }
}
