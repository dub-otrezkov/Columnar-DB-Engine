#pragma once

#include <utils/perf_stats/perf_stats.h>

#include <chrono>
#include <istream>
#include <memory>
#include <streambuf>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace JfEngine {

class TMmapStreambuf : public std::streambuf {
public:
    explicit TMmapStreambuf(const std::string& path) {
        fd_ = ::open(path.c_str(), O_RDONLY);
        if (fd_ < 0) {
            return;
        }
        struct stat st;
        if (::fstat(fd_, &st) < 0) {
            ::close(fd_);
            fd_ = -1;
            return;
        }
        size_ = static_cast<size_t>(st.st_size);
        if (size_ == 0) {
            return;
        }
        auto t0 = std::chrono::steady_clock::now();
        void* m = ::mmap(nullptr, size_, PROT_READ, MAP_SHARED, fd_, 0);
        if (m == MAP_FAILED) {
            ::close(fd_);
            fd_ = -1;
            return;
        }
        data_ = static_cast<char*>(m);
        ::madvise(m, size_, MADV_SEQUENTIAL);
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - t0).count();
        if (TQueryStats::instance) {
            TQueryStats::instance->RecordDiskRead(static_cast<uint64_t>(ns), 0);
        }
        setg(data_, data_, data_ + size_);
    }

    ~TMmapStreambuf() override {
        if (data_) {
            ::munmap(data_, size_);
        }
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    TMmapStreambuf(const TMmapStreambuf&) = delete;
    TMmapStreambuf& operator=(const TMmapStreambuf&) = delete;

    bool valid() const {
        return data_ != nullptr;
    }

protected:
    pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                     std::ios_base::openmode /*which*/) override {
        char* target;
        if (dir == std::ios_base::beg) {
            target = data_ + off;
        } else if (dir == std::ios_base::cur) {
            target = gptr() + off;
        } else {
            target = data_ + size_ + off;
        }
        if (target < data_ || target > data_ + size_) {
            return pos_type(off_type(-1));
        }
        setg(data_, target, data_ + size_);
        return pos_type(target - data_);
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override {
        return seekoff(off_type(pos), std::ios_base::beg, which);
    }

    std::streamsize xsgetn(char* s, std::streamsize n) override {
        if (TQueryStats::instance) {
            auto t0 = std::chrono::steady_clock::now();
            std::streamsize r = std::streambuf::xsgetn(s, n);
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now() - t0).count();
            TQueryStats::instance->RecordDiskRead(static_cast<uint64_t>(ns),
                                                   static_cast<uint64_t>(r));
            return r;
        }
        return std::streambuf::xsgetn(s, n);
    }

private:
    int fd_ = -1;
    char* data_ = nullptr;
    size_t size_ = 0;
};

class TMmapIstream : public std::iostream {
public:
    explicit TMmapIstream(const std::string& path)
        : std::iostream(nullptr),
          buf_(std::make_unique<TMmapStreambuf>(path))
    {
        rdbuf(buf_.get());
        if (!buf_->valid()) {
            setstate(std::ios::failbit);
        }
    }

private:
    std::unique_ptr<TMmapStreambuf> buf_;
};

} // namespace JfEngine
