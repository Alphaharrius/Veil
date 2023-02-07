#ifndef VEIL_DIAGNOSTICS_H
#define VEIL_DIAGNOSTICS_H

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

#include <string>

#include "typedefs.h"

namespace veil::util {

    class RequestConsumer;

    class Request {
    public:
        static const uint32 STATUS_OK = 0;

        Request(): status(STATUS_OK) {}

        [[nodiscard]] bool is_ok() const {
            return this->status == STATUS_OK;
        }

        [[nodiscard]] uint32 get_status() const {
            return status;
        }

    protected:
        uint16 status;

        friend class RequestConsumer;
    };

    class RequestConsumer {
    public:
        virtual std::string get_status_information(uint16 status) = 0;
    };

}

#endif //VEIL_DIAGNOSTICS_H
