#pragma once
#include "../support.hpp"

namespace hot100::p0019 {
class Solution {
public:
    ListNode* removeNthFromEnd(ListNode* head, int n) {
        if (n <= 0) return head;
        ListNode dummy(0, head);
        ListNode* fast = &dummy;
        for (int i = 0; i < n; ++i) {
            fast = fast->next;
            if (!fast) return head;
        }
        ListNode* slow = &dummy;
        while (fast->next) { fast = fast->next; slow = slow->next; }
        ListNode* victim = slow->next;
        slow->next = victim->next;
        victim->next = nullptr; // borrowed node: the caller retains ownership
        return dummy.next;
    }
};
} // namespace hot100::p0019
