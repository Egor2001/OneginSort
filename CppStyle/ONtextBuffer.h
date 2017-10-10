#ifndef ON_TEXT_BUFFER_H_INCLUDED
#define ON_TEXT_BUFFER_H_INCLUDED

#include "ONinputFile.h"
#include "ONmapping.h"

namespace onegin_sort {

struct SLine
{
    const char* start_pos;
    const char* end_pos;
    uint32_t begin_offset;
    uint32_t end_offset;
};

class CAbstractTextBuffer
{
public:
    CAbstractTextBuffer(size_t file_text_buf_size, const char* file_text_buf):
        lines_buf_size_(0), lines_buf_(nullptr), is_constant_buffer_(true)
    {
        ON_CHECK_NOT_EQ(file_text_buf, nullptr);
        ON_CHECK_GT    (file_text_buf_size, 0);

        const char* cur_pos = file_text_buf;
        const char* end_pos = cur_pos + file_text_buf_size;

        while (cur_pos < end_pos)
        {
            if (*cur_pos == '\n')
                lines_buf_size_++;

            cur_pos++;
        }
        lines_buf_size_++;

        lines_buf_ = static_cast<SLine*>(calloc(lines_buf_size_, sizeof(SLine)));
        ON_CHECK_NOT_EQ(lines_buf_, nullptr);

        lines_buf_[0].start_pos = cur_pos = file_text_buf;

        size_t i = 1;

        while (cur_pos < end_pos)
        {
            if (*cur_pos == '\n')
            {
                lines_buf_[i-1].end_pos = cur_pos;
                lines_buf_[i].start_pos = cur_pos+1;

                i++;
            }

            cur_pos++;
        }
        lines_buf_[i-1].end_pos = end_pos-1;

        ON_CHECK_OK(this);
    }

    CAbstractTextBuffer(size_t file_text_buf_size, char* file_text_buf):
        lines_buf_size_(0), lines_buf_(nullptr), is_constant_buffer_(false)
    {
        ON_CHECK_NOT_EQ(file_text_buf, nullptr);
        ON_CHECK_GT    (file_text_buf_size, 0);

        file_text_buf[file_text_buf_size-1] = '\0';

        char* cur_pos = file_text_buf;

        while ((cur_pos = strchr(cur_pos, '\n')))
        {
            lines_buf_size_++;
            cur_pos++;
        }
        lines_buf_size_++;

        lines_buf_ = static_cast<SLine*>(calloc(lines_buf_size_, sizeof(SLine)));
        ON_CHECK_NOT_EQ(lines_buf_, nullptr);

        lines_buf_[0].start_pos = cur_pos = file_text_buf;

        size_t i = 1;

        while ((cur_pos = strchr(cur_pos, '\n')))
        {
            *cur_pos = '\0';
            lines_buf_[i-1].end_pos = cur_pos;
            lines_buf_[i].start_pos = ++cur_pos;

            i++;
        }
        lines_buf_[i-1].end_pos = file_text_buf + file_text_buf_size-1;

        ON_CHECK_OK(this);
    }

    CAbstractTextBuffer             (const CAbstractTextBuffer&) = delete;
    CAbstractTextBuffer& operator = (const CAbstractTextBuffer&) = delete;

    virtual ~CAbstractTextBuffer()
    {
        ON_CHECK_OK(this);

        free(lines_buf_);
        lines_buf_ = nullptr;

        ON_POISON(lines_buf_size_);
    }

    void calculate_offsets()
    {
        ON_CHECK_OK(this);

        for (size_t i = 0; i < lines_buf_size_-1; i++)
        {
            const char* cur_pos = lines_buf_[i].start_pos;

            while (!std::isalpha(*cur_pos) && cur_pos < lines_buf_[i].end_pos)
                cur_pos++;

            if (cur_pos == lines_buf_[i].end_pos)
            {
                lines_buf_[i].begin_offset = 0;
                lines_buf_[i].  end_offset = 0;

                continue;
            }

            lines_buf_[i].begin_offset = cur_pos - lines_buf_[i].start_pos;

            cur_pos = lines_buf_[i].end_pos;

            while (!std::isalpha(*cur_pos)) cur_pos--;

            lines_buf_[i].end_offset = cur_pos - lines_buf_[i].start_pos;
        }

        ON_CHECK_OK(this);
    }

    void split_lines()
    {
        ON_CHECK_OK(this);
        ON_CHECK_NOT_EQ(is_constant_buffer_, true);
        //because it works and just for lulz (I've never used const_cast before):
        for (size_t i = 0; i < lines_buf_size_-1; i++)
            *(const_cast<char*>(lines_buf_[i].end_pos)) = '\0';

        ON_CHECK_OK(this);
    }

    void concat_lines()
    {
        ON_CHECK_OK(this);
        ON_CHECK_NOT_EQ(is_constant_buffer_, true);
        //also because it works (now, I've used const_cast before and it was not good experience):
        for (size_t i = 0; i < lines_buf_size_-1; i++)
            *(const_cast<char*>(lines_buf_[i].end_pos)) = '\n';

        *(const_cast<char*>(lines_buf_[lines_buf_size_-1].end_pos)) = '\0';

        ON_CHECK_OK(this);
    }

    size_t get_lines_buf_size() const { return lines_buf_size_; }

    SLine*       get_lines_buf()       { return lines_buf_; }
    const SLine* get_lines_buf() const { return lines_buf_; }

    bool is_constant_buffer() const { return is_constant_buffer_; }

    virtual bool ok() const
    {
        if (ON_IS_POISON(lines_buf_size_) || !lines_buf_)
            return false;

        return true;
    }

    virtual void dump() const
    {
        ON_LOG_MSG("lines_buf_: [size: %d, first line: \"%.*s\", last line: \"%.*s\"]]",
                   lines_buf_size_,
                   ON_MAX_LINE_LEN, lines_buf_size_ ? lines_buf_[0].start_pos : "",
                   ON_MAX_LINE_LEN, lines_buf_size_ ? lines_buf_[lines_buf_size_-1].start_pos : "");
    }

private:
    size_t lines_buf_size_;
    SLine* lines_buf_;

    bool is_constant_buffer_;
};

class CFileTextBuffer: public CAbstractTextBuffer
{
private:
    CFileTextBuffer(size_t file_text_buf_size_set, char* file_text_buf_set):
        CAbstractTextBuffer(file_text_buf_size_set, file_text_buf_set),
        file_text_buf_size_(file_text_buf_size_set),
        file_text_buf_     (file_text_buf_set)
    {
        ON_CHECK_NOT_EQ(file_text_buf_, nullptr);
        ON_CHECK_GTEQ  (file_text_buf_size_, 1);

        ON_CHECK_OK(this);
    }

public:
    CFileTextBuffer(const CInputFile& input_file):
        CFileTextBuffer(input_file.get_text_buf_size(), input_file.allocate_text_buf()) {}

    CFileTextBuffer             (const CFileTextBuffer&) = delete;
    CFileTextBuffer& operator = (const CFileTextBuffer&) = delete;

    virtual ~CFileTextBuffer() override
    {
        ON_CHECK_OK(this);

        free(file_text_buf_);

        file_text_buf_ = nullptr;
        ON_POISON(file_text_buf_size_);
    }

    virtual bool ok() const override
    {
        return !ON_IS_POISON(file_text_buf_size_) && file_text_buf_ && CAbstractTextBuffer::ok();
    }

    virtual void dump() const override
    {
        ON_LOG_MSG("[file_text_buf_size_: %d]", file_text_buf_size_);
        CAbstractTextBuffer::dump();
    }

private:
    size_t file_text_buf_size_;
    char*  file_text_buf_;
};

class CMappedTextBuffer: public CAbstractTextBuffer
{
private:
    CMappedTextBuffer(size_t file_view_size_set, const char* file_view_set):
        CAbstractTextBuffer(file_view_size_set, file_view_set),
        file_view_size_(file_view_size_set),
        file_view_     (file_view_set)
    {
        ON_CHECK_NOT_EQ(file_view_, nullptr);
        ON_CHECK_GTEQ  (file_view_size_, 1);

        ON_CHECK_OK(this);
    }

public:
    CMappedTextBuffer(const CMapping& mapping_class):
        CMappedTextBuffer(mapping_class.get_file_length(),
                          (const char*)MapViewOfFile(mapping_class.get_map_handle(),
                                                     FILE_MAP_READ, 0, 0, mapping_class.get_file_length()))
    {
        ON_CHECK_NOT_EQ(mapping_class.get_map_mode(), ECMapMode::MAP_WRITEONLY_FILE);

        ON_CHECK_OK(this);
    }

    CMappedTextBuffer             (const CFileTextBuffer&) = delete;
    CMappedTextBuffer& operator = (const CFileTextBuffer&) = delete;

    virtual ~CMappedTextBuffer() override
    {
        ON_CHECK_OK(this);

        if (file_view_)
            UnmapViewOfFile(file_view_);

        file_view_ = nullptr;
        ON_POISON(file_view_size_);
    }

    virtual bool ok() const override
    {
        return !ON_IS_POISON(file_view_size_) && file_view_ && CAbstractTextBuffer::ok();
    }

    virtual void dump() const override
    {
        ON_LOG_MSG("[file_text_buf_size_: %d]", file_view_size_);
        CAbstractTextBuffer::dump();
    }

private:
    DWORD       file_view_size_;
    const char* file_view_;
};

}//namespace onegin_sort

#endif // ON_TEXT_BUFFER_H_INCLUDED
