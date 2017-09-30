#ifndef __SQLITE_TYPES_HPP__
#define __SQLITE_TYPES_HPP__

namespace wspp { namespace util { namespace sqlite {

class NullType {} ;

// Wraps pointer to buffer and its size. Memory management is external
class Blob {
public:

    Blob(const char *data, uint32_t sz): size_(sz), data_(data) {}

    const char *data() const { return data_ ; }
    uint32_t size() const { return size_ ; }

private:
    const char *data_ = nullptr;
    uint32_t size_ = 0 ;
};

} // namespace sqlite
} // namespace util
} // namespace wspp


#endif
