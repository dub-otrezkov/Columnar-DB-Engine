#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace JfEngine {

class TLogger {
public:
    static std::ostream* out_;

    static inline void Write(
        const char* file,
        int line,
        const void* obj,
        const std::string& msg
    ) {
        if (!out_) {
            return;
        }
        (*out_) << file << ":" << line << " " << obj << " " << msg << '\n';
    }
};

} // namespace JfEngine

// #define HAND_NDEBUG 1

#ifdef NDEBUG
    #define JF_LOG(obj, msg) ((void)0)
#elif HAND_NDEBUG
    #define JF_LOG(obj, msg) ((void)0)
#else
    #define JF_LOG(obj, msg)                                            \
        do {                                                            \
            std::stringstream jf_log_ss_;                               \
            jf_log_ss_ << msg;                                          \
            ::JfEngine::TLogger::Write(                                 \
                __FILE__, __LINE__, (obj), jf_log_ss_.str()             \
            );                                                          \
        } while (0)
#endif
