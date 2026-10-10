#pragma once
#include "../support.hpp"

namespace hot100::p0025 {
class Solution {
public:
    ListNode* reverseKGroup(ListNode* head, int k) {
        if (k <= 1) return head;
        ListNode dummy(0, head);
        ListNode* before = &dummy;
        while (true) {
            ListNode* kth = before;
            for (int i = 0; i < k && kth; ++i) kth = kth->next;
            if (!kth) break;
            ListNode* after = kth->next;
            ListNode* oldHead = before->next;
            ListNode* previous = after;
            ListNode* current = oldHead;
            while (current != after) {
                ListNode* next = current->next;
                current->next = previous;
                previous = current;
                current = next;
            }
            before->next = kth;
            before = oldHead;
        }
        return dummy.next;
    }
};
} // namespace hot100::p0025
