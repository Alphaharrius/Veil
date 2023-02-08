#include "runtime.h"

namespace veil {

    RuntimeConstituent::RuntimeConstituent(Runtime &runtime) : runtime(&runtime) {}

    const Runtime *RuntimeConstituent::get_runtime() {
        return this->runtime;
    }

}
