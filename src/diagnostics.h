#ifndef VEIL_DIAGNOSTICS_H
#define VEIL_DIAGNOSTICS_H

#include <cassert>
#include <string>

#include "typedefs.h"

#define not_null(pointer) assert(pointer != nullptr)

namespace veil::util {

    class RequestConsumer;

    class Request {
    public:
        static const uint16 STATUS_OK = 0;

        Request(): status(STATUS_OK) {}

        [[nodiscard]] bool is_ok() const {
            return this->status == STATUS_OK;
        }

        [[nodiscard]] uint16 get_status() const {
            return status;
        }

    protected:
        uint16 status;

        friend class RequestConsumer;
    };

    class RequestConsumer {
    public:
        virtual std::string get_status_information(uint16 status) = 0;
    };

}

#endif //VEIL_DIAGNOSTICS_H
