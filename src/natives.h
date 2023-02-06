#ifndef VEIL_NATIVES_H
#define VEIL_NATIVES_H

#include "typedefs.h"
#include "diagnostics.h"

namespace veil::natives {

    uint32 get_page_size();

    uint8 *malloc(uint64 size);

}

#endif //VEIL_NATIVES_H
