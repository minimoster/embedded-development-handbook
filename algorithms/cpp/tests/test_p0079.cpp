#include "../solutions/p0079_word_search.hpp"
using namespace hot100::p0079;
int main() {
Solution s;std::vector<std::vector<char>>b{{'A','B'},{'D','C'}};auto old=b;assert(s.exist(b,"ABCD"));assert(b==old);assert(!s.exist(b,"ABA"));assert(b==old);assert(s.exist(b,""));b={};assert(!s.exist(b,"A"));assert(s.exist(b,""));b={{'A'}};assert(s.exist(b,"A"));assert(!s.exist(b,"AA"));

std::mt19937 rng(79);
for(int z=0;z<200;++z){
 b.assign(2,std::vector<char>(3));for(auto&row:b)for(auto&c:row)c=char('A'+rng()%3);
 std::string word(1+rng()%7,'A');for(auto&c:word)c=char('A'+rng()%3);
 std::function<bool(int,int,unsigned)>oracle=[&](int cell,int index,unsigned mask){
  if(mask&(1u<<cell)) return false;
  if(b[cell/3][cell%3]!=word[index]) return false;
  if(index+1==int(word.size())) return true;
  mask|=1u<<cell;
  for(int next=0;next<6;++next)if(std::abs(cell/3-next/3)+std::abs(cell%3-next%3)==1&&oracle(next,index+1,mask))return true;
  return false;
 };
 bool expected=false;for(int cell=0;cell<6;++cell)expected|=oracle(cell,0,0);
 old=b;assert(s.exist(b,word)==expected);assert(b==old);
}
}
