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

#include "src/threading/handshake.hpp"

using namespace veil::threading;

HandShake::HandShake() : internal_state(TIK) {}

bool HandShake::tik() { return internal_state.compare_exchange(TIK, TOK); }

bool HandShake::tok() { return internal_state.compare_exchange(TOK, TIK); }

bool HandShake::is_tik() { return internal_state.load() == TIK; }

bool HandShake::is_tok() { return internal_state.load() == TOK; }
