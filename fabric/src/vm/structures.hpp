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

#ifndef VEIL_FABRIC_SRC_STRUCTURES_HPP
#define VEIL_FABRIC_SRC_STRUCTURES_HPP

#include <string>

#include "src/typedefs.hpp"
#include "src/vm/errors.hpp"
#include "src/memory/global.hpp"
#include "src/vm/os.hpp"

namespace veil::vm {

    /// Place this macro with proper reason if a fault state happens due to VM implementation instead of runtime events.
#   define VeilExitOnImplementationFault(reason) VeilForceExitOnError("ImplementationFault(" reason ")")

    class RequestExecutor;

    class Request : public memory::ValueObject {
    public:

        Request() : error(ERR_NONE) {}

        [[nodiscard]] bool is_ok() const {
            return this->error == ERR_NONE;
        }

        [[nodiscard]] uint32 get_error() const {
            return error;
        }

    protected:
        uint32 error;

        friend class RequestExecutor;
    };

    class RequestExecutor {
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

    template<class R>
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
        Composite() : composition(nullptr) {}

        void bind(C &composition);

        C *get_composition();

    private:
        C *composition;
    };

    template<class C>
    C *Composite<C>::get_composition() {
        if (!this->composition)
            VeilExitOnImplementationFault("Access composition before registering one.");
        return this->composition;
    }

    template<class C>
    void Composite<C>::bind(C &c) {
        if (this->composition)
            VeilExitOnImplementationFault("Registering a registered composition.");
        this->composition = &c;
    }

    class Callable {
    public:
        virtual void run() = 0;
    };

}

#endif //VEIL_FABRIC_SRC_STRUCTURES_HPP
