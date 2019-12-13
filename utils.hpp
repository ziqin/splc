#ifndef UTILS_HPP
#define UTILS_HPP

#include <memory>

template<typename T>
inline static bool hasNull(T * ptr) {
    return ptr == nullptr;
}

template<typename V, template<typename, typename...> class Collection, typename... Alloc>
bool hasNull(const Collection<V*, Alloc...>& ptrs) {
    for (auto ptr: ptrs) {
        if (ptr == nullptr) return true;
    }
    return false;
}

template<typename T, typename... Args>
bool hasNull(const T& head, const Args&... tail) {
    return hasNull(head) || hasNull(tail...);
}

template<typename T>
void deleteAll(T*& ptr) {
    delete ptr;
    ptr = nullptr;
}

template<typename V, template<typename, typename...> class Collection, typename... Alloc>
void deleteAll(Collection<V*, Alloc...>& ptrs) {
    for (auto& ptr: ptrs) {
        delete ptr;
        ptr = nullptr;
    }
}

template<typename T, typename... Args>
void deleteAll(T& head, Args&... tail) {
    deleteAll(head);
    deleteAll(tail...);
}

#endif // UTILS_HPP
