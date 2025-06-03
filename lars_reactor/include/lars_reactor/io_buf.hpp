#ifndef __IO_BUF_HPP__
#define __IO_BUF_HPP__

namespace qc {

/**
 * @brief buffer存放数据的结构
 */
class io_buf {
public:
    io_buf(int capacity);
    /// @brief 清除buffer中的内容
    void clear();
    /// @brief 将other处的buffer copy 到当前data中
    /// @param other 
    void copy(const io_buf *other);
    /// @brief pop长度为len的数据
    /// @param len 
    void pop(int len);
    /// @brief 将已经处理过的数据，清空,将未处理的数据提前至数据首地址
    void adjust();

	/// @brief 下面是一种更高效的内存管理方式
	//union {
	//	io_buf *next;
	//	int m_capacity;
	//	int m_length;
	//	int m_head;
	//	char *m_data;
	//};

    /// @brief 如果有多个buffer,采用链表链接起来
    io_buf *next;
    /// @brief buffer容量大小
    int m_capacity;
    /// @brief buffer有效数据长度
    int m_length;
    /// @brief 未处理数据的头部位置索引,head之前的都是处理过的
    int m_head;
    /// @brief 当前io_buf所保存的数据地址
    char *m_data;
};

}  // namespace qc

#endif
