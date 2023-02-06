#include "natives.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h>

#elif defined(__linux__) || defined(__linux) || defined(linux)
#include <sys/mman.h>
#endif

namespace veil::natives {

    uint32 get_page_size() {
#       if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        _SYSTEM_INFO system_info {};
        GetSystemInfo(&system_info);
        return static_cast<uint32>(system_info.dwPageSize);
#       elif defined(__linux__) || defined(__linux) || defined(linux)
        // TODO: Implement Linux usage.
#       endif
    }

    uint8 *malloc(uint64 size) {
        void *allocated_address = nullptr;
#       if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        allocated_address = VirtualAlloc((LPVOID) nullptr,
                                         (SIZE_T) size,
                                         MEM_RESERVE | MEM_COMMIT,
                                         PAGE_READWRITE);
#       elif defined(__linux__) || defined(__linux) || defined(linux)
        // TODO: Implement Linux usage.
#       endif
        return static_cast<uint8 *>(allocated_address);
    }

}
