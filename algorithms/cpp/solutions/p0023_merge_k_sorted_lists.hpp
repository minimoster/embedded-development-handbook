#pragma once
#include "../support.hpp"

namespace hot100::p0023 {
class Solution {
    struct GreaterValue {
        bool operator()(const ListNode* a, const ListNode* b) const {
            return a->val > b->val;
        }
    };
public:
    ListNode* mergeKLists(std::vector<ListNode*>& lists) {
        std::priority_queue<ListNode*, std::vector<ListNode*>, GreaterValue> frontier;
        for (ListNode* head : lists) if (head) frontier.push(head);
        ListNode dummy;
        ListNode* tail = &dummy;
        while (!frontier.empty()) {
            ListNode* node = frontier.top();
            frontier.pop();
            if (node->next) frontier.push(node->next);
            tail->next = node;
            tail = node;
        }
        tail->next = nullptr;
        return dummy.next;
    }
};
} // namespace hot100::p0023
