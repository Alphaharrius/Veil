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
#include "src/vm/diagnostics.hpp"

namespace veil::vm {

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
        explicit HasName(std::string name);

        std::string get_name();

    private:
        std::string name;
    };

    template<class R>
    class HasRoot {
    public:
        explicit HasRoot(R &root) : target(&root) {};

        HasRoot() : target(nullptr) {};

        void bind(R &root);

        void unbind();

        R *root();

    protected:
        R *target;
    };

    template<class R>
    void HasRoot<R>::bind(R &r) {
        VeilAssert(this->HasRoot<R>::target == nullptr, "Rebinding target.");
        this->HasRoot<R>::target = &r;
    }

    template<class R>
    void HasRoot<R>::unbind() {
        this->HasRoot<R>::target = nullptr;
    }

    template<class R>
    R *HasRoot<R>::root() {
        VeilAssert(this->HasRoot<R>::target != nullptr, "Root not bind.");
        return this->HasRoot<R>::target;
    }

    template<class M>
    class HasMember {
    public:
        HasMember() : target(nullptr) {}

        explicit HasMember(M &member) : target(member) {}

        void bind(M &m);

        void unbind();

        M *member();

    private:
        M *target;
    };

    template<class M>
    M *HasMember<M>::member() {
        VeilAssert(this->HasMember<M>::target != nullptr, "Member not bind.");
        return this->HasMember<M>::target;
    }

    template<class M>
    void HasMember<M>::bind(M &m) {
        VeilAssert(this->HasMember<M>::target == nullptr, "Rebinding target.");
        this->HasMember<M>::target = &m;
    }

    template<class M>
    void HasMember<M>::unbind() {
        this->HasMember<M>::target = nullptr;
    }

    class Executable {
    public:
        virtual void execute() = 0;
    };

    template<typename T>
    class Consumer {
    public:
        virtual void execute(T param) = 0;
    };

    template<typename A, typename B>
    class BiConsumer {
    public:
        virtual void execute(A a, B b) = 0;
    };

    template<typename P, typename R>
    class Function {
    public:
        virtual R execute(P param) = 0;
    };

}

#endif //VEIL_FABRIC_SRC_STRUCTURES_HPP
