#pragma once

#include <map>
#include <functional>

#include "photobooth.h"
#include "watchable_fd.h"


class webrequest {
public:
    std::string url;
    std::string method;
};

class webserver {
private:
    watchable_fd _fd;
    FCGX_Request _request;

    std::map<std::string, std::function<void(webrequest &req)>> mRouteTable;

    void request_basic(webrequest &req);

    void _thread(void);

public:
    webserver(Context &ctx, std::string socket);
    watchable_fd fd();
    void ExecuteRequest();
};
