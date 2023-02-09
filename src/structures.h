#ifndef VEIL_STRUCTURES_H
#define VEIL_STRUCTURES_H

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
#include "errors.h"

namespace veil::util {

    class RequestConsumer;

    class Request {
    public:

        Request(): error(ERR_NONE) {}

        [[nodiscard]] bool is_ok() const {
            return this->error == ERR_NONE;
        }

        [[nodiscard]] uint32 get_error() const {
            return error;
        }

    protected:
        uint32 error;

        friend class RequestConsumer;
    };

    class RequestConsumer {
    public:
        virtual std::string get_error_info(uint32 status) = 0;

    protected:
        static void set_error(Request &request, uint32 error) {
            request.error = error;
        }
    };

    template <class P>
    class Constituent {
    public:
        P *get_parent();

    protected:
        explicit Constituent(P &parent);

    private:
        const P *parent;
    };

    template<class P>
    P *Constituent<P>::get_parent() {
        return this->parent;
    }

    template<class P>
    Constituent<P>::Constituent(P &parent) : parent(&parent) {}

}

#endif //VEIL_STRUCTURES_H
