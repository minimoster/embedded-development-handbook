#include "../solutions/p0136_single_number.hpp"

int main() {
    hot100::p0136::Solution s;
    std::vector<int> a{-4,8,3,8,3};assert(s.singleNumber(a)==-4);
    std::vector<int> zero{0},extreme{INT_MIN,INT_MAX,INT_MAX};assert(s.singleNumber(zero)==0);assert(s.singleNumber(extreme)==INT_MIN);
    std::mt19937 rng(136);
    for(int z=0;z<1800;++z){int unique=static_cast<int>(rng()%1000)-500;std::vector<int> v{unique};std::set<int> used{unique};
        for(int i=0;i<20;++i){int x=static_cast<int>(rng()%1000)-500;if(used.insert(x).second){v.push_back(x);v.push_back(x);}}
        std::shuffle(v.begin(),v.end(),rng);assert(s.singleNumber(v)==unique);}
}
