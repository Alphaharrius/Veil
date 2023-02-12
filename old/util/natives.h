#ifndef VEIL_NATIVES_H
#define VEIL_NATIVES_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#elif defined(__linux__) || defined(__linux) || defined(linux)
#include <sys/mman.h>
#endif
#include "type-alias.h"

namespace veil::natives {

    enum SystemType {WINDOWS, LINUX};

#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    const SystemType SYSTEM_TYPE = WINDOWS;
#   elif defined(__linux__) || defined(__linux) || defined(linux)
    const SystemType SYSTEM_TYPE = LINUX;
#   endif

    struct OperationStatus {
        const SystemType system_type = SYSTEM_TYPE;
        bool success = false;
        uint32 status_code = 0;
    };

    uint32 get_page_size();

    void *virtual_allocate(uint64 request_size, OperationStatus &allocation_status);

    void virtual_free(void *allocated_address, uint64 allocated_size, OperationStatus &release_status);

}

#endif //VEIL_NATIVES_H
