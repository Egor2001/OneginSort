#ifndef SORT_H_INCLUDED
#define SORT_H_INCLUDED

#include <cstdint>
#include <cstdio>
#include <memory.h>
#include <cstring>
#include <assert.h>

#include "defines.h"

namespace onegin_sort
{

struct SAnyLine
{
    struct DirectComparator;
    struct ReverseComparator;
    struct ReversePackComparator;

    SAnyLine(): line_arr{}, line_len{} {}
    SAnyLine(const char line_set[ON_MAX_LINE_LEN], size_t line_len_set): line_arr{}, line_len(line_len_set)
    {
        assert(line_set);
        assert(line_len < ON_MAX_LINE_LEN-2);

        memcpy(line_arr, line_set, line_len);
        line_arr[line_len+1] = '\0';
    }

    char line_arr[ON_MAX_LINE_LEN];
    size_t line_len;
};

struct SAnyLine::DirectComparator
{
    bool operator () (const SAnyLine& lhs_line, const SAnyLine& rhs_line)
    {
        const char* lhs_it = lhs_line.line_arr;
        const char* rhs_it = rhs_line.line_arr;

        for (;;)
        {
            if (*lhs_it < *rhs_it) return true;
            if (*lhs_it > *rhs_it) return false;

            lhs_it++; rhs_it++;

            if (!(*rhs_it)) return false;
            if (!(*lhs_it)) return true;
        }

        return false;
    }
};

struct SAnyLine::ReverseComparator
{
    bool operator () (const SAnyLine& lhs_line, const SAnyLine& rhs_line)
    {
        const char* lhs_it = lhs_line.line_arr + lhs_line.line_len - 1;
        const char* rhs_it = rhs_line.line_arr + rhs_line.line_len - 1;

        for (;;)
        {
            if (*lhs_it < *rhs_it) return true;
            if (*lhs_it > *rhs_it) return false;

            lhs_it--; rhs_it--;

            if (!(*rhs_it)) return false;
            if (!(*lhs_it)) return true;
        }

        return false;
    }
};

struct SAnyLine::ReversePackComparator
{
    bool operator () (const SAnyLine& lhs_line, const SAnyLine& rhs_line)
    {

        const char* lhs_it = lhs_line.line_arr + lhs_line.line_len - 1;
        const char* rhs_it = rhs_line.line_arr + rhs_line.line_len - 1;

        if (lhs_line.line_len >= sizeof(uint32_t) && rhs_line.line_len >= sizeof(uint32_t))
        {
            for (lhs_it = lhs_line.line_arr + lhs_line.line_len - sizeof(uint32_t),
                 rhs_it = rhs_line.line_arr + rhs_line.line_len - sizeof(uint32_t);
                 lhs_it >= lhs_line.line_arr &&
                 rhs_it >= rhs_line.line_arr;
                 lhs_it -= sizeof(uint32_t), rhs_it -= sizeof(uint32_t))
            {
                if (*reinterpret_cast<const uint32_t*>(lhs_it) < *reinterpret_cast<const uint32_t*>(rhs_it)) return true;
                if (*reinterpret_cast<const uint32_t*>(lhs_it) > *reinterpret_cast<const uint32_t*>(rhs_it)) return false;
            }
        }

        for (;;)
        {
            if (*lhs_it < *rhs_it) return true;
            if (*lhs_it > *rhs_it) return false;

            lhs_it--; rhs_it--;

            if (!(*rhs_it)) return false;
            if (!(*lhs_it)) return true;
        }

        return false;
    }
};

}//namespace onegin_sort

#endif//SORT_H_INCLUDED
