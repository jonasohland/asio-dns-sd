#pragma once


#define DNSSD_NO_COPY(classname)                                               \
    classname(const classname&) = delete;                                      \
    void operator=(const classname&) = delete;

#define DNSSD_NO_MOVE(classname)                                               \
    classname(classname&&) = delete;                                           \
    void operator=(classname&&) = delete;