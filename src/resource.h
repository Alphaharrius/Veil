#ifndef VEIL_RESOURCE_H
#define VEIL_RESOURCE_H

namespace veil::util {

    template<typename T>
    class Resource {
    public:
        Resource();

        T *get_prev();

        T *get_next();

        void pop(Resource<T> *elem);

        void insert(Resource<T> *group);

    private:
        T *prev;
        T *next;
    };

}

#endif //VEIL_RESOURCE_H
