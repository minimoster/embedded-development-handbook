#include "../solutions/p0131_palindrome_partitioning.hpp"
using namespace hot100::p0131;
int main() {
Solution s;auto r=s.partition("aba");std::sort(r.begin(),r.end());assert((r==std::vector<std::vector<std::string>>{{"a","b","a"},{"aba"}}));assert((s.partition("")==std::vector<std::vector<std::string>>{{}}));
for(int n=1;n<=8;++n){std::string a(n,'a');r=s.partition(a);assert(r.size()==std::size_t(1)<<(n-1));for(const auto&parts:r){std::string joined;for(auto v:parts){auto rev=v;std::reverse(rev.begin(),rev.end());assert(v==rev);joined+=v;}assert(joined==a);}assert(std::set<std::vector<std::string>>(r.begin(),r.end()).size()==r.size());}
}
