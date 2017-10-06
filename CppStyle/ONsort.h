#ifndef SORT_H_INCLUDED
#define SORT_H_INCLUDED

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <memory.h>
#include <cstring>
#include <assert.h>

#include "../ONdefines.h"
#include "ONtextBuffer.h"

namespace onegin_sort
{

//<kostyl>
struct SFilterComparator//because I'm too lazy to write regex for it
{
    bool operator () (const SLine& lhs_line, const SLine& rhs_line)
    {
        if (lhs_line.end_offset - lhs_line.begin_offset < 8) return false;//magic constants, wow
        if (rhs_line.end_offset - rhs_line.begin_offset < 8) return true;

        return lhs_line.start_pos < rhs_line.start_pos;
    }
};
//</kostyl>
struct SStrcmpDirectComparator
{
    bool operator () (const SLine& lhs_line, const SLine& rhs_line)
    {
        return (std::strcmp(lhs_line.start_pos + lhs_line.begin_offset,
                            rhs_line.start_pos + rhs_line.begin_offset) < 0);//it is slower, than simple comparator and I have no idea, why
    }
};

struct SSimpleDirectComparator
{
    bool operator () (const SLine& lhs_line, const SLine& rhs_line)
    {
        const char* lhs_it = lhs_line.start_pos, *lhs_end = lhs_it + lhs_line.end_offset;
        const char* rhs_it = rhs_line.start_pos, *rhs_end = rhs_it + rhs_line.end_offset;

        for (;;)
        {
            if (rhs_it == rhs_end) return false;
            if (lhs_it == lhs_end) return true;

//            int isspace_diff = !!isspace(*lhs_it) - !!isspace(*rhs_it);
//
//            if (isspace_diff == 0) { lhs_it++; rhs_it++; continue; }
//            if (isspace_diff > 0) return true;
//            if (isspace_diff < 0) return false;

            if (ispunct(*lhs_it) || isdigit(*lhs_it)) { lhs_it++; continue; }
            if (ispunct(*rhs_it) || isdigit(*rhs_it)) { rhs_it++; continue; }

            if (*lhs_it < *rhs_it) return true;
            if (*lhs_it > *rhs_it) return false;

            lhs_it++; rhs_it++;
        }

        return false;
    }
};

struct SSimpleReverseComparator
{
    bool operator () (const SLine& lhs_line, const SLine& rhs_line)
    {
        const char* lhs_it = lhs_line.start_pos + lhs_line.end_offset;
        const char* rhs_it = rhs_line.start_pos + rhs_line.end_offset;

        for (;;)
        {
            if (rhs_it == rhs_line.start_pos) return false;
            if (lhs_it == lhs_line.start_pos) return true;

//            int isspace_diff = !!isspace(*lhs_it) - !!isspace(*rhs_it);
//
//            if (isspace_diff == 0) { lhs_it--; rhs_it--; continue; }
//            if (isspace_diff > 0) return true;
//            if (isspace_diff < 0) return false;

            if (ispunct(*lhs_it) || isdigit(*lhs_it)) { lhs_it--; continue; }
            if (ispunct(*rhs_it) || isdigit(*rhs_it)) { rhs_it--; continue; }

            if (*lhs_it < *rhs_it) return true;
            if (*lhs_it > *rhs_it) return false;

            lhs_it--; rhs_it--;
        }

        return false;
    }
};

class CSTLSorter
{
public:
    enum class ESortType
    {
        DIRECT, REVERSE
    };

public:
    CSTLSorter() = default;
    ~CSTLSorter() = default;

    CSTLSorter             (const CSTLSorter&) = delete;
    CSTLSorter& operator = (const CSTLSorter&) = delete;

    void operator () (CAbstractTextBuffer* text_buffer, ESortType sort_order)
    {
        ON_CHECK_NOT_EQ(text_buffer, nullptr);

        SLine* line_buf = text_buffer->get_lines_buf();

        switch (sort_order)
        {
        case ESortType::DIRECT:
            if (text_buffer->is_constant_buffer())
                std::sort(line_buf, line_buf + text_buffer->get_lines_buf_size(), SSimpleDirectComparator());
            else
                std::sort(line_buf, line_buf + text_buffer->get_lines_buf_size(), SStrcmpDirectComparator());

            break;
        case ESortType::REVERSE:
            std::sort(line_buf, line_buf + text_buffer->get_lines_buf_size(), SSimpleReverseComparator());
            break;
        default:
            ON_LOG_MSG("impossible default case reached");
            break;
        }
    }

    bool ok() { return true; }
    void dump() { ON_LOG_MSG("[]"); }
};

}//namespace onegin_sort

#endif//SORT_H_INCLUDED
