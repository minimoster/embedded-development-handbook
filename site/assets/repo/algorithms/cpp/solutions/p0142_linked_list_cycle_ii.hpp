#pragma once
#include "../support.hpp"

namespace hot100::p0142 {
class Solution {
public:
    ListNode* detectCycle(ListNode* head) {
        ListNode* slow = head;
        ListNode* fast = head;
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
            if (slow == fast) {
                ListNode* entrance = head;
                while (entrance != slow) {
                    entrance = entrance->next;
                    slow = slow->next;
                }
                return entrance;
            }
        }
        return nullptr;
    }
};
} // namespace hot100::p0142
