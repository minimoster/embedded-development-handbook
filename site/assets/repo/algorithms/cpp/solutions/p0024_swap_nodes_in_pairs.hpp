#pragma once
#include "../support.hpp"

namespace hot100::p0024 {
class Solution {
public:
    ListNode* swapPairs(ListNode* head) {
        ListNode dummy(0, head);
        ListNode* before = &dummy;
        while (before->next && before->next->next) {
            ListNode* first = before->next;
            ListNode* second = first->next;
            first->next = second->next;
            second->next = first;
            before->next = second;
            before = first;
        }
        return dummy.next;
    }
};
} // namespace hot100::p0024
