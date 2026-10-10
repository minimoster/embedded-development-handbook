#include "../solutions/p0139_word_break.hpp"

static bool slow(const std::string& s,const std::vector<std::string>& dict,std::size_t pos=0) {
    if(pos==s.size())return true;
    for(const auto& w:dict)if(!w.empty()&&pos+w.size()<=s.size()&&s.substr(pos,w.size())==w&&slow(s,dict,pos+w.size()))return true;
    return false;
}
int main() {
    hot100::p0139::Solution sol;
    std::vector<std::string> d{"ab","aba","c"};assert(sol.wordBreak("ababc",d));assert(!sol.wordBreak("abacb",d));assert(sol.wordBreak("",d));
    std::vector<std::string> reuse{"a","aa"};assert(sol.wordBreak("aaaaa",reuse));
    std::mt19937 rng(139);
    for(int z=0;z<1400;++z){std::string s(rng()%11,'a');for(char& c:s)c=static_cast<char>('a'+rng()%2);
        std::vector<std::string> words(1+rng()%6);for(auto& w:words){w=std::string(1+rng()%4,'a');for(char& c:w)c=static_cast<char>('a'+rng()%2);}assert(sol.wordBreak(s,words)==slow(s,words));}
}
