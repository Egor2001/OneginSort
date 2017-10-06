#ifndef ON_OUTPUT_FILE_H_INCLUDED
#define ON_OUTPUT_FILE_H_INCLUDED

#include <stdio.h>
#include <cstring>

#include "io.h"
#include "share.h"
#include "fcntl.h"
#include "sys/stat.h"

#include "Logging/ONlogging.h"
#include "../ONdefines.h"
#include "ONtextBuffer.h"

namespace onegin_sort
{

class IAbstractOutput
{
public:
    IAbstractOutput() = default;

    IAbstractOutput             (const IAbstractOutput&) = delete;
    IAbstractOutput& operator = (const IAbstractOutput&) = delete;

    virtual ~IAbstractOutput() = default;

    virtual void write_text_buffer(const CAbstractTextBuffer* text_buffer_ptr) = 0;

    virtual bool ok()   const = 0;
    virtual void dump() const = 0;
};

class CFileOutput: public IAbstractOutput
{
public:
    CFileOutput(const char* file_name): IAbstractOutput(), descriptor_{-1}
    {
        ON_CHECK_NOT_EQ(file_name, nullptr);

        int open_mode = _O_TEXT | _O_SEQUENTIAL | _O_WRONLY;
        descriptor_ = ON_CHECK_SYSCALL(_sopen(file_name, open_mode, _SH_DENYNO));

        ON_CHECK_OK(this);
    }

    virtual ~CFileOutput()
    {
        ON_CHECK_OK(this);

        ON_CHECK_SYSCALL(_close(descriptor_));
        descriptor_ = -1;
    }

    virtual void write_text_buffer(const CAbstractTextBuffer* text_buffer_ptr) override
    {
        ON_CHECK_OK(this);
        ON_CHECK_NOT_EQ(text_buffer_ptr, nullptr);

        size_t       lines_buf_size = text_buffer_ptr->get_lines_buf_size();
        const SLine* lines_buf      = text_buffer_ptr->get_lines_buf();

        for (size_t i = 0; i < lines_buf_size-1; i++)
            ON_CHECK_SYSCALL(_write(descriptor_, lines_buf[i].start_pos,
                                    static_cast<size_t>(lines_buf[i].end_pos -
                                                        lines_buf[i].start_pos)+1));

        ON_CHECK_OK(this);
    }

    virtual bool ok() const override
    {
        return descriptor_ != -1;
    }

    virtual void dump() const override
    {
        ON_LOG_MSG("[descriptor: %d]", descriptor_);
    }

private:
    int descriptor_;
};

class CMappedOutput: public IAbstractOutput
{
public:
    CMappedOutput(const CMapping& mapping_class):
        IAbstractOutput(),
        file_view_size_(mapping_class.get_file_length()),
        file_view_     ((char*)MapViewOfFile(mapping_class.get_map_handle(),
                                             FILE_MAP_WRITE, 0, 0, file_view_size_))
    {
        ON_CHECK_NOT_EQ(file_view_, nullptr);

        ON_CHECK_OK(this);
    }

    virtual void write_text_buffer(const CAbstractTextBuffer* text_buffer_ptr) override
    {
        ON_CHECK_OK(this);
        ON_CHECK_NOT_EQ(text_buffer_ptr, nullptr);

        size_t       lines_buf_size = text_buffer_ptr->get_lines_buf_size();
        const SLine* lines_buf      = text_buffer_ptr->get_lines_buf();

        char* cur_pos = file_view_;
        size_t cur_copy_size = 0;

        for (size_t i = 0; i < lines_buf_size-1; i++)
        {
            cur_copy_size = static_cast<size_t>((lines_buf[i].end_pos - lines_buf[i].start_pos)+1);

            if ((cur_pos - file_view_) + cur_copy_size >= file_view_size_)
                break;

            memcpy(cur_pos, lines_buf[i].start_pos, cur_copy_size);

            cur_pos += cur_copy_size;
        }

        ON_CHECK_OK(this);
    }

    virtual bool ok() const override
    {
        return file_view_ && file_view_size_;
    }

    virtual void dump() const override
    {
        ON_LOG_MSG("[file_view: [size: %d, ptr: %p]]", file_view_size_, file_view_);
    }

private:
    DWORD file_view_size_;
    char* file_view_;
};

}//namespace onegin_sort

#endif//ON_OUTPUT_FILE_H_INCLUDED

