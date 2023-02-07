#ifndef VEIL_NATIVES_H
#define VEIL_NATIVES_H

#include "typedefs.h"
#include "diagnostics.h"

namespace veil::natives {

    uint32 get_page_size();

    uint64 atomic_fetch_and_add(const volatile uint64 *target, uint64 value);

    template <typename T>
    class NativeAccess {
    public:
        virtual bool access() = 0;

        virtual uint32 error() = 0;

        T get_result();

    protected:
        T result;
    };

    template<typename T>
    T NativeAccess<T>::get_result() {
        return this->result;
    }

    class Mmap : public NativeAccess<uint8 *> {
    public:
        static const uint32 ERR_NOMEM = 1;

        Mmap(void *address, uint64 size, bool readwrite, bool reserve);

        bool access() override;

        uint32 error() override;

    private:
        void *address;
        uint64 size;
        bool readwrite, reserve;
    };

}

#endif //VEIL_NATIVES_H
