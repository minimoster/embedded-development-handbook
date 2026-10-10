#pragma once
#include "../support.hpp"

namespace hot100::p0138 {
class Solution {
public:
    Node* copyRandomList(Node* head) {
        if (!head) return nullptr;
        std::unordered_map<Node*, std::unique_ptr<Node>> copies;
        for (Node* p = head; p; p = p->next)
            copies.emplace(p, std::make_unique<Node>(p->val));
        for (Node* p = head; p; p = p->next) {
            Node* copy = copies.at(p).get();
            copy->next = p->next ? copies.at(p->next).get() : nullptr;
            copy->random = p->random ? copies.at(p->random).get() : nullptr;
        }
        Node* answer = copies.at(head).get();
        for (auto& entry : copies) entry.second.release();
        return answer;
    }
};
} // namespace hot100::p0138
