#include "../solutions/p0169_majority_element.hpp"

int main() {
    hot100::p0169::Solution s;
    std::vector<int> a{5,1,5,2,5,5};assert(s.majorityElement(a)==5);
    std::vector<int> one{INT_MIN};assert(s.majorityElement(one)==INT_MIN);
    std::mt19937 rng(169);
    for(int z=0;z<2500;++z){int n=1+rng()%80,majority=static_cast<int>(rng()%31)-15;std::vector<int> v(n,majority);
        for(int i=0;i<(n-1)/2;++i){int x=static_cast<int>(rng()%31)-15;v[i]=x;}
        std::shuffle(v.begin(),v.end(),rng);assert(s.majorityElement(v)==majority);
        assert(std::count(v.begin(),v.end(),majority)>n/2);}
}
