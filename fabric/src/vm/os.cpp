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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#endif

#include "os.hpp"

uint64 veil::os::current_time_milliseconds() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    FILETIME now;
    GetSystemTimeAsFileTime(&now);

    // The milliseconds elapsed from 00:00 1/1/1601 to now.
    uint64 millis_1_1_1601 = ((LONGLONG) now.dwLowDateTime + ((LONGLONG) (now.dwHighDateTime) << 32LL)) / 10000ULL;
    // The milliseconds elapsed from 00:00 1/1/1601 to 00:00 1/1/1970.
    uint64 unix_epoch_1_1_1601 = 11644473600000ULL;
    return millis_1_1_1601 - unix_epoch_1_1_1601;
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}
