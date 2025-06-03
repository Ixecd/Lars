#pragma once
#include <memory>

namespace qc {

/**
 * @brief 单例模式(指针封装)
 * 
 * @tparam T 
 * @tparam X 
 * @tparam N 
 */
template <class T, class X = void, int N = 0>
class Singleton {
public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
};

template <class T, class X = void, int N = 0>
class SingletonPtr {
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    } 
};

}  // namespace qc
