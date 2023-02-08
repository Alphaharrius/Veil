#ifndef VEIL_NATIVES_H
#define VEIL_NATIVES_H

#include "typedefs.h"
#include "diagnostics.h"

namespace veil::natives {

    uint32 get_page_size();

    template<typename T>
    class NativeAccess {
    public:
        virtual bool access() = 0;

        T get_result();

        uint32 get_error();

    protected:
        T result;
        uint32 error = 0;
    };

    template<typename T>
    T NativeAccess<T>::get_result() {
        return this->result;
    }

    template<typename T>
    uint32 NativeAccess<T>::get_error() {
        return this->error;
    }

    class Mmap : public NativeAccess<uint8 *> {
    public:
        Mmap(void *address, uint64 size, bool readwrite, bool reserve);

        bool access() override;

    private:
        void *address;
        uint64 size;
        bool readwrite, reserve;
    };

}

#endif //VEIL_NATIVES_H
