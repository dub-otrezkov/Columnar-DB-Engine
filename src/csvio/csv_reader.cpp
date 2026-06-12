#include "csv_reader.h"

#include <algorithm>

namespace JfEngine {

TCsvReader::TCsvReader(IFileInput* in, char sep) :
    in_(in),
    sep_(sep)
{
}

Expected<std::vector<std::string>> TCsvReader::ReadRow() {
    if (in_->Eof()) {
        return {std::vector<std::string>(), MakeError<EError::EofErr>()};
    }

    bool read_smth = false;
    bool in_quotes = false;

    std::vector<std::string> ans;
    ans.push_back("");

    while (!in_->Eof()) {
        auto c = in_->Get();

        if (in_quotes && c == EOF) {
            return {std::vector<std::string>(), MakeError<EError::EofErr>()};
        }
        if ((!in_quotes && (c == '\n' || c == '\r')) || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (in_->Peek() == '"') {
                    ans.back() += static_cast<char>(in_->Get());
                } else if (in_->Peek() != sep_ && in_->Peek() != '\n' && in_->Peek() != '\r') {
                    return {std::vector<std::string>(), MakeError<EError::EofErr>()};
                } else {
                    in_quotes = false;
                }
            } else if (ans.back().empty()) {
                in_quotes = true;
            } else {
                ans.back() += static_cast<char>(c);
            }
        } else {
            ans.back() += static_cast<char>(c);
        }
    }

    if (!read_smth) {
        return {std::move(ans), MakeError<EError::EofErr>()};
    }

    return {std::move(ans)};
}

TCsvOptimizedReader::TCsvOptimizedReader(IFileInput* in, char sep) :
    in_(in),
    sep_(sep)
{
}

void TCsvOptimizedReader::Refill() {
    ui64 remaining = in_->Size() - in_->TellPos();
    ui64 to_read = std::min<ui64>(kIBufSize, remaining);
    if (to_read == 0) {
        av_ = 0;
        cpos_ = 0;
        return;
    }
    in_->Read(buf_, to_read);
    av_ = to_read;
    cpos_ = 0;
}

int TCsvOptimizedReader::ReadSym() {
    if (cpos_ < static_cast<i64>(av_)) {
        return static_cast<unsigned char>(buf_[cpos_++]);
    }
    Refill();
    if (av_ == 0) {
        return EOF;
    }
    return static_cast<unsigned char>(buf_[cpos_++]);
}


bool TCsvOptimizedReader::EofC() {
    return cpos_ >= static_cast<i64>(av_) && in_->TellPos() >= in_->Size();
}

int TCsvOptimizedReader::Peek() {
    if (cpos_ < static_cast<i64>(av_)) {
        return static_cast<unsigned char>(buf_[cpos_]);
    }
    if (in_->TellPos() >= in_->Size()) {
        return EOF;
    }
    Refill();
    if (av_ == 0) {
        return EOF;
    }
    return static_cast<unsigned char>(buf_[cpos_]);
}

Expected<std::vector<std::string>> TCsvOptimizedReader::ReadRow() {
    if (EofC()) {
        return {std::vector<std::string>(), MakeError<EError::EofErr>()};
    }

    bool read_smth = false;
    bool in_quotes = false;

    Expected<std::vector<std::string>> ans_e{std::vector<std::string>()};

    auto& ans = ans_e.GetRes();
    ans.push_back("");

    while (!EofC()) {
        auto c = ReadSym();
        if (in_quotes && c == EOF) {
            return {std::vector<std::string>(), MakeError<EError::EofErr>()};
        }
        if ((!in_quotes && (c == '\n' || c == '\r')) || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            ans.emplace_back();
        } else if (c == '"') {
            if (in_quotes) {
                if (Peek() == '"') {
                    ans.back() += static_cast<char>(ReadSym());
                } else if (Peek() != sep_ && Peek() != '\n' && Peek() != '\r') {
                    return {std::vector<std::string>(), MakeError<EError::EofErr>()};
                } else {
                    in_quotes = false;
                }
            } else if (ans.back().empty()) {
                in_quotes = true;
            } else {
                ans.back() += static_cast<char>(c);
            }
        } else {
            ans.back() += static_cast<char>(c);
        }
    }

    if (!read_smth) {
        return MakeError<EError::EofErr>();
    }

    return ans_e;
}

Expected<void> TCsvOptimizedReader::ReadRow(TVectorString2d& out) {
    if (EofC()) {
        return MakeError<EError::EofErr>();
    }

    bool read_smth = false;
    bool in_quotes = false;

    out.NewRow();

    while (!EofC()) {
        auto c = ReadSym();
        if (in_quotes && c == EOF) {
            return MakeError<EError::EofErr>();
        }
        if ((!in_quotes && (c == '\n' || c == '\r')) || c == EOF) {
            break;
        }

        read_smth = true;

        if (!in_quotes && c == sep_) {
            out.NewCol();
        } else if (c == '"') {
            if (in_quotes) {
                if (Peek() == '"') {
                    out.WriteSymToLastCR(static_cast<char>(ReadSym()));
                } else if (Peek() != sep_ && Peek() != '\n' && Peek() != '\r') {
                    return MakeError<EError::EofErr>();
                } else {
                    in_quotes = false;
                }
            } else if (out.LastEmpty()) {
                in_quotes = true;
            } else {
                out.WriteSymToLastCR(static_cast<char>(c));
            }
        } else {
            out.WriteSymToLastCR(static_cast<char>(c));
        }
    }

    if (!read_smth) {
        return EError::EofErr;
    }

    return EError::NoError;
}

}
