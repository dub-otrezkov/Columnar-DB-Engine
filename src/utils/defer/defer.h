#pragma once

#include <functional>
#include <utility>

namespace JfEngine {

class TDefer {
public:
    template<typename F>
    explicit TDefer(F&& f) : f_(std::forward<F>(f)) {}

    TDefer(const TDefer&) = delete;
    TDefer& operator=(const TDefer&) = delete;
    TDefer(TDefer&&) = delete;
    TDefer& operator=(TDefer&&) = delete;

    ~TDefer() {
        if (f_) {
            f_();
        }
    }

private:
    std::function<void()> f_;
};

} // namespace JfEngine
