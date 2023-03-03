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

#ifndef VEIL_FABRIC_SRC_THREADING_HANDSHAKE_HPP
#define VEIL_FABRIC_SRC_THREADING_HANDSHAKE_HPP

#include "src/typedefs.hpp"
#include "src/threading/os.hpp"
#include "src/memory/global.hpp"

namespace veil::threading {

    class HandShake : memory::ValueObject {
    public:
        static const uint32 TIK = 0, TOK = 1;

        HandShake();

        bool tik();

        bool tok();

    private:
        os::atomic_u32 internal_state;
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_HANDSHAKE_HPP
