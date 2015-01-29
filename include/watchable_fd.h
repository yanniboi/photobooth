#pragma once
#include <fcgiapp.h>

class watchable_fd {
public:
    int fd;
    void(*onReadable)(watchable_fd);
    void *obj;
};