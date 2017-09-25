//#define ON_UNIX_SEPARATORS

#include "locale.h"
#include <vector>
#include <algorithm>

#include "ONfile.h"
#include "ONsort.h"
#include "ONmapping.h"
#include "ONmappedSort.h"

#define ON_DIRECT_COMPARE true
#define ON_REVERSE_COMPARE false

using namespace onegin_sort;

void SortMappedTest(bool compare_type = ON_DIRECT_COMPARE);
void SortSimpleTest(bool compare_type = ON_DIRECT_COMPARE);

int main()
{
    setlocale(LC_ALL, "Russian");

    unsigned long simple_sort_ms_elapsed = 0u, mapped_sort_ms_elapsed = 0u;;
    unsigned long start_tick_count = 0u;

    start_tick_count = GetTickCount();
    SortSimpleTest(ON_DIRECT_COMPARE);
    simple_sort_ms_elapsed = GetTickCount() - start_tick_count;

    start_tick_count = GetTickCount();
    SortMappedTest(ON_DIRECT_COMPARE);
    mapped_sort_ms_elapsed = GetTickCount() - start_tick_count;

    printf("DIRECT: [simple: %d_ms, mapped: %d_ms]\n", simple_sort_ms_elapsed, mapped_sort_ms_elapsed);

    start_tick_count = GetTickCount();
    SortSimpleTest(ON_REVERSE_COMPARE);
    simple_sort_ms_elapsed = GetTickCount() - start_tick_count;

    start_tick_count = GetTickCount();
    SortMappedTest(ON_REVERSE_COMPARE);
    mapped_sort_ms_elapsed = GetTickCount() - start_tick_count;

    printf("REVERSE: [simple: %d_ms, mapped: %d_ms]\n", simple_sort_ms_elapsed, mapped_sort_ms_elapsed);

    return 0;
}

void SortMappedTest(bool compare_type/* = ON_DIRECT_COMPARE */)
{
    const char* out_file_name = (compare_type == ON_DIRECT_COMPARE ?
                                 ON_DIRECT_FILE_OUT_PATH : ON_REVERSE_FILE_OUT_PATH);

    CMapping  in_file_mapping = CMapping(ECMapMode::MAP_INITIAL_FILE,   ON_FILE_IN_PATH);
    CMapping out_file_mapping = CMapping(ECMapMode::MAP_RESULTING_FILE, out_file_name,
                                         in_file_mapping.get_file_length());

    CFileView  in_file_view = CFileView( in_file_mapping, 0, 0);
    CFileView out_file_view = CFileView(out_file_mapping, 0, 0);

    STokenized out_data = CParser(in_file_view)();

    if (compare_type == ON_DIRECT_COMPARE)
        std::sort(out_data.lines.begin(), out_data.lines.end(), SMappedLine::DirectComparator{});
    else
        std::sort(out_data.lines.begin(), out_data.lines.end(), SMappedLine::ReverseComparator{});

    CPrinter printer = CPrinter(out_file_view);

    printer(out_data);

}

void SortSimpleTest(bool compare_type/* = ON_DIRECT_COMPARE */)
{
    const char* out_file_name = (compare_type == ON_DIRECT_COMPARE ?
                                 ON_DIRECT_FILE_OUT_PATH : ON_REVERSE_FILE_OUT_PATH);

    int input_file  = _sopen(ON_FILE_IN_PATH, _O_TEXT | _O_SEQUENTIAL | _O_RDONLY, _SH_DENYNO);
    int output_file = _sopen(out_file_name,   _O_TEXT | _O_SEQUENTIAL | _O_WRONLY, _SH_DENYNO);

    std::vector<SAnyLine> lines_vec;

    uint64_t file_len = _filelengthi64(input_file);

    while (file_len - _telli64(input_file) > ON_MAX_LINE_LEN)
        lines_vec.push_back(on_read_line<SAnyLine>(input_file));

    lines_vec.push_back(on_read_line<SAnyLine>(input_file));

    if (compare_type == ON_DIRECT_COMPARE)
        std::sort(lines_vec.begin(), lines_vec.end(), SAnyLine::DirectComparator{});
    else
        std::sort(lines_vec.begin(), lines_vec.end(), SAnyLine::ReverseComparator{});

    for (const auto& line : lines_vec)
        on_write_line<SAnyLine>(output_file, line);

    _close(input_file);
    _close(output_file);

    input_file = output_file = -1;

}
