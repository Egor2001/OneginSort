#ifndef ON_MAPPING_H_INCLUDED
#define ON_MAPPING_H_INCLUDED

#include "windows.h"

#include "../ONdefines.h"

namespace onegin_sort {

enum ECMapMode
{
    MAP_INITIAL_FILE, MAP_RESULTING_FILE
};

class CMapping
{
public:
    CMapping() = default;
    CMapping(ECMapMode map_mode_set, const char* file_path, DWORD file_length_set = 0):
        map_mode_(map_mode_set), map_handle_{NULL}, file_handle_{NULL}, file_length_(file_length_set)
    {
        switch (map_mode_)
        {
        case MAP_INITIAL_FILE:
            file_handle_ = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

            break;

        case MAP_RESULTING_FILE:
            file_handle_ = CreateFile(file_path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

            break;

        default: assert(0);
        }

        assert(file_handle_);

        if (!file_length_)
        {
            file_length_ = GetFileSize(file_handle_, &file_length_);
            assert(file_length_);
        }

        switch (map_mode_)
        {
        case MAP_INITIAL_FILE:
            map_handle_ = CreateFileMapping(file_handle_, NULL, PAGE_READONLY, 0, file_length_, ON_INITIAL_MAPPING_NAME);

            break;

        case MAP_RESULTING_FILE:
            map_handle_ = CreateFileMapping(file_handle_, NULL, PAGE_READWRITE, 0, file_length_, ON_RESULT_MAPPING_NAME);

            break;

        default: assert(0);
        }

        assert(map_handle_);
    }

    ~CMapping()
    {
        if (map_handle_)
        {
            CloseHandle(map_handle_);
            map_handle_ = NULL;
        }

        if (file_handle_)
        {
            CloseHandle(file_handle_);
            file_handle_ = NULL;//mb -1 is better
        }
    }

    ECMapMode    get_map_mode  () const { return map_mode_; }
    const HANDLE get_map_handle() const { return map_handle_; }

    DWORD get_file_length() const { return file_length_; }

private:
    ECMapMode map_mode_;
    HANDLE    map_handle_;

    HANDLE file_handle_;
    DWORD  file_length_;

private:
    static DWORD get_granularity_()
    {
        SYSTEM_INFO sys_info = {};

        GetSystemInfo(&sys_info);

        return sys_info.dwAllocationGranularity;
    }

public:
    static DWORD granularity;
};

DWORD CMapping::granularity = CMapping::get_granularity_();

class CFileView
{
public:
    CFileView() = default;
    CFileView(const CMapping& mapping_class, DWORD absolute_offset, DWORD length):
        offset_(absolute_offset%CMapping::granularity),
        length_(length+offset_), view_ptr_{NULL}
    {
        DWORD view_start = absolute_offset - offset_;

        switch (mapping_class.get_map_mode())
        {
        case MAP_INITIAL_FILE:
            view_ptr_ = (char*)MapViewOfFile(mapping_class.get_map_handle(), FILE_MAP_READ, 0, view_start, length_);

            break;

        case MAP_RESULTING_FILE:
            view_ptr_ = (char*)MapViewOfFile(mapping_class.get_map_handle(), FILE_MAP_WRITE, 0, view_start, length_);

            break;

        default: assert(0);
        }

        if (!length_)
            length_ = mapping_class.get_file_length();

        assert(view_ptr_);
    }

    ~CFileView()
    {
        if (view_ptr_)
            UnmapViewOfFile(view_ptr_);

        view_ptr_ = NULL;
        length_ = offset_ = 0;
    }

    DWORD       get_offset() const { return offset_; }
    DWORD       get_length() const { return length_; }
    const char* get_view  () const { return view_ptr_; }
          char* get_view  ()       { return view_ptr_; }

private:
    DWORD offset_;
    DWORD length_;
    char* view_ptr_;
};

}//namespace onegin_sort

#endif //ON_MAPPING_H_INCLUDED
