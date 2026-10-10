#pragma once
#include "../support.hpp"

namespace hot100::p0208 {
class Trie {
    static constexpr std::size_t missing = std::numeric_limits<std::size_t>::max();
    struct Entry {
        std::array<std::size_t, 26> children;
        bool terminal = false;
        Entry() { children.fill(missing); }
    };
    std::vector<Entry> nodes_{1};
    static void validate(const std::string& text) {
        for (char ch : text)
            if (ch < 'a' || ch > 'z') throw std::invalid_argument("Trie expects lowercase ASCII");
    }
    std::size_t lookup(const std::string& text) const {
        validate(text);
        std::size_t current = 0;
        for (char ch : text) {
            current = nodes_[current].children[static_cast<std::size_t>(ch - 'a')];
            if (current == missing) break;
        }
        return current;
    }
public:
    Trie() = default;
    void insert(const std::string& word) {
        validate(word);
        std::size_t current = 0;
        for (char ch : word) {
            std::size_t edge = static_cast<std::size_t>(ch - 'a');
            std::size_t next = nodes_[current].children[edge];
            if (next == missing) {
                next = nodes_.size();
                nodes_.emplace_back(); // may relocate storage; keep indexes, not references
                nodes_[current].children[edge] = next;
            }
            current = next;
        }
        nodes_[current].terminal = true;
    }
    bool search(const std::string& word) const {
        std::size_t found = lookup(word);
        return found != missing && nodes_[found].terminal;
    }
    bool startsWith(const std::string& prefix) const { return lookup(prefix) != missing; }
};
} // namespace hot100::p0208
