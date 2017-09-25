#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <io.h>
#include <stdio.h>
#include <assert.h>
#include <vector>

#include "defines.h"
#include "io.h"
#include "share.h"
#include "fcntl.h"

namespace onegin_sort
{

template<class LineType>
LineType on_read_line(int file_descr)
{
    char buffer[ON_MAX_LINE_LEN];
    _read(file_descr, buffer, ON_MAX_LINE_LEN);

    size_t line_len = ON_MAX_LINE_LEN-1;

    for (char* it = buffer; it < buffer + ON_MAX_LINE_LEN - 1; it++)
    {
        if (*it == '\n')
        {
            line_len = it+1 - buffer;
            break;
        }
        else if (*it == '\r')
        {
            line_len = it+2 - buffer;
            break;
        }
    }

    _lseek(file_descr, line_len+1 - ON_MAX_LINE_LEN, SEEK_CUR);

    return LineType(buffer, line_len);
}

template<class LineType>
void on_write_line(int file_descr, const LineType& line)
{
    _write(file_descr, line.line_arr, line.line_len);
}

}//namespace onegin_sort

#endif//FILE_H_INCLUDED
