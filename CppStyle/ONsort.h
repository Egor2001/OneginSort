#ifndef SORT_H_INCLUDED
#define SORT_H_INCLUDED

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <memory.h>
#include <cstring>
#include <assert.h>

#include "../ONdefines.h"
#include "Logging/ONlogging.h"
#include "ONtextBuffer.h"

namespace onegin_sort
{

enum class ECSortType
{
    DIRECT, REVERSE
};

class ISorter
{
public:
    ISorter() = default;
    virtual ~ISorter() = default;

    ISorter             (const ISorter&) = delete;
    ISorter& operator = (const ISorter&) = delete;

    virtual void sort_text_buf(CAbstractTextBuffer* text_buffer_ptr, ECSortType sort_order) = 0;
};

//<kostyl>
bool FilterComparator(const SLine& lhs_line, const SLine& rhs_line)//because I'm too lazy to write regex for it
{
    if (lhs_line.end_offset - lhs_line.begin_offset < 8) return false;//magic constants, wow
    if (rhs_line.end_offset - rhs_line.begin_offset < 8) return true;

    return lhs_line.start_pos < rhs_line.start_pos;
}
//</kostyl>

bool StrcmpDirectComparator(const SLine& lhs_line, const SLine& rhs_line)
{
    return (std::strcmp(lhs_line.start_pos + lhs_line.begin_offset,
                        rhs_line.start_pos + rhs_line.begin_offset) < 0);//it is slower, than simple comparator and I have no idea, why
}

bool SimpleDirectComparator(const SLine& lhs_line, const SLine& rhs_line)
{
    const char* lhs_it = lhs_line.start_pos, *lhs_end = lhs_it + lhs_line.end_offset;
    const char* rhs_it = rhs_line.start_pos, *rhs_end = rhs_it + rhs_line.end_offset;

    for (;;)
    {
        if (rhs_it == rhs_end) return false;
        if (lhs_it == lhs_end) return true;

        if (ispunct(*lhs_it) || isdigit(*lhs_it)) { lhs_it++; continue; }
        if (ispunct(*rhs_it) || isdigit(*rhs_it)) { rhs_it++; continue; }

        bool lhs_isspace = isspace(*lhs_it);
        bool rhs_isspace = isspace(*rhs_it);

        if (lhs_isspace && rhs_isspace) { lhs_it++; rhs_it++; continue; }
        if (lhs_isspace) return true;
        if (rhs_isspace) return false;

        if (*lhs_it < *rhs_it) return true;
        if (*lhs_it > *rhs_it) return false;

        lhs_it++; rhs_it++;
    }

    return false;
}

bool SimpleReverseComparator(const SLine& lhs_line, const SLine& rhs_line)
{
    const char* lhs_it = lhs_line.start_pos + lhs_line.end_offset;
    const char* rhs_it = rhs_line.start_pos + rhs_line.end_offset;

    for (;;)
    {
        if (rhs_it == rhs_line.start_pos) return false;
        if (lhs_it == lhs_line.start_pos) return true;

        if (ispunct(*lhs_it) || isdigit(*lhs_it)) { lhs_it--; continue; }
        if (ispunct(*rhs_it) || isdigit(*rhs_it)) { rhs_it--; continue; }

        bool lhs_isspace = isspace(*lhs_it);
        bool rhs_isspace = isspace(*rhs_it);

        if (lhs_isspace && rhs_isspace) { lhs_it--; rhs_it--; continue; }
        if (lhs_isspace) return true;
        if (rhs_isspace) return false;

        if (*lhs_it < *rhs_it) return true;
        if (*lhs_it > *rhs_it) return false;

        lhs_it--; rhs_it--;
    }

    return false;
}

class CSTLSorter: public ISorter
{
public:
    CSTLSorter() = default;
    virtual ~CSTLSorter() override = default;

    CSTLSorter             (const CSTLSorter&) = delete;
    CSTLSorter& operator = (const CSTLSorter&) = delete;

    virtual void sort_text_buf(CAbstractTextBuffer* text_buffer_ptr, ECSortType sort_order) override
    {
        ON_CHECK_OK(this);

        ON_CHECK_NOT_EQ(text_buffer_ptr, nullptr);

        SLine* line_buf = text_buffer_ptr->get_lines_buf();

        switch (sort_order)
        {
        case ECSortType::DIRECT:
            if (text_buffer_ptr->is_constant_buffer())
                std::sort(line_buf, line_buf + text_buffer_ptr->get_lines_buf_size(), SimpleDirectComparator);
            else
                std::sort(line_buf, line_buf + text_buffer_ptr->get_lines_buf_size(), StrcmpDirectComparator);

            break;

        case ECSortType::REVERSE:
            std::sort(line_buf, line_buf + text_buffer_ptr->get_lines_buf_size(), SimpleReverseComparator);
            break;

        default:
            ON_LOG_MSG("impossible default case reached");
            break;
        }

        ON_CHECK_OK(this);
    }

    bool ok() { return true; }
    void dump() { ON_LOG_MSG("[]"); }
};

int QuickSortStrcmpDirectComparator(const void* lhs_line_ptr, const void* rhs_line_ptr)
{
    return strcmp(((const SLine*)lhs_line_ptr)->start_pos + ((const SLine*)lhs_line_ptr)->begin_offset,
                  ((const SLine*)rhs_line_ptr)->start_pos + ((const SLine*)rhs_line_ptr)->begin_offset);
}

int QuickSortSimpleDirectComparator(const void* lhs_line_ptr, const void* rhs_line_ptr)
{
    const char* lhs_it = ((const SLine*)lhs_line_ptr)->start_pos;
    const char* rhs_it = ((const SLine*)rhs_line_ptr)->start_pos;

    const char* lhs_end = lhs_it + ((const SLine*)lhs_line_ptr)->end_offset;
    const char* rhs_end = rhs_it + ((const SLine*)rhs_line_ptr)->end_offset;

    for (;;)
    {
        if (rhs_it == rhs_end) return 1;
        if (lhs_it == lhs_end) return -1;

        if (ispunct(*lhs_it) || isdigit(*lhs_it)) { lhs_it++; continue; }
        if (ispunct(*rhs_it) || isdigit(*rhs_it)) { rhs_it++; continue; }

        bool lhs_isspace = isspace(*lhs_it);
        bool rhs_isspace = isspace(*rhs_it);

        if (lhs_isspace && rhs_isspace) { lhs_it++; rhs_it++; continue; }
        if (lhs_isspace) return -1;
        if (rhs_isspace) return 1;

        if (*lhs_it < *rhs_it) return -1;
        if (*lhs_it > *rhs_it) return 1;

        lhs_it++; rhs_it++;
    }

    return 1;
}

int QuickSortSimpleReverseComparator(const void* lhs_line_ptr, const void* rhs_line_ptr)
{
    const char* lhs_it = ((const SLine*)lhs_line_ptr)->start_pos + ((const SLine*)lhs_line_ptr)->end_offset;
    const char* rhs_it = ((const SLine*)rhs_line_ptr)->start_pos + ((const SLine*)rhs_line_ptr)->end_offset;

    const char* lhs_start = ((const SLine*)lhs_line_ptr)->start_pos;
    const char* rhs_start = ((const SLine*)rhs_line_ptr)->start_pos;

    for (;;)
    {
        if (rhs_it == rhs_start) return 1;
        if (lhs_it == lhs_start) return -1;

        if (ispunct(*lhs_it) || isdigit(*lhs_it)) { lhs_it--; continue; }
        if (ispunct(*rhs_it) || isdigit(*rhs_it)) { rhs_it--; continue; }

        bool lhs_isspace = isspace(*lhs_it);
        bool rhs_isspace = isspace(*rhs_it);

        if (lhs_isspace && rhs_isspace) { lhs_it--; rhs_it--; continue; }
        if (lhs_isspace) return -1;
        if (rhs_isspace) return 1;

        if (*lhs_it < *rhs_it) return -1;
        if (*lhs_it > *rhs_it) return 1;

        lhs_it--; rhs_it--;
    }

    return 1;
}

class CQuickSorter: public ISorter
{
public:
    CQuickSorter() = default;
    virtual ~CQuickSorter() override = default;

    CQuickSorter             (const CQuickSorter&) = delete;
    CQuickSorter& operator = (const CQuickSorter&) = delete;

    virtual void sort_text_buf(CAbstractTextBuffer* text_buffer_ptr, ECSortType sort_order) override
    {
        ON_CHECK_NOT_EQ(text_buffer_ptr, nullptr);

        SLine* line_buf = text_buffer_ptr->get_lines_buf();

        switch (sort_order)
        {
        case ECSortType::DIRECT:
            if (text_buffer_ptr->is_constant_buffer())
                qsort(line_buf, text_buffer_ptr->get_lines_buf_size(), sizeof(SLine),
                      &QuickSortSimpleDirectComparator);
            else
                qsort(line_buf, text_buffer_ptr->get_lines_buf_size(), sizeof(SLine),
                      &QuickSortStrcmpDirectComparator);

            break;

        case ECSortType::REVERSE:
            qsort(line_buf, text_buffer_ptr->get_lines_buf_size(), sizeof(SLine),
                  &QuickSortSimpleReverseComparator);
            break;

        default:
            ON_LOG_MSG("impossible default case reached");
            break;
        }
    }

    bool ok() { return true; }
    void dump() { ON_LOG_MSG("[]"); }
};

class CBitonicSorter: public ISorter
{
public:
    CBitonicSorter() = default;
    virtual ~CBitonicSorter() override = default;

    CBitonicSorter             (const CBitonicSorter&) = delete;
    CBitonicSorter& operator = (const CBitonicSorter&) = delete;

    virtual void sort_text_buf(CAbstractTextBuffer* text_buffer_ptr, ECSortType sort_order) override
    {
        ON_CHECK_NOT_EQ(text_buffer_ptr, nullptr);

        SLine* line_buf = text_buffer_ptr->get_lines_buf();

        switch (sort_order)
        {
        case ECSortType::DIRECT:
            if (text_buffer_ptr->is_constant_buffer())
                bitonic_sort(line_buf, text_buffer_ptr->get_lines_buf_size(),
                             SimpleDirectComparator);
            else
                bitonic_sort(line_buf, text_buffer_ptr->get_lines_buf_size(),
                             StrcmpDirectComparator);

            break;

        case ECSortType::REVERSE:
            bitonic_sort(line_buf, text_buffer_ptr->get_lines_buf_size(),
                         SimpleReverseComparator);
            break;

        default:
            ON_LOG_MSG("impossible default case reached");
            break;
        }
    }

    template<typename LineType, typename Comp>
    void bitonic_sort(LineType* line_buf, size_t line_buf_size, Comp comparator)
    {
        size_t ceil_log = 0;

        for (size_t i = 1; i < line_buf_size; i <<= 1)
            ceil_log++;

        for (size_t step_idx = 0; step_idx < ceil_log; step_idx++)
            for (size_t column_idx = 0; column_idx <= step_idx; column_idx++)
                bitonic_sort_step(line_buf, line_buf_size, step_idx, column_idx, comparator);
    }

    template<typename LineType, typename Comp>
    void bitonic_sort_step(LineType* line_buf, size_t line_buf_size,
                           size_t step_idx, size_t column_idx, Comp comparator)
    {
        ON_CHECK_GTEQ(step_idx, column_idx);

        size_t chunk_size = 1ul << (step_idx - column_idx);
        size_t order_mask = 2ul << step_idx;

        for (size_t i = 0; i < line_buf_size; i++)
        {
            if (i & chunk_size)
                continue;

            bool is_inverse_order = i & order_mask;
            size_t pair_idx = i | chunk_size;

            if (pair_idx < line_buf_size &&
                comparator(line_buf[i], line_buf[pair_idx]) == is_inverse_order)
                std::swap(line_buf[i], line_buf[pair_idx]);
        }
    }

    bool ok() { return true; }
    void dump() { ON_LOG_MSG("[]"); }
};

}//namespace onegin_sort

#endif//SORT_H_INCLUDED
