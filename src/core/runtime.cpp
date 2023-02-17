#include "runtime.hpp"

veil::Runtime::Runtime(veil::memory::Management &memory_management, veil::threading::Management &threading_management) :
        vm::Composite<memory::Management>(memory_management),
        vm::Composite<threading::Management>(threading_management) {}
