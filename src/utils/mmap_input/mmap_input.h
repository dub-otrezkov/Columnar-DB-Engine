#pragma once

#include "utils/cint/int.h"

#include <cstring>
#include <istream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace JfEngine {

class IFileInput {
public:
    virtual ~IFileInput() = default;

    virtual ui64 TellPos() = 0;
    virtual void SetPos(i64) = 0;

    virtual ui64 Size() const = 0;

    virtual void Read(char*&, ui64 n) = 0;

    virtual bool Eof() = 0;
    virtual int Get() = 0;
    virtual int Peek() = 0;
};

class IFileOutput {
public:
    virtual ~IFileOutput() = default;

    virtual ui64 TellPos() = 0;
    virtual void SetPos(i64) = 0;

    virtual void Write(char*&, ui64 n) = 0;
};

using IFileInputPtr  = std::shared_ptr<IFileInput>;
using IFileOutputPtr = std::shared_ptr<IFileOutput>;


class TIstreamFileInput final : public IFileInput {
public:
    explicit TIstreamFileInput(std::shared_ptr<std::istream> in) : in_(std::move(in)) {
        in_->clear();
        in_->seekg(0, std::ios::end);
        size_ = static_cast<ui64>(in_->tellg());
        in_->seekg(0, std::ios::beg);
    }

    ui64 TellPos() override {
        return static_cast<ui64>(in_->tellg());
    }

    void SetPos(i64 p) override {
        in_->clear();
        in_->seekg(p);
    }

    ui64 Size() const override {
        return size_;
    }

    void Read(char*& dst, ui64 n) override {
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
    ui64 size_ = 0;
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

    void Read(char*& dst, ui64 n) override {
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

    void Write(char*& src, ui64 n) override {
        out_->write(src, n);
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

    void Write(char*& src, ui64 n) override {
        ss_.write(src, n);
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
    char* p = buf;
    out->Write(p, sizeof(T));
}

template <typename T>
inline T ReadInt(IFileInput* in) {
    char* p = nullptr;
    in->Read(p, sizeof(T));
    T ans = 0;
    std::memcpy(&ans, p, sizeof(T));
    return ans;
}

inline void PutI8(IFileOutput* o, i8 v) {
    PutInt<i8>(o, v);
}

inline void PutI16(IFileOutput* o, i16 v) {
    PutInt<i16>(o, v);
}

inline void PutI32(IFileOutput* o, i32 v) {
    PutInt<i32>(o, v);
}

inline void PutI64(IFileOutput* o, i64 v) {
    PutInt<i64>(o, v);
}

inline i8 ReadI8(IFileInput* i) {
    return ReadInt<i8>(i);
}

inline i16 ReadI16(IFileInput* i) {
    return ReadInt<i16>(i);
}

inline i32 ReadI32(IFileInput* i) {
    return ReadInt<i32>(i);
}

inline i64 ReadI64(IFileInput* i) {
    return ReadInt<i64>(i);
}

} // namespace JfEngine
