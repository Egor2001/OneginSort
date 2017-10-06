#ifndef ON_INPUT_FILE_H_INCLUDED
#define ON_INPUT_FILE_H_INCLUDED

#include <stdio.h>
#include <cstring>

#include "io.h"
#include "share.h"
#include "fcntl.h"
#include "sys/stat.h"

#include "Logging/ONlogging.h"
#include "../ONdefines.h"

namespace onegin_sort
{

class CInputFile
{
public:
    CInputFile(const char* file_name): descriptor_{-1}
    {
        int open_mode = _O_TEXT | _O_SEQUENTIAL | _O_RDONLY;
        descriptor_ = ON_CHECK_SYSCALL(_sopen(file_name, open_mode, _SH_DENYNO));

        ON_CHECK_SYSCALL(_lseeki64(descriptor_, 0, SEEK_END));
        file_size_ = ON_CHECK_SYSCALL(_telli64(descriptor_));
        ON_CHECK_SYSCALL(_lseeki64(descriptor_, 0, SEEK_SET));

        ON_CHECK_OK(this);
    }

    ~CInputFile()
    {
        ON_CHECK_OK(this);

        ON_CHECK_SYSCALL(_close(descriptor_));
        descriptor_ = -1;
        ON_POISON(file_size_);
    }

    size_t get_text_buf_size() const { return file_size_ + 1; }

    char* allocate_text_buf() const
    {
        ON_CHECK_OK(this);

        char* result = nullptr;

        size_t text_buf_size = get_text_buf_size();

        result = static_cast<char*>(calloc(text_buf_size, sizeof(char)));
        ON_CHECK_NOT_EQ(result, nullptr);

        ON_CHECK_SYSCALL(_read(descriptor_, result, file_size_));

        ON_CHECK_OK(this);

        return result;
    }

    bool ok() const
    {
        return (descriptor_ != -1) && !ON_IS_POISON(file_size_);
    }

    void dump() const
    {
        ON_LOG_MSG("[descriptor: %d, size: %d]", descriptor_, file_size_);
    }

private:
    int    descriptor_;
    size_t file_size_;
};

}//namespace onegin_sort

#endif//ON_INPUT_FILE_H_INCLUDED

