#define ON_UNIX_SEPARATORS

#include "locale.h"
#include <vector>
#include <algorithm>

#include "CppStyle/ONinputFile.h"
#include "CppStyle/ONoutputFile.h"
#include "CppStyle/ONmappedSort.h"
#include "CppStyle/ONmapping.h"
#include "CppStyle/ONsort.h"

#define ON_PROFILE(operation) \
([]() -> unsigned long \
{ \
    unsigned long start_tick_count = GetTickCount(); \
    (operation); \
    return GetTickCount() - start_tick_count; \
}())

using namespace onegin_sort;

void SortMappedTest(CSTLSorter::ESortType sort_order);
void SortSimpleTest(CSTLSorter::ESortType sort_order);

int main()
{
    setlocale(LC_ALL, "");

    unsigned long simple_sort_ms_elapsed = 0u, mapped_sort_ms_elapsed = 0u;

    simple_sort_ms_elapsed = ON_PROFILE(SortSimpleTest(CSTLSorter::ESortType::REVERSE));
    mapped_sort_ms_elapsed = ON_PROFILE(SortMappedTest(CSTLSorter::ESortType::REVERSE));

    printf("REVERSE: [simple: %lu_ms, mapped: %lu_ms]\n", simple_sort_ms_elapsed, mapped_sort_ms_elapsed);

    simple_sort_ms_elapsed = ON_PROFILE(SortSimpleTest(CSTLSorter::ESortType::DIRECT));
    mapped_sort_ms_elapsed = ON_PROFILE(SortMappedTest(CSTLSorter::ESortType::DIRECT));

    printf("DIRECT: [simple: %lu_ms, mapped: %lu_ms]\n", simple_sort_ms_elapsed, mapped_sort_ms_elapsed);

    return 0;
}

void SortMappedTest(CSTLSorter::ESortType sort_order)
{
    const char* out_file_name = (sort_order == CSTLSorter::ESortType::DIRECT ?
                                 ON_DIRECT_FILE_OUT_PATH : ON_REVERSE_FILE_OUT_PATH);

    CMapping  in_file_mapping = CMapping(ECMapMode::MAP_INITIAL_FILE,   ON_FILE_IN_PATH);
    CMapping out_file_mapping = CMapping(ECMapMode::MAP_RESULTING_FILE, out_file_name,
                                         in_file_mapping.get_file_length());

    CMappedTextBuffer text_buffer( in_file_mapping);
    CMappedOutput     output     (out_file_mapping);

    text_buffer.calculate_offsets();

    CSTLSorter sorter;
    sorter(&text_buffer, sort_order);

    output.write_text_buffer(&text_buffer);
}

void SortSimpleTest(CSTLSorter::ESortType sort_order)
{
    const char* out_file_name = (sort_order == CSTLSorter::ESortType::DIRECT ?
                                 ON_DIRECT_FILE_OUT_PATH : ON_REVERSE_FILE_OUT_PATH);

    CInputFile input_file(ON_FILE_IN_PATH);

    CFileTextBuffer text_buffer(input_file);
    CFileOutput     output_file(out_file_name);

    text_buffer.calculate_offsets();

    CSTLSorter sorter;
    sorter(&text_buffer, sort_order);

    text_buffer.concat_lines();

    output_file.write_text_buffer(&text_buffer);
}
