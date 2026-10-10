#include "../solutions/p0287_find_the_duplicate_number.hpp"

static void check(std::vector<int> a) {
    std::vector<int> freq(a.size());for(int x:a)++freq[x];int want=-1,duplicates=0;
    for(int x=1;x<static_cast<int>(a.size());++x)if(freq[x]>1){want=x;++duplicates;}
    if(duplicates!=1)return;
    hot100::p0287::Solution s;auto original=a;assert(s.findDuplicate(a)==want);assert(a==original);
}
int main() {
    check({3,1,3,4,2});check({2,2,2,2,2});check({1,1});
    for(int n=1;n<=5;++n){int total=1;for(int i=0;i<n+1;++i)total*=n;
        for(int mask=0;mask<total;++mask){int k=mask;std::vector<int> a(n+1);for(int& x:a){x=1+k%n;k/=n;}check(a);}}
    std::mt19937 rng(287);
    for(int n=2;n<=100;++n){std::vector<int> a(n);std::iota(a.begin(),a.end(),1);a.push_back(1+rng()%n);std::shuffle(a.begin(),a.end(),rng);check(a);}
}
