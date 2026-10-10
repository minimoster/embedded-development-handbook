#include "../solutions/p0017_letter_combinations_of_a_phone_number.hpp"
using namespace hot100::p0017;
int main() {
Solution s;auto r=s.letterCombinations("27");assert(r.size()==12);assert(r.front()=="ap"&&r.back()=="cs");assert(s.letterCombinations("").empty());assert(s.letterCombinations("1").empty());assert(s.letterCombinations("9").size()==4);auto again=s.letterCombinations("2");assert((again==std::vector<std::string>{"a","b","c"}));
}
