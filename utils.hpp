#ifndef UTILS_HPP
#define UTILS_HPP

#include <typeinfo>
#include <memory>

template <typename T>
class Shared {
private:
    struct Info {
        long count;
        T * val;
    } * info;

public:
    Shared() {
        info = new Info { .count = 1, .val = nullptr };
    }

    explicit Shared(T *value) {
        info = new Info { .count = 1, .val = value };
    }

    Shared(std::nullptr_t nil) {
        info = new Info { .count = 1, .val = nullptr };
    }

    Shared(const Shared& another): info(another.info) {
        info->count++;
    }

    Shared(Shared&& another) noexcept: info(another.info) {
        another.info = nullptr;
    }

    ~Shared() {
        if (info != nullptr && --(info->count) == 0) {
            delete info->val;
            delete info;
        }
    }

    Shared& operator=(const Shared& another) {
        if (this != &another) {
            if (--(info->count) == 0) {
                delete info->val;
                delete info;
            }
            info = another.info;
            info->count++;
        }
        return *this;
    }

    Shared& operator=(Shared&& another) noexcept {
        info = another.info;
        another.info = nullptr;
        return *this;
    }

    Shared& operator=(T * value) {
        if (--(info->count) == 0) {
            delete info->val;
            delete info;
        }
        info = new Info { .count = 1, .val = value };
        return *this;
    }

    T * get() const {
        return info->val;
    }

    T& value() const {
        return *(info->val);
    }

    T * operator->() const {
        return info->val;
    }

    T& operator*() const {
        return *(info->val);
    }

    T& operator[](std::ptrdiff_t idx) const {
        return (info->val)[idx];
    }

    // data are shared across different containers
    void set(T * value) {
        delete info->val;
        info->val = value;
    }

    operator bool() const {
        return info->val != nullptr;
    }

    bool operator==(const Shared& another) const {
        return info->val == another.info->val;
    }

    bool operator==(const T * another) const {
        return info->val == another;
    }

    bool operator!=(const Shared& another) const {
        return info->val != another.info->val;
    }

    bool operator!=(const T * another) const {
        return info->val != another;
    }

    size_t useCount() {
        return info->count;
    }
};


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


#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
template <class T>
std::string fullTypeName(const T& t) {
    const char * name = typeid(t).name();
    int status = 0;
    char * _demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    if (status == 0) {
        std::string demangled(_demangled);
        std::free(_demangled);
        return demangled;
    }
    return name;
}
#else
template <class T>
std::string fullTypeName(const T& t) {
    return typeid(t).name();
}
#endif

#endif // UTILS_HPP
