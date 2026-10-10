#include "../solutions/p0146_lru_cache.hpp"

struct Reference {
    int capacity;std::vector<std::pair<int,int>> entries;
    int get(int key){for(std::size_t i=0;i<entries.size();++i)if(entries[i].first==key){auto x=entries[i];entries.erase(entries.begin()+static_cast<std::ptrdiff_t>(i));entries.insert(entries.begin(),x);return x.second;}return -1;}
    void put(int key,int value){for(std::size_t i=0;i<entries.size();++i)if(entries[i].first==key){entries.erase(entries.begin()+static_cast<std::ptrdiff_t>(i));break;}if(capacity==0)return;entries.insert(entries.begin(),{key,value});if(entries.size()>static_cast<std::size_t>(capacity))entries.pop_back();}
};
int main() {
    hot100::p0146::LRUCache c(2);c.put(4,40);c.put(8,80);assert(c.get(4)==40);c.put(5,50);assert(c.get(8)==-1);c.put(4,41);assert(c.get(4)==41);assert(c.get(5)==50);
    std::mt19937 rng(146);
    for(int cap=0;cap<9;++cap){hot100::p0146::LRUCache cache(cap);Reference ref{cap,{}};
        for(int i=0;i<3500;++i){int key=static_cast<int>(rng()%20);if(rng()%2){int value=static_cast<int>(rng()%1000);cache.put(key,value);ref.put(key,value);}else{assert(cache.get(key)==ref.get(key));}}
        for(int key=0;key<20;++key)assert(cache.get(key)==ref.get(key));
    }
}
