#pragma once
#include "../support.hpp"

namespace hot100::p0148 {
class Solution {
    static ListNode* split(ListNode* head, std::size_t width) {
        if (!head) return nullptr;
        for (std::size_t i = 1; i < width && head->next; ++i) head = head->next;
        ListNode* rest = head->next;
        head->next = nullptr;
        return rest;
    }
    static ListNode* appendMerge(ListNode* a, ListNode* b, ListNode* tail) {
        while (a && b) {
            if (a->val <= b->val) { tail->next = a; a = a->next; }
            else { tail->next = b; b = b->next; }
            tail = tail->next;
        }
        tail->next = a ? a : b;
        while (tail->next) tail = tail->next;
        return tail;
    }
public:
    ListNode* sortList(ListNode* head) {
        std::size_t count = 0;
        for (ListNode* p = head; p; p = p->next) ++count;
        if (count < 2) return head;
        for (std::size_t width = 1; width < count;
             width = width > count / 2 ? count : width * 2) {
            ListNode dummy(0, head);
            ListNode* tail = &dummy;
            ListNode* current = head;
            while (current) {
                ListNode* left = current;
                ListNode* right = split(left, width);
                current = split(right, width);
                tail = appendMerge(left, right, tail);
            }
            head = dummy.next;
        }
        return head;
    }
};
} // namespace hot100::p0148
