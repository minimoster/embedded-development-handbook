#include "../solutions/p0022_generate_parentheses.hpp"
using namespace hot100::p0022;
int main() {
Solution s;auto r=s.generateParenthesis(2);assert((r==std::vector<std::string>{"(())","()()"}));assert((s.generateParenthesis(0)==std::vector<std::string>{""}));const int counts[]={1,1,2,5,14,42,132};for(int n=0;n<=6;++n){r=s.generateParenthesis(n);assert(int(r.size())==counts[n]);assert(std::set<std::string>(r.begin(),r.end()).size()==r.size());for(auto&str:r){int bal=0;for(char c:str){bal+=c=='('?1:-1;assert(bal>=0);}assert(bal==0&&int(str.size())==2*n);}}
}
