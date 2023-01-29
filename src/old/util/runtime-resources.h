#ifndef VEIL_RUNTIME_RESOURCES_H
#define VEIL_RUNTIME_RESOURCES_H

#include <type_traits>
#include <functional>

namespace veil::util {

    template <typename T> class ReusableResource {
    public:
        virtual void reset() {};
        virtual void reuse(T reuse_info) {};
    };

    template <class T> class ChainableResource {
    public:
        ChainableResource();
        T *get_next();
        T *get_previous();
        void insert_before(T *element);
        void insert_after(T *element);
        T *unchain();
        void iterate_clockwise(
                std::function<T *(T *anchor_element)> start_from,
                std::function<void(T *anchor_element, T *current_element)> callback,
                std::function<bool(T *anchor_element, T *current_element)> stop_condition);
        void iterate_anticlockwise(
                std::function<T *(T *anchor_element)> start_from,
                std::function<void(T *anchor_element, T *current_element)> callback,
                std::function<bool(T *anchor_element, T *current_element)> stop_condition);
    private:
        T *previous;
        T *next;
    };

    template<class T>
    ChainableResource<T>::ChainableResource() {
        static_assert(std::is_base_of<ChainableResource<T>, T>::value,
                "Subclasses `T` of `ChainableResource` must extends `ChainableResource<T>.`");
        this->previous = static_cast<T *>(this);
        this->next = static_cast<T *>(this);
    }

    template<class T>
    T *ChainableResource<T>::get_previous() {
        return this->previous;
    }

    template<class T>
    T *ChainableResource<T>::get_next() {
        return this->next;
    }

    template<class T>
    void ChainableResource<T>::insert_before(T *element) {
        T *element_previous = static_cast<ChainableResource<T> *>(element)->previous;
        static_cast<ChainableResource<T> *>(element_previous)->next = static_cast<T *>(this);
        static_cast<ChainableResource<T> *>(element)->previous = this->previous;
        static_cast<ChainableResource<T> *>(this->previous)->next = element;
        this->previous = element_previous;
    }

    template<class T>
    void ChainableResource<T>::insert_after(T *element) {
        T *element_next = static_cast<ChainableResource<T> *>(element)->next;
        static_cast<ChainableResource<T> *>(element_next)->previous = this->previous;
        static_cast<ChainableResource<T> *>(this->previous)->next = element_next;
        this->previous = element;
        static_cast<ChainableResource<T> *>(element)->next = static_cast<T *>(this);
    }

    template<class T>
    T *ChainableResource<T>::unchain() {
        static_cast<ChainableResource<T> *>(this->previous)->next = this->next;
        static_cast<ChainableResource<T> *>(this->next)->previous = this->previous;
        this->next = nullptr;
        this->previous = nullptr;
        return this;
    }

    template<class T>
    void ChainableResource<T>::iterate_clockwise(std::function<T *(T *)> start_from,
                                                 std::function<void(T *, T *)> callback,
                                                 std::function<bool(T *, T *)> stop_condition) {
        T *anchor_element = static_cast<T *>(this);
        T *current_element = start_from(anchor_element);
        do {
            T *next_element = static_cast<ChainableResource<T> *>(current_element)->next;
            callback(anchor_element, current_element);
            current_element = next_element;
        } while(!stop_condition(anchor_element, current_element));
    }

    template<class T>
    void ChainableResource<T>::iterate_anticlockwise(std::function<T *(T *)> start_from,
                                                     std::function<void(T *, T *)> callback,
                                                     std::function<bool(T *, T *)> stop_condition) {
        T *anchor_element = static_cast<T *>(this);
        T *current_element = start_from(anchor_element);
        do {
            T *previous_element = static_cast<ChainableResource<T> *>(current_element)->previous;
            callback(anchor_element, current_element);
            current_element = previous_element;
        } while(!stop_condition(anchor_element, current_element));
    }

}

#endif //VEIL_RUNTIME_RESOURCES_H
