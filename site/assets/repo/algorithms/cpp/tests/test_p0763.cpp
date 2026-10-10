#include "../solutions/p0763_partition_labels.hpp"

static int bruteCount(const std::string& s) {
    int n=static_cast<int>(s.size()),best=0;if(!n)return 0;
    for(int cuts=0;cuts<(1<<(n-1));++cuts) {
        std::array<int,26> owner;owner.fill(-1);int part=0;bool valid=true;
        for(int i=0;i<n;++i){int c=s[i]-'a';if(owner[c]>=0&&owner[c]!=part)valid=false;owner[c]=part;if(i<n-1&&(cuts&(1<<i)))++part;}
        if(valid)best=std::max(best,part+1);
    }
    return best;
}
int main() {
    hot100::p0763::Solution sol;
    assert((sol.partitionLabels("abacddce")==std::vector<int>{3,4,1}));
    assert((sol.partitionLabels("zzzz")==std::vector<int>{4})); assert(sol.partitionLabels("").empty());
    for(int n=1;n<=8;++n){int total=1;for(int i=0;i<n;++i)total*=3;
        for(int mask=0;mask<total;++mask){int k=mask;std::string s(n,'a');for(char& c:s){c=static_cast<char>('a'+k%3);k/=3;}
            auto out=sol.partitionLabels(s);assert(static_cast<int>(out.size())==bruteCount(s));assert(std::accumulate(out.begin(),out.end(),0)==n);
            std::array<int,26> owner;owner.fill(-1);int pos=0;
            for(int p=0;p<static_cast<int>(out.size());++p)for(int j=0;j<out[p];++j){int c=s[pos++]-'a';assert(owner[c]==-1||owner[c]==p);owner[c]=p;}
        }
    }
}
