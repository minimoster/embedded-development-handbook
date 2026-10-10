// Additional independent small-domain oracles. Not another problem or online AC record.
#include "../solutions/p0003_longest_substring_without_repeating_characters.hpp"
#include "../solutions/p0015_3sum.hpp"
#include "../solutions/p0041_first_missing_positive.hpp"
#include "../solutions/p0042_trapping_rain_water.hpp"
#include "../solutions/p0076_minimum_window_substring.hpp"
#include "../solutions/p0239_sliding_window_maximum.hpp"
#include "../solutions/p0438_find_all_anagrams_in_a_string.hpp"
#include "../solutions/p0039_combination_sum.hpp"
#include "../solutions/p0131_palindrome_partitioning.hpp"
#include "../solutions/p0051_n_queens.hpp"
#include "../solutions/p0075_sort_colors.hpp"
#include "../solutions/p0287_find_the_duplicate_number.hpp"
#include "../solutions/p0004_median_of_two_sorted_arrays.hpp"
#include "../solutions/p0084_largest_rectangle_in_histogram.hpp"
#include "../solutions/p0045_jump_game_ii.hpp"
#include "../solutions/p0032_longest_valid_parentheses.hpp"
#include "../solutions/p0031_next_permutation.hpp"
using std::vector;using std::string;
int power(int b,int n){int v=1;while(n--)v*=b;return v;}
vector<int> digits(int z,int n,int b,int shift=0){vector<int>a(n);for(int&x:a){x=z%b+shift;z/=b;}return a;}
int main(){
 hot100::p0003::Solution p3; hot100::p0076::Solution p76;hot100::p0438::Solution p438;
 vector<string> ss{string()};for(int n=1;n<=6;++n)for(int z=0;z<power(3,n);++z){string a(n,'a');int t=z;for(char&c:a){c=char('a'+t%3);t/=3;}ss.push_back(a);}
 for(const auto&a:ss){int want=0;for(int l=0;l<int(a.size());++l){std::set<char>s;for(int r=l;r<int(a.size());++r){if(!s.insert(a[r]).second)break;want=std::max(want,r-l+1);}}assert(p3.lengthOfLongestSubstring(a)==want);}
 for(const auto&a:ss)for(const auto&b:ss){if(b.size()>3)continue;string best;int bestlen=INT_MAX;vector<int> anagram;vector<int>need(256);for(unsigned char c:b)++need[c];
  if(!b.empty())for(int l=0;l<int(a.size());++l){vector<int>cnt(256);for(int r=l;r<int(a.size());++r){++cnt[static_cast<unsigned char>(a[r])];bool ok=true;for(int c=0;c<256;++c)if(cnt[c]<need[c])ok=false;if(ok&&r-l+1<bestlen){bestlen=r-l+1;best=a.substr(l,bestlen);}if(r-l+1==int(b.size())&&cnt==need)anagram.push_back(l);}}
  assert(p76.minWindow(a,b)==best);assert(p438.findAnagrams(a,b)==anagram);
 }
 assert(p3.lengthOfLongestSubstring(string("\x80\xFF\x80",3))==2);assert(p76.minWindow(string("\x80\xFF\x80",3),string("\xFF\x80",2))==string("\x80\xFF",2));
 hot100::p0015::Solution p15;hot100::p0041::Solution p41;hot100::p0239::Solution p239;hot100::p0031::Solution p31;
 for(int n=0;n<=7;++n)for(int z=0;z<power(5,n);++z){auto a=digits(z,n,5,-2),old=a;std::set<vector<int>>want;for(int i=0;i<n;++i)for(int j=i+1;j<n;++j)for(int k=j+1;k<n;++k)if(a[i]+a[j]+a[k]==0){vector<int>v{a[i],a[j],a[k]};std::sort(v.begin(),v.end());want.insert(v);}auto r=p15.threeSum(a);assert(std::set<vector<int>>(r.begin(),r.end())==want);assert(r.size()==want.size());a=old;int absent=1;while(std::find(a.begin(),a.end(),absent)!=a.end())++absent;assert(p41.firstMissingPositive(a)==absent);
  for(int k=1;k<=n;++k){vector<int>expected;for(int l=0;l+k<=n;++l)expected.push_back(*std::max_element(old.begin()+l,old.begin()+l+k));assert(p239.maxSlidingWindow(old,k)==expected);}
  if(n){a=old;auto next=a;std::next_permutation(next.begin(),next.end());p31.nextPermutation(a);assert(a==next);}
 }
 hot100::p0042::Solution p42;hot100::p0084::Solution p84;hot100::p0045::Solution p45;
 for(int n=0;n<=8;++n)for(int z=0;z<power(4,n);++z){auto a=digits(z,n,4);int water=0,area=0;for(int i=0;i<n;++i){int ml=0,mr=0;for(int j=0;j<=i;++j)ml=std::max(ml,a[j]);for(int j=i;j<n;++j)mr=std::max(mr,a[j]);water+=std::min(ml,mr)-a[i];int mn=a[i];for(int j=i;j<n;++j){mn=std::min(mn,a[j]);area=std::max(area,mn*(j-i+1));}}assert(p42.trap(a)==water);assert(p84.largestRectangleArea(a)==area);vector<int>d(n,1000);if(n)d[0]=0;for(int i=0;i<n;++i)for(int j=i+1;j<n&&j<=i+a[i];++j)d[j]=std::min(d[j],d[i]+1);int jumps=n<2?0:d.back()==1000?-1:d.back();assert(p45.jump(a)==jumps);}
 hot100::p0131::Solution p131;for(int n=0;n<=10;++n)for(int z=0;z<power(2,n);++z){string a(n,'a');int tmp=z;for(char&c:a){c=char('a'+tmp%2);tmp/=2;}std::set<vector<string>>want;for(int m=0;m<power(2,std::max(0,n-1));++m){vector<string>partition;int start=0;bool good=true;for(int i=0;i<n;++i)if(i==n-1||(m&(1<<i))){string sub=a.substr(start,i-start+1),reversed=sub;std::reverse(reversed.begin(),reversed.end());if(sub!=reversed)good=false;partition.push_back(sub);start=i+1;}if(good)want.insert(partition);}auto r=p131.partition(a);assert(std::set<vector<string>>(r.begin(),r.end())==want);assert(r.size()==want.size());}
 hot100::p0051::Solution p51;assert(p51.solveNQueens(9).size()==352);
 hot100::p0032::Solution p32;for(int n=0;n<=16;++n)for(int z=0;z<power(2,n);++z){string a(n,'(');int tmp=z;for(char&c:a){c=tmp%2?')':'(';tmp/=2;}int best=0;for(int l=0;l<n;++l){int balance=0;for(int r=l;r<n;++r){balance+=a[r]=='('?1:-1;if(balance<0)break;if(balance==0)best=std::max(best,r-l+1);}}assert(p32.longestValidParentheses(a)==best);}
 hot100::p0004::Solution p4;vector<vector<int>>sorted;for(int n=0;n<=5;++n)for(int z=0;z<power(3,n);++z){auto a=digits(z,n,3,-1);if(std::is_sorted(a.begin(),a.end()))sorted.push_back(a);}for(auto&a:sorted)for(auto&b:sorted){if(a.empty()&&b.empty())continue;auto all=a;all.insert(all.end(),b.begin(),b.end());std::sort(all.begin(),all.end());double expected=all.size()%2?all[all.size()/2]:(all[all.size()/2-1]+all[all.size()/2])/2.0;assert(p4.findMedianSortedArrays(a,b)==expected);}
 hot100::p0287::Solution p287;for(int n=1;n<=6;++n)for(int z=0;z<power(n,n+1);++z){auto a=digits(z,n+1,n,1);vector<int>cnt(n+1);for(int x:a)++cnt[x];int dupe=0,multiple=0;for(int i=1;i<=n;++i)if(cnt[i]>1){dupe=i;++multiple;}if(multiple==1){auto orig=a;assert(p287.findDuplicate(a)==dupe);assert(a==orig);}}

 hot100::p0075::Solution p75;for(int n=0;n<=9;++n)for(int z=0;z<power(3,n);++z){auto a=digits(z,n,3),expected=a;std::sort(expected.begin(),expected.end());p75.sortColors(a);assert(a==expected);}
 hot100::p0039::Solution p39;for(int mask=1;mask<32;++mask){vector<int>a;for(int x=2;x<=6;++x)if(mask&(1<<(x-2)))a.push_back(x);for(int t=0;t<=22;++t){int radix=t/a[0]+1;std::set<vector<int>>expected;for(int z=0;z<power(radix,int(a.size()));++z){auto cnt=digits(z,int(a.size()),radix);int total=0;vector<int>comb;for(int i=0;i<int(a.size());++i){total+=a[i]*cnt[i];comb.insert(comb.end(),cnt[i],a[i]);}if(total==t)expected.insert(comb);}auto result=p39.combinationSum(a,t);assert(std::set<vector<int>>(result.begin(),result.end())==expected);assert(result.size()==expected.size());}}
 std::cout<<"Independent exhaustive cases passed\n";
}
