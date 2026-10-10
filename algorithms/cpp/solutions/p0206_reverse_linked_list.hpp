#pragma once
#include "../support.hpp"

namespace hot100::p0206 {
class Solution {
public:
    ListNode* reverseList(ListNode* head) {
        ListNode* previous = nullptr;
        for (ListNode* current = head; current;) {
            ListNode* following = current->next;
            current->next = previous;
            previous = current;
            current = following;
        }
        return previous;
    }
};
} // namespace hot100::p0206
