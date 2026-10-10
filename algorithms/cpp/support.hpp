#pragma once
// Local teaching harness. Online judges normally provide node declarations.
#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


// This teaching target deliberately matches the tested integer value range.
// C++17 alone does not promise these byte, range, or execution-encoding properties.
namespace hot100::target_contract {
constexpr bool asciiLettersAndDigits() {
    constexpr char lower[] = "abcdefghijklmnopqrstuvwxyz";
    constexpr char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    constexpr char digits[] = "0123456789";
    for (int i = 0; i < 26; ++i) {
        if (static_cast<unsigned char>(lower[i]) != 0x61 + i ||
            static_cast<unsigned char>(upper[i]) != 0x41 + i) return false;
    }
    for (int i = 0; i < 10; ++i)
        if (static_cast<unsigned char>(digits[i]) != 0x30 + i) return false;
    return true;
}
static_assert(CHAR_BIT == 8 && UCHAR_MAX == 255,
              "Hot100 teaching target requires 8-bit bytes");
static_assert(std::numeric_limits<int>::digits == 31 &&
              INT_MAX == 2147483647 && INT_MIN == (-2147483647 - 1),
              "Hot100 teaching target requires the signed 32-bit int value range");
static_assert(std::numeric_limits<long long>::digits >= 63,
              "Hot100 teaching target requires at least 63 long long value bits");
static_assert(asciiLettersAndDigits(),
              "Hot100 teaching target requires ASCII letters and digits");
} // namespace hot100::target_contract

struct ListNode {
    int val;
    ListNode* next;
    explicit ListNode(int value = 0, ListNode* link = nullptr) : val(value), next(link) {}
};
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    explicit TreeNode(int value = 0, TreeNode* l = nullptr, TreeNode* r = nullptr)
        : val(value), left(l), right(r) {}
};
struct Node {
    int val;
    Node* next;
    Node* random;
    explicit Node(int value) : val(value), next(nullptr), random(nullptr) {}
};

namespace testing {
template<class T> struct Arena {
    std::vector<std::unique_ptr<T>> nodes;
    T* make(int value) {
        nodes.push_back(std::make_unique<T>(value));
        return nodes.back().get();
    }
};
using ListArena = Arena<ListNode>;
using TreeArena = Arena<TreeNode>;
using RandomArena = Arena<Node>;
inline std::vector<int> listValues(ListNode* head) {
    std::vector<int> values;
    std::unordered_set<ListNode*> visited;
    for (; head; head = head->next) {
        if (!visited.insert(head).second) throw std::logic_error("cycle in listValues");
        values.push_back(head->val);
    }
    return values;
}
// Only use destroy helpers for newly allocated outputs, never arena-owned nodes.
inline void destroyList(ListNode* head) {
    while (head) { ListNode* next = head->next; delete head; head = next; }
}
inline void destroyTree(TreeNode* root) {
    if (!root) return;
    std::vector<TreeNode*> pending{root};
    while (!pending.empty()) {
        TreeNode* node = pending.back(); pending.pop_back();
        if (node->left) pending.push_back(node->left);
        if (node->right) pending.push_back(node->right);
        delete node;
    }
}
inline void destroyRandomList(Node* head) {
    while (head) { Node* next = head->next; delete head; head = next; }
}
} // namespace testing
