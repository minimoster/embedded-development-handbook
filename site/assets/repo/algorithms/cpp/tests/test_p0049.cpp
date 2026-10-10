#include "../solutions/p0049_group_anagrams.hpp"
using namespace hot100::p0049;
int main() {
Solution s;std::vector<std::string>a{"pots","stop","abc","tops",""};auto r=s.groupAnagrams(a);for(auto&g:r)std::sort(g.begin(),g.end());std::sort(r.begin(),r.end());std::vector<std::vector<std::string>>e{{""},{"abc"},{"pots","stop","tops"}};std::sort(e.begin(),e.end());assert(r==e);
a={"",""};assert(s.groupAnagrams(a).at(0).size()==2);a={};assert(s.groupAnagrams(a).empty());
}
