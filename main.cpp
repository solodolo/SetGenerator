#include <stdio.h>
#include <iostream>
#include "set_generator.hpp"

// Grammar 1
const static std::vector<std::string> G1 {
  {"S -> F"},
  {"S -> (S + F)"},
  {"F -> a"}
};

const static std::vector<std::string> G2 { 
  {"E -> TX"},
  {"X -> +TX"},
  {"X -> ~"},
  {"T -> FY"},
  {"Y -> *FY"},
  {"Y -> ~"},
  {"F -> a"},
  {"F -> (E)"}
};

const static std::vector<std::string> G3 {
  {"S -> Ac"},
  {"S -> Bb"},
  {"S -> SCab"},
  {"A -> a"},
  {"A -> bB"},
  {"A -> C"},
  {"B -> AC"},
  {"B -> d"},
  {"C -> aS"},
  {"C -> CdBd"},
  {"C -> AB"},
  {"C -> ~"}
};

const static std::vector<std::string> G4 {
  {"A -> B"},
  {"B -> C"},
  {"C -> D"},
  {"C -> E"},
  {"D -> aFb"},
  {"D -> aGb"},
  {"E -> cFb"},
  {"H -> deIf"},
  {"I -> J"},
  {"I -> ~"},
  {"J -> JuK"},
  {"J -> K"},
  {"K -> gvK"},
  {"K -> L"},
  {"L -> hL"},
  {"L -> M"},
  {"M -> MiN"},
  {"M -> N"},
  {"N -> NjO"},
  {"N -> O"},
  {"O -> OkP"},
  {"O -> P"},
  {"P -> g"},
  {"P -> l"},
  {"P -> m"},
  {"P -> eKf"},
  {"Q -> neKfGSTq"},
  {"S -> oeKfG"},
  {"S -> ~"},
  {"T -> pG"},
  {"T -> ~"},
  {"R -> relfGq"},
  {"R -> regfGq"},
  {"R -> reHGq"},
  {"F -> K"},
  {"F -> H"},
  {"G -> GFt"},
  {"G -> ~"}
};

const static std::vector<std::string> G5 {
  {"E -> T X"},
  {"T -> ( E )"},
  {"T -> a Y"},
  {"X -> + E"},
  {"X -> ~"},
  {"Y -> * T E &"},
  {"T -> ~"}
};

const static std::vector<std::string> G6 {
  {"E -> T R"},
  {"R -> ~"},
  {"R -> + E"},
  {"T -> F S"},
  {"S -> ~"},
  {"S -> * T"},
  {"F -> n"},
  {"F -> ( E )"}
};

void print_first_sets(std::unordered_map<std::string, std::unordered_set<std::string>> first_sets) {
  for(const auto& a : first_sets) {
    std::cout << a.first << " : " << "[";
    
    std::string set_str = "";
    std::vector<std::string> first_set(a.second.begin(), a.second.end());
    sort(first_set.begin(), first_set.end());

    for(auto it = first_set.begin(); it != first_set.end(); ++it) {
      set_str += (*it + ", ");
    }

    set_str = set_str.substr(0, set_str.size() - 2);
    std::cout << set_str << "]\n";
  }
  std::cout << "\n";
}

int main() {
  SetGenerator generator;

  print_first_sets(generator.build_first_sets(G1));
  print_first_sets(generator.build_first_sets(G2));
  print_first_sets(generator.build_first_sets(G3));
  print_first_sets(generator.build_first_sets(G6));
}