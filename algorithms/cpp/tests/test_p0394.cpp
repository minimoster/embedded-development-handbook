#include "../solutions/p0394_decode_string.hpp"

static std::string parse(const std::string& s,std::size_t& pos) {
    std::string out;
    while(pos<s.size() && s[pos]!=']') {
        if(s[pos]>='0' && s[pos]<='9') {
            int count=0; while(s[pos]>='0'&&s[pos]<='9') count=count*10+s[pos++]-'0';
            ++pos; auto sub=parse(s,pos); ++pos; while(count--) out+=sub;
        } else out+=s[pos++];
    }
    return out;
}
int main() {
    hot100::p0394::Solution s;
    assert(s.decodeString("2[x3[y]]z")=="xyyyxyyyz");
    assert(s.decodeString("12[a]")==std::string(12,'a'));
    assert(s.decodeString("plain")=="plain"); assert(s.decodeString("")=="");
    std::mt19937 rng(394);
    std::function<std::string(int)> gen=[&](int depth) {
        std::string out;
        for(int i=0;i<3;++i) {
            if(depth && rng()%2) out+=std::to_string(1+rng()%3)+"["+gen(depth-1)+"]";
            else out+=static_cast<char>('a'+rng()%3);
        }
        return out;
    };
    for(int i=0;i<300;++i) { auto input=gen(3); std::size_t p=0; assert(s.decodeString(input)==parse(input,p)); assert(p==input.size()); }
}
