#include "../solutions/p0208_implement_trie_prefix_tree.hpp"

int main() {
    hot100::p0208::Trie trie;assert(!trie.search("ca"));assert(trie.startsWith(""));assert(!trie.search(""));
    trie.insert("cat");trie.insert("car");assert(!trie.search("ca"));assert(trie.startsWith("ca"));trie.insert("ca");assert(trie.search("ca"));trie.insert("cat");
    trie.insert("");assert(trie.search(""));bool threw=false;try{trie.insert("dog!");}catch(const std::invalid_argument&){threw=true;}assert(threw);assert(!trie.startsWith("dog"));
    hot100::p0208::Trie randomTrie;std::set<std::string> words;std::mt19937 rng(208);
    auto makeWord=[&](){std::string word;int n=1+static_cast<int>(rng()%10);for(int i=0;i<n;++i)word.push_back(static_cast<char>('a'+rng()%8));return word;};
    for(int round=0;round<1600;++round){std::string w=makeWord();if(rng()%2){randomTrie.insert(w);words.insert(w);}else assert(randomTrie.search(w)==(words.count(w)!=0));std::string p=w.substr(0,rng()%(w.size()+1));bool expected=p.empty();for(const auto&known:words)if(known.compare(0,p.size(),p)==0){expected=true;break;}assert(randomTrie.startsWith(p)==expected);}
    for(const auto&w:words){assert(randomTrie.search(w));for(std::size_t n=0;n<=w.size();++n)assert(randomTrie.startsWith(w.substr(0,n)));}
    auto copied=randomTrie;copied.insert("zzzz");assert(copied.search("zzzz"));assert(!randomTrie.search("zzzz"));
}
