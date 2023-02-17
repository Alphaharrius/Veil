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

#ifndef VEIL_SRC_STRUCTURES_HPP
#define VEIL_SRC_STRUCTURES_HPP

#include <string>

#include "src/typedefs.hpp"
#include "src/errors.hpp"
#include "src/memory/memory.hpp"

namespace veil::vm {

    class RequestConsumer;

    class Request : public memory::ValueObject {
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

    class HasName {
    public:
        explicit HasName(std::string &name);

        std::string get_name();

    private:
        std::string name;
    };

    template <class R>
    class Constituent {
    public:
        explicit Constituent(R &root);

        R *get_root();

    protected:
        R *root;
    };

    template<class P>
    P *Constituent<P>::get_root() {
        return this->root;
    }

    template<class P>
    Constituent<P>::Constituent(P &root) : root(&root) {}

    template<class C>
    class Composite {
    public:
        explicit Composite(C &composition) : composition(&composition) {}

        C *get_composition();

    private:
        C *composition;
    };

    template<class C>
    C *Composite<C>::get_composition() {
        return composition;
    }

}

#endif //VEIL_SRC_STRUCTURES_HPP