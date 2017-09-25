#ifndef ON_MAPPED_SORT_INCLUDED
#define ON_MAPPED_SORT_INCLUDED

#include "ONmapping.h"
#include "defines.h"

#include <string>
#include <vector>
#include <memory>

namespace onegin_sort {

struct SMappedLine
{
    struct DirectComparator;
    struct ReverseComparator;

    SMappedLine(): index{-1}, start_pos{nullptr}, length{0} {}
    ~SMappedLine() = default;

    size_t index;

    char* start_pos;
    size_t length;
};

struct SMappedLine::DirectComparator
{
    bool operator () (const SMappedLine& lhs, const SMappedLine& rhs)
    {
        char* lhs_it = lhs.start_pos, *lhs_end = lhs_it + lhs.length;
        char* rhs_it = rhs.start_pos, *rhs_end = rhs_it + rhs.length;

        for (;;)
        {
            if (*lhs_it < *rhs_it) return true;
            if (*lhs_it > *rhs_it) return false;

            if (++lhs_it == lhs_end) return true;
            if (++rhs_it == rhs_end) return false;
        }

        return false;
    }
};

struct SMappedLine::ReverseComparator
{
    bool operator () (const SMappedLine& lhs, const SMappedLine& rhs)
    {
        char* lhs_end = lhs.start_pos, *lhs_it = lhs_end + lhs.length-1;
        char* rhs_end = rhs.start_pos, *rhs_it = rhs_end + rhs.length-1;

        for (;;)
        {
            if (*lhs_it < *rhs_it) return true;
            if (*lhs_it > *rhs_it) return false;

            if (--lhs_it < lhs_end) return true;
            if (--rhs_it < rhs_end) return false;
        }

        return false;
    }
};

struct STokenized
{
    std::vector<char*> delimiters;
    std::vector<SMappedLine> lines;
};

class CParser
{
public:
    explicit CParser(CFileView& file_view): view_end_{}, cur_pos_{}
    {
        cur_pos_ = file_view.get_view();
        view_end_ = cur_pos_ + file_view.get_length() + 1;
    }

    ~CParser()
    {
        cur_pos_  = nullptr;
        view_end_ = nullptr;
    }

    SMappedLine parse_line()
    {
        SMappedLine result = SMappedLine();

        while (!std::isalpha(*cur_pos_))
        {
            if (*cur_pos_ == '\n' || *cur_pos_ == '\r')
            {
                cur_pos_++;
                return result;
            }

            cur_pos_++;

            if (cur_pos_ == view_end_)
                return result;
        }

        result.start_pos = cur_pos_;

#ifdef ON_UNIX_SEPARATORS
        while (!(*cur_pos_ == '\n'))
        {
            cur_pos_++;

            if (cur_pos_ == view_end_)
                return result;
        }
#else
        while (!(*cur_pos_ == '\r'))
        {
            cur_pos_++;

            if (cur_pos_ == view_end_)
                return result;
        }
        cur_pos_++;
#endif

        char* temp_pos = cur_pos_;
        cur_pos_++;

        while(!std::isalpha(*temp_pos))//mb isalnum is better to use
            temp_pos--;

        result.length = temp_pos - result.start_pos + 1;

        return result;
    }

    STokenized operator () ()
    {
        STokenized result = {};
        result.delimiters.push_back(cur_pos_);

        SMappedLine cur_line = SMappedLine();

        while (cur_pos_ < view_end_)
        {
            cur_line = parse_line();
            result.delimiters.push_back(cur_pos_);

            if (cur_line.length > 0)
            {
                cur_line.index = result.delimiters.size() - 2;
                result.lines.push_back(cur_line);
            }
        }

        return result;
    }

private:
    char* view_end_;
    char* cur_pos_;
};

class CPrinter
{
public:
    explicit CPrinter(CFileView& file_view):
        cur_pos_(file_view.get_view()) {}

    ~CPrinter()
    {
        cur_pos_ = nullptr;
    }

    void operator () (const STokenized& tokenized_data)
    {
        //printf ("reached");

        for (const auto& line : tokenized_data.lines)
        {
            size_t full_line_len = tokenized_data.delimiters[line.index+1] - tokenized_data.delimiters[line.index];

            std::memcpy(cur_pos_, tokenized_data.delimiters[line.index], full_line_len);

            cur_pos_ += full_line_len;
        }
    }

private:
    char* cur_pos_;
};

class CSorter
{
public:
    CSorter(): buckets_num_{2+'z'-'a'} {}

    ~CSorter()
    {}

    size_t bucket_index(char symbol)
    {
        return (isalpha(symbol) ? (tolower(symbol) - 'a' + 1) : 0);
    }

    void abc_sort(STokenized* tokenized_file)
    {
        size_t symb_tracker_[ON_MAX_LINE_LEN][buckets_num_];
        std::vector<int> word_tracker_(tokenized_file->lines.size());
    }

private:
    size_t buckets_num_;
};

}//namespace onegin_sort

#endif // ON_MAPPED_SORT_INCLUDED
