#ifndef VEIL_NATIVES_H
#define VEIL_NATIVES_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#elif defined(__linux__) || defined(__linux) || defined(linux)
#include <sys/mman.h>
#endif
#include "util/type-alias.h"

namespace veil::natives {

    enum SystemType {WINDOWS, LINUX};

#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    const SystemType SYSTEM_TYPE = WINDOWS;
#   elif defined(__linux__) || defined(__linux) || defined(linux)
    const SystemType SYSTEM_TYPE = LINUX;
#   endif

    struct OperationStatus {
        veil::natives::SystemType system_type;
        bool success;
        uint32 status_code;
    };

    uint32 get_page_size() {
#       if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        _SYSTEM_INFO system_info {};
        GetSystemInfo(&system_info);
        return static_cast<uint32>(system_info.dwPageSize);
#       endif
    }

    void *virtual_allocate(uint64 request_size, OperationStatus &allocation_status) {
#       if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        void *allocated_address = VirtualAlloc((LPVOID) nullptr,
                                               (SIZE_T) request_size,
                                               MEM_RESERVE | MEM_COMMIT,
                                               PAGE_READWRITE);
#       elif defined(__linux__) || defined(__linux) || defined(linux)
#       endif
        if (allocated_address != nullptr) {
            allocation_status.success = false;
#           if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
            allocation_status.status_code = static_cast<uint32>(GetLastError());
#           endif
        } else {
            allocation_status.success = true;
        }
        allocation_status.system_type = SYSTEM_TYPE;
        return allocated_address;
    }

    void virtual_free(void *allocated_address, uint64 allocated_size, OperationStatus &release_status) {
    }

}

#endif //VEIL_NATIVES_H
