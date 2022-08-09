#include "natives.h"
#include "util/diagnostics.h"

uint32 veil::natives::get_page_size() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    _SYSTEM_INFO system_info {};
    GetSystemInfo(&system_info);
    return static_cast<uint32>(system_info.dwPageSize);
#   endif
}

void *veil::natives::virtual_allocate(uint64 request_size, OperationStatus &allocation_status) {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    void *allocated_address = VirtualAlloc((LPVOID) nullptr,
                                           (SIZE_T) request_size,
                                           MEM_RESERVE | MEM_COMMIT,
                                           PAGE_READWRITE);
#   endif
    if (allocated_address == nullptr) {
        allocation_status.success = false;
#       if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
        allocation_status.status_code = static_cast<uint32>(GetLastError());
#       endif
    } else {
        allocation_status.success = true;
    }
    return allocated_address;
}

void veil::natives::virtual_free(void *allocated_address, uint64 allocated_size, OperationStatus &release_status) {
    not_null(allocated_address);

#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    int success = VirtualFree(allocated_address, allocated_size, MEM_RELEASE);
    if (success == 0) {
        release_status.success = false;
        release_status.status_code = static_cast<uint32>(GetLastError());
        return;
    }
#   endif
    release_status.success = true;
}
