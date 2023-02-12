#ifndef VEIL_RESOURCE_H
#define VEIL_RESOURCE_H

namespace veil::util {

    template<typename T>
    class Storage {
    public:
        static const uint32 INITIAL_BUFFER_LEN = 64;

        explicit Storage();

        T get(uint32 index);

        T pop();

        void push(T el);

        uint32 get_top_index();

    protected:
        T *buffer;
        uint32 buffer_len;
        uint32 buffered_count;
    };

    template<typename T>
    Storage<T>::Storage() : buffer(new T[INITIAL_BUFFER_LEN]), buffer_len(INITIAL_BUFFER_LEN), buffered_count(0) {}

    template<typename T>
    T Storage<T>::get(uint32 index) {
        return this->buffer[index];
    }


    template<typename T>
    T Storage<T>::pop() {
        return buffer[this->buffered_count--];
    }

    template<typename T>
    void Storage<T>::push(T el) {
        if (this->buffered_count == this->buffer_len) {
            auto *new_buffer = new T[this->buffer_len * 1.5f];
            memcpy(new_buffer, this->buffer, this->buffered_count);
            delete[] this->buffer;
            this->buffer = new_buffer;
        }
        this->buffer[this->buffered_count++] = el;
    }

    template<typename T>
    uint32 Storage<T>::get_top_index() {
        return this->buffered_count;
    }
}

#endif //VEIL_RESOURCE_H
