#pragma once
#include "../support.hpp"

namespace hot100::p0234 {
class Solution {
    static ListNode* reverse(ListNode* head) {
        ListNode* previous = nullptr;
        while (head) {
            ListNode* next = head->next;
            head->next = previous;
            previous = head;
            head = next;
        }
        return previous;
    }
public:
    bool isPalindrome(ListNode* head) {
        if (!head || !head->next) return true;
        ListNode* slow = head;
        ListNode* fast = head;
        while (fast->next && fast->next->next) {
            slow = slow->next;
            fast = fast->next->next;
        }
        ListNode* second = reverse(slow->next);
        bool equal = true;
        for (ListNode* a = head, *b = second; b; a = a->next, b = b->next) {
            if (a->val != b->val) { equal = false; break; }
        }
        slow->next = reverse(second);
        return equal;
    }
};
} // namespace hot100::p0234
