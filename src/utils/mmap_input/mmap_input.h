#pragma once

#include "utils/cint/int.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <istream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace JfEngine {

class IFileInput {
public:
    virtual ~IFileInput() = default;

    virtual ui64 TellPos() = 0;
    virtual void SetPos(i64) = 0;

    virtual ui64 Size() const = 0;

    virtual void Read(const char*&, ui64 n) = 0;

    virtual bool Eof() = 0;
    virtual int Get() = 0;
    virtual int Peek() = 0;
};

class IFileOutput {
public:
    virtual ~IFileOutput() = default;

    virtual ui64 TellPos() = 0;
    virtual void SetPos(i64) = 0;

    virtual void Write(const char*&, ui64 n) = 0;

    virtual void Flush() = 0;
};

using IFileInputPtr  = std::shared_ptr<IFileInput>;
using IFileOutputPtr = std::shared_ptr<IFileOutput>;

class TMMapFileInput final : public IFileInput {
public:
    TMMapFileInput(const std::string& name) {
        int fd = open(name.c_str(), O_RDONLY);
        if (fd < 0) {
            std::cerr << "TMMapFileInput: cannot open '" << name
                      << "': " << std::strerror(errno) << std::endl;
            std::abort();
        }

        struct stat sb{};
        if (fstat(fd, &sb) < 0) {
            std::cerr << "TMMapFileInput: fstat failed for '" << name
                      << "': " << std::strerror(errno) << std::endl;
            close(fd);
            std::abort();
        }

        file_size_ = static_cast<ui64>(sb.st_size);

        if (file_size_ == 0) {
            file_ = nullptr;
            close(fd);
            return;
        }

        void* p = mmap(nullptr, file_size_, PROT_READ, MAP_PRIVATE, fd, 0);
        if (p == MAP_FAILED) {
            std::cerr << "TMMapFileInput: mmap failed for '" << name
                      << "' (size=" << file_size_ << "): "
                      << std::strerror(errno) << std::endl;
            close(fd);
            std::abort();
        }
        file_ = reinterpret_cast<char*>(p);
        close(fd);
    }

    ~TMMapFileInput() {
        if (file_) {
            munmap(file_, file_size_);
        }
    }

    TMMapFileInput(const TMMapFileInput&) = delete;
    TMMapFileInput& operator=(const TMMapFileInput&) = delete;

    ui64 TellPos() override {
        return offset_;
    }

    void SetPos(i64 p) override {
        offset_ = static_cast<ui64>(p);
    }

    ui64 Size() const override {
        return file_size_;
    }

    void Read(const char*& out, ui64 n) override {
        out = file_ + offset_;
        offset_ += n;
    }

    bool Eof() override {
        return offset_ >= file_size_;
    }

    int Get() override {
        if (offset_ >= file_size_) {
            return EOF;
        }
        return static_cast<unsigned char>(file_[offset_++]);
    }

    int Peek() override {
        if (offset_ >= file_size_) {
            return EOF;
        }
        return static_cast<unsigned char>(file_[offset_]);
    }

private:
    char* file_ = nullptr;
    ui64 offset_ = 0;
    ui64 file_size_ = 0;
};

class TIstreamFileInput final : public IFileInput {
public:
    explicit TIstreamFileInput(std::shared_ptr<std::istream> in) : in_(std::move(in)) {
        in_->clear();
    }

    ui64 TellPos() override {
        return static_cast<ui64>(in_->tellg());
    }

    void SetPos(i64 p) override {
        in_->clear();
        in_->seekg(p);
    }

    ui64 Size() const override {
        in_->clear();
        auto cur = in_->tellg();
        in_->seekg(0, std::ios::end);
        auto end = in_->tellg();
        in_->seekg(cur);
        return static_cast<ui64>(end);
    }

    void Read(const char*& dst, ui64 n) override {
        buf_.resize(n);
        in_->read(buf_.data(), n);
        dst = buf_.data();
    }

    bool Eof() override {
        return in_->eof() || in_->peek() == EOF;
    }

    int Get() override {
        return in_->get();
    }

    int Peek() override {
        return in_->peek();
    }

private:
    std::shared_ptr<std::istream> in_;
    std::vector<char> buf_;
};


class TStringStreamFileInput final : public IFileInput {
public:
    TStringStreamFileInput() = default;
    explicit TStringStreamFileInput(const std::string& init) : ss_(init) {}

    ui64 TellPos() override {
        return static_cast<ui64>(ss_.tellg());
    }

    void SetPos(i64 p) override {
        ss_.clear();
        ss_.seekg(p);
    }

    ui64 Size() const override {
        return ss_.view().size();
    }

    void Read(const char*& dst, ui64 n) override {
        buf_.resize(n);
        ss_.read(buf_.data(), n);
        dst = buf_.data();
    }

    bool Eof() override {
        return ss_.eof() || ss_.peek() == EOF;
    }

    int Get() override {
        return ss_.get();
    }

    int Peek() override {
        return ss_.peek();
    }

    std::stringstream& Stream() {
        return ss_;
    }

    std::string Str() const {
        return ss_.str();
    }

private:
    std::stringstream ss_;
    std::vector<char> buf_;
};


class TOstreamFileOutput final : public IFileOutput {
public:
    explicit TOstreamFileOutput(std::shared_ptr<std::ostream> out) : out_(std::move(out)) {}

    ui64 TellPos() override {
        return static_cast<ui64>(out_->tellp());
    }

    void SetPos(i64 p) override {
        out_->clear();
        out_->seekp(p);
    }

    void Write(const char*& src, ui64 n) override {
        out_->write(src, n);
    }

    void Flush() override {
        out_->flush();
    }

private:
    std::shared_ptr<std::ostream> out_;
};


class TStringStreamFileOutput final : public IFileOutput {
public:
    TStringStreamFileOutput() = default;

    ui64 TellPos() override {
        return static_cast<ui64>(ss_.tellp());
    }

    void SetPos(i64 p) override {
        ss_.clear();
        ss_.seekp(p);
    }

    void Write(const char*& src, ui64 n) override {
        ss_.write(src, n);
    }

    void Flush() override {
        ss_.flush();
    }

    std::stringstream& Stream() {
        return ss_;
    }

    std::string Str() const {
        return ss_.str();
    }

private:
    std::stringstream ss_;
};


template <typename T>
inline void PutInt(IFileOutput* out, T v) {
    char buf[sizeof(T)];
    std::memcpy(buf, &v, sizeof(T));
    const char* p = buf;
    out->Write(p, sizeof(T));
}

template <typename T>
inline T ReadInt(IFileInput* in) {
    const char* p = nullptr;
    in->Read(p, sizeof(T));
    T ans = 0;
    std::memcpy(&ans, p, sizeof(T));
    return ans;
}

inline void PutI64(IFileOutput* o, i64 v) {
    PutInt<i64>(o, v);
}

inline i64 ReadI64(IFileInput* i) {
    return ReadInt<i64>(i);
}

} // namespace JfEngine
