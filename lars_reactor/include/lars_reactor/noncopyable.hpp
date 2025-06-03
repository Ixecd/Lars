#ifndef __NONCOPYABLE_HPP__
#define __NONCOPYABLE_HPP__

namespace qc {

class Noncopyable {
public:
    // 默认构造函数
    Noncopyable() = default;
    // 默认析构函数
    ~Noncopyable() = default;
    // 拷贝构造函数
    Noncopyable(const Noncopyable&) = delete;
    // 赋值函数
    Noncopyable& operator=(const Noncopyable&) = delete;
};

}  // namespace qc

#endif
