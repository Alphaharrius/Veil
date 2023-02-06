#ifndef VEIL_RESOURCE_IMPL_H
#define VEIL_RESOURCE_IMPL_H

#include <type_traits>

#include "resource.h"

namespace veil::util {

    template<typename T>
    Resource<T>::Resource() {
        static_assert(std::is_base_of<Resource<T>, T>::value,
                      "Subclasses `T` of `Resource` must extends `Resource<T>.`");

        this->prev = static_cast<T *>(this);
        this->next = static_cast<T *>(this);
    }

    template<typename T>
    T *Resource<T>::get_prev() { return this->prev; }

    template<typename T>
    T *Resource<T>::get_next() { return this->next; }

    template <typename T>
    void Resource<T>::insert(Resource<T> *group) {
        static_cast<Resource<T> *>(this->next)->prev = group->prev;
        static_cast<Resource<T> *>(group->prev)->next = this->next;

        this->next = group;
        group->prev = this;
    }

    template <typename T>
    void Resource<T>::pop(Resource<T> *elem) {
        static_cast<Resource<T> *>(elem->next)->prev = elem->prev;
        static_cast<Resource<T> *>(elem->prev)->next = elem->next;

        elem->prev = elem;
        elem->next = elem;
    }

}

#endif //VEIL_RESOURCE_IMPL_H
