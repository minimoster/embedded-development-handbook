#pragma once
#include "../support.hpp"

namespace hot100::p0207 {
class Solution {
public:
    bool canFinish(int numCourses, std::vector<std::vector<int>>& prerequisites) {
        std::vector<std::vector<int>> outgoing(numCourses);
        std::vector<int> indegree(numCourses, 0);
        for (const auto& dependency : prerequisites) {
            int after = dependency[0], before = dependency[1];
            outgoing[before].push_back(after);
            ++indegree[after];
        }
        std::queue<int> ready;
        for (int course = 0; course < numCourses; ++course)
            if (indegree[course] == 0) ready.push(course);
        int completed = 0;
        while (!ready.empty()) {
            int course = ready.front();
            ready.pop();
            ++completed;
            for (int next : outgoing[course]) if (--indegree[next] == 0) ready.push(next);
        }
        return completed == numCourses;
    }
};
} // namespace hot100::p0207
