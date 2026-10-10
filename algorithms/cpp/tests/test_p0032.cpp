#include "../solutions/p0032_longest_valid_parentheses.hpp"

static int slow(const std::string& s) {
    int best=0;
    for(int i=0;i<static_cast<int>(s.size());++i){int balance=0;for(int j=i;j<static_cast<int>(s.size());++j){balance+=s[j]=='(' ? 1:-1;if(balance<0)break;if(balance==0)best=std::max(best,j-i+1);}}
    return best;
}
int main() {
    hot100::p0032::Solution sol;
    assert(sol.longestValidParentheses("()(())")==6);assert(sol.longestValidParentheses(")(()())(")==6);
    assert(sol.longestValidParentheses("")==0);assert(sol.longestValidParentheses("(((")==0);
    for(int n=0;n<=15;++n)for(int mask=0;mask<(1<<n);++mask){std::string s(n,'(');for(int i=0;i<n;++i)if(mask&(1<<i))s[i]=')';assert(sol.longestValidParentheses(s)==slow(s));}
}
