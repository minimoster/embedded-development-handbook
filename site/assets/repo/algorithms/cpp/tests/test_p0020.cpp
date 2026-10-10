#include "../solutions/p0020_valid_parentheses.hpp"

static bool slow(std::string s) {
    while(true) {
        bool changed=false;
        for(const std::string p:{"()","[]","{}"}) {
            auto pos=s.find(p);
            if(pos!=std::string::npos) { s.erase(pos,2); changed=true; break; }
        }
        if(!changed) return s.empty();
    }
}
int main() {
    hot100::p0020::Solution s;
    assert(s.isValid("{[()]}()")); assert(!s.isValid("([)]"));
    assert(!s.isValid(")(")); assert(!s.isValid("((")); assert(s.isValid(""));
    std::string alphabet="()[]{}";
    for(int n=0;n<=6;++n) {
        int total=1; for(int i=0;i<n;++i) total*=6;
        for(int mask=0;mask<total;++mask) {
            int k=mask; std::string v(n,' '); for(char& c:v) { c=alphabet[k%6]; k/=6; }
            assert(s.isValid(v)==slow(v));
        }
    }
}
