#include "natives.h"
#include "errors.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)

#include <sys/mman.h>
#include <unistd.h>

#endif

using namespace veil::natives;

uint32 get_page_size() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    _SYSTEM_INFO system_info{};
    GetSystemInfo(&system_info);
    return static_cast<uint32>(system_info.dwPageSize);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    return (uint32) getpagesize();
#   endif
}

Mmap::Mmap(
        void *address,
        uint64 size,
        bool readwrite,
        bool reserve) : address(address), size(size), readwrite(readwrite), reserve(reserve) {}

bool Mmap::access() {
    void *allocated_address = nullptr;
    bool success;
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    allocated_address = VirtualAlloc((LPVOID) nullptr,
                                     (SIZE_T) size,
                                     (reserve ? MEM_RESERVE | MEM_COMMIT : 0),
                                     (readwrite ? PAGE_READWRITE : 0));
    success = allocated_address != nullptr;
    if (!success) {
        switch ((uint32) GetLastError()) {
        case ERROR_NOT_ENOUGH_MEMORY: this->error = ERR_NOMEM;
        }
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    allocated_address = mmap(nullptr,
                             size,
                             (readwrite ? PROT_READ | PROT_WRITE : 0),
                             MAP_PRIVATE | (reserve ? 0 : MAP_NORESERVE),
                             -1, 0);
    this->result = static_cast<uint8 *>(allocated_address);
    success = allocated_address != MAP_FAILED;
    if (!success) {
        switch ((uint32) errno) {
            case ENOMEM:
                return ERR_NOMEM;
        }
    }
#   endif
    return success;
}
