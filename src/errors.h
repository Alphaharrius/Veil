#ifndef VEIL_ERRORS_H
#define VEIL_ERRORS_H

#include "typedefs.h"

// The error handling approach of the Veil runtime is to use error codes confined to their resultant namespace level,
// each level will map the error code from the previous level to their own error code. This header is used side by side
// with the request & executor structure defined in 'diagnostics.h'.

namespace veil {

    /// Defines no error presence in the current operation.
    static const uint32 ERR_NONE = 0;

}

namespace veil::natives {

    static const uint32 ERR_NOMEM = 1;

}

namespace veil::memory {

    static const uint32 ERR_HEAP_OVERFLOW = 1;
    static const uint32 ERR_HOST_NOMEM = 2;

    static const uint32 ERR_INV_HEAP_SIZE = 3;
    static const uint32 ERR_NO_ALGO = 4;
    static const uint32 ERR_ALGO_INIT = 5;

}

#endif //VEIL_ERRORS_H
