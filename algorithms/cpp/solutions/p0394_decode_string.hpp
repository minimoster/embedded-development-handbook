#pragma once
#include "../support.hpp"

namespace hot100::p0394 {
class Solution {
public:
    std::string decodeString(std::string s) {
        struct Frame { std::string prefix; std::size_t repeat; };
        std::vector<Frame> frames;
        std::string current;
        std::size_t count = 0;
        for (char c : s) {
            if (c >= '0' && c <= '9') count = count * 10 + static_cast<std::size_t>(c - '0');
            else if (c == '[') {
                frames.push_back({std::move(current), count});
                current.clear(); count = 0;
            } else if (c == ']') {
                Frame f = std::move(frames.back()); frames.pop_back();
                if (!current.empty() && f.repeat > (f.prefix.max_size() - f.prefix.size()) / current.size())
                    throw std::length_error("decoded string too large");
                f.prefix.reserve(f.prefix.size() + f.repeat * current.size());
                for (std::size_t i = 0; i < f.repeat; ++i) f.prefix += current;
                current = std::move(f.prefix);
            } else current.push_back(c);
        }
        return current;
    }
};
} // namespace hot100::p0394
