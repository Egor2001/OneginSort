#ifndef ON_LOGGING_H_INCLUDED
#define ON_LOGGING_H_INCLUDED

#include <cstdio>
#include <typeinfo>
#include <assert.h>

#define ON_LOG_MSG(format, ...) //if (CLogger->get_global_logger())

#define ON_POISON(var) (var = static_cast<decltype(var)>(-1))
#define ON_IS_POISON(var) !(var + 1)
#define ON_CHECK_POISON(var) if (var + 1 == 0) ON_LOG_MSG("poisoned variable: "#var)

#define ON_CHECK_EQ(lhs, rhs) assert(lhs == rhs)
#define ON_CHECK_NOT_EQ(lhs, rhs) assert(lhs != rhs)
#define ON_CHECK_LT(lhs, rhs) assert(lhs < rhs)
#define ON_CHECK_GT(lhs, rhs) assert(lhs > rhs)
#define ON_CHECK_LTEQ(lhs, rhs) assert(lhs <= rhs)
#define ON_CHECK_GTEQ(lhs, rhs) assert(lhs >= rhs)

#define ON_CHECK_SYSCALL(result) (result)

#define ON_CHECK_OK(obj_ptr) \
    if (!obj_ptr->ok()) { obj_ptr->dump(); assert(!typeid(*obj_ptr).name()); }

namespace onegin_sort {

class CLogger
{
private:
    static CLogger* global_logger_ptr_;//TODO: shared_ptr

public:
    static CLogger* set_global_logger(CLogger* global_logger_ptr_set = nullptr)
    {
        CLogger* old_global_logger_ptr = global_logger_ptr_;

        global_logger_ptr_ = global_logger_ptr_set;

        return old_global_logger_ptr;
    }

    static CLogger* get_global_logger() { return global_logger_ptr_; }

public:
    CLogger(const char* log_file_name = nullptr): log_file_{nullptr}
    {
        if (!log_file_name) return;
        if (!(log_file_ = fopen(log_file_name, "a"))) return;

        fprintf(log_file_, "LOGGING STARTED [file: \"%s\", mode: TRACE]" "\n\n", log_file_name);
    }

    CLogger             (const CLogger&) = delete;
    CLogger& operator = (const CLogger&) = delete;

    ~CLogger()
    {
        if (!log_file_) return;

        fprintf(log_file_, "\n" "LOGGING FINISHED" "\n\n");

        fclose(log_file_);
        log_file_ = nullptr;
    }

    template<typename... Types>
    int print_log(const char* format, Types&&... args)
    {
        if (!log_file_) return 0;

        fprintf(log_file_, format, std::forward<Types>(args)...);
    }

private:
    FILE* log_file_;
};

CLogger* CLogger::global_logger_ptr_ = nullptr;

}// namespace onegin_sort

#endif // ON_LOGGING_H_INCLUDED

