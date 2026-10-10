#include "../solutions/p0155_min_stack.hpp"

int main() {
    hot100::p0155::MinStack s;
    s.push(4); s.push(-2); s.push(-2); s.push(7);
    assert(s.getMin()==-2); s.pop(); s.pop(); assert(s.getMin()==-2);
    s.pop(); assert(s.top()==4 && s.getMin()==4); s.pop();
    s.push(INT_MIN); s.push(INT_MAX); assert(s.getMin()==INT_MIN); s.pop(); s.pop();
    std::mt19937 rng(155); std::vector<int> ref;
    for(int i=0;i<20000;++i) {
        if(ref.empty() || rng()%3) { int x=static_cast<int>(rng()%2001)-1000; s.push(x); ref.push_back(x); }
        else { s.pop(); ref.pop_back(); }
        if(!ref.empty()) { assert(s.top()==ref.back()); assert(s.getMin()==*std::min_element(ref.begin(),ref.end())); }
    }
    while(!ref.empty()) { s.pop(); ref.pop_back(); }
    bool caught=false; try { s.getMin(); } catch(const std::out_of_range&) { caught=true; } assert(caught);
}
