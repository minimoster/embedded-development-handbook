#pragma once
#include "../support.hpp"

namespace hot100::p0002 {
class Solution {
public:
    ListNode* addTwoNumbers(ListNode* a, ListNode* b) {
        ListNode dummy;
        ListNode* tail = &dummy;
        int carry = 0;
        while (a || b || carry) {
            int total = carry;
            if (a) { total += a->val; a = a->next; }
            if (b) { total += b->val; b = b->next; }
            tail->next = new ListNode(total % 10);
            tail = tail->next;
            carry = total / 10;
        }
        return dummy.next;
    }
};
} // namespace hot100::p0002
