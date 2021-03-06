#define ON_UNIX_SEPARATORS

#include "locale.h"
#include <vector>
#include <algorithm>

#include "CppStyle/ONinputFile.h"
#include "CppStyle/ONoutputFile.h"
#include "CppStyle/ONmapping.h"
#include "CppStyle/ONsort.h"

#define ON_PROFILE(operation) \
([&]() -> unsigned long \
{ \
    unsigned long start_tick_count = GetTickCount(); \
    (operation); \
    return GetTickCount() - start_tick_count; \
}())

using namespace onegin_sort;

void SortMappedTest(ECSortType sort_order, ISorter* sorter_ptr);
void SortSimpleTest(ECSortType sort_order, ISorter* sorter_ptr);

int main()
{
    setlocale(LC_ALL, "");

    unsigned long simple_sort_ms_elapsed = 0u, mapped_sort_ms_elapsed = 0u;

    CBitonicSorter stl_sorter;
    CQuickSorter   quick_sorter;

    simple_sort_ms_elapsed = ON_PROFILE(SortSimpleTest(ECSortType::REVERSE, &quick_sorter));
    mapped_sort_ms_elapsed = ON_PROFILE(SortMappedTest(ECSortType::REVERSE, &quick_sorter));

    printf("REVERSE: [simple: %lu_ms, mapped: %lu_ms]\n", simple_sort_ms_elapsed, mapped_sort_ms_elapsed);

    simple_sort_ms_elapsed = ON_PROFILE(SortSimpleTest(ECSortType::DIRECT, &quick_sorter));
    mapped_sort_ms_elapsed = ON_PROFILE(SortMappedTest(ECSortType::DIRECT, &quick_sorter));

    printf("DIRECT: [simple: %lu_ms, mapped: %lu_ms]\n", simple_sort_ms_elapsed, mapped_sort_ms_elapsed);

    return 0;
}

void SortMappedTest(ECSortType sort_order, ISorter* sorter_ptr)
{
    ON_CHECK_NOT_EQ(sorter_ptr, nullptr);

    const char* out_file_name = (sort_order == ECSortType::DIRECT ?
                                 ON_DIRECT_FILE_OUT_PATH : ON_REVERSE_FILE_OUT_PATH);

    CMapping  in_file_mapping = CMapping(ECMapMode::MAP_READONLY_FILE,   ON_FILE_IN_PATH);
    CMapping out_file_mapping = CMapping(ECMapMode::MAP_WRITEONLY_FILE, out_file_name,
                                         in_file_mapping.get_file_length());

    CMappedTextBuffer text_buffer( in_file_mapping);
    CMappedOutput     output     (out_file_mapping);

    text_buffer.calculate_offsets();

    sorter_ptr->sort_text_buf(&text_buffer, sort_order);

    output.write_text_buffer(&text_buffer);
}

void SortSimpleTest(ECSortType sort_order, ISorter* sorter_ptr)
{
    ON_CHECK_NOT_EQ(sorter_ptr, nullptr);

    const char* out_file_name = (sort_order == ECSortType::DIRECT ?
                                 ON_DIRECT_FILE_OUT_PATH : ON_REVERSE_FILE_OUT_PATH);

    CInputFile input_file(ON_FILE_IN_PATH);

    CFileTextBuffer text_buffer(input_file);
    CFileOutput     output_file(out_file_name);

    text_buffer.calculate_offsets();

    sorter_ptr->sort_text_buf(&text_buffer, sort_order);

    text_buffer.concat_lines();

    output_file.write_text_buffer(&text_buffer);
}
