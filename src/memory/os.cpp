/// This file is part of the Veil distribution (https://github.com/Alphaharrius/Veil).
/// Copyright (c) 2023 Alphaharrius.
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, version 3.
///
/// This program is distributed in the hope that it will be useful, but
/// WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
/// General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "os.hpp"
#include "errors.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)

#include <sys/mman.h>
#include <unistd.h>

#endif

uint32 veil::os::get_page_size() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    _SYSTEM_INFO system_info{};
    GetSystemInfo(&system_info);
    return static_cast<uint32>(system_info.dwPageSize);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    return (uint32) getpagesize();
#   endif
}

uint8 *veil::os::mmap(void *address, uint64 size, bool readwrite, bool reserve, uint32 &error) {
    error = veil::ERR_NONE;
    uint8 *allocated_address = nullptr;
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    allocated_address = (uint8 *) VirtualAlloc((LPVOID) nullptr,
                                               (SIZE_T) size,
                                               (reserve ? MEM_RESERVE | MEM_COMMIT : 0),
                                               (readwrite ? PAGE_READWRITE : 0));
    if (!allocated_address) {
        switch ((uint32) GetLastError()) {
        case ERROR_NOT_ENOUGH_MEMORY: error = ERR_NOMEM;
        }
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    allocated_address = (uint8 *) mmap(nullptr,
                                       size,
                                       (readwrite ? PROT_READ | PROT_WRITE : 0),
                                       MAP_PRIVATE | (reserve ? 0 : MAP_NORESERVE),
                                       -1, 0);
    success = allocated_address != MAP_FAILED;
    if (allocated_address == MAP_FAILED) {
        switch ((uint32) errno) {
        case ENOMEM: error = ERR_NOMEM;
        }
    }
#   endif
    return allocated_address;
}
