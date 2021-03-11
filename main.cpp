#include <stdio.h>
#include <iostream>
#include <fstream>
#include <set>
#include "parse_table_generator.hpp"

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
  {"B -> C"},
  {"C -> D"},
  {"C -> E"},
  {"D -> '{{' F '}}'"},
  {"D -> '{{' V '}}'"},
  {"D -> '{{' F '-}'"},
  {"D -> '{{' V '-}'"},
  {"E -> '{{:' F '}}'"},
  {"Y -> 'ID' '.' Y"},
  {"Y -> 'ID'"},
  {"H -> 'ID' '(' I ')'"},
  {"I -> J"},
  {"I -> ~"},
  {"J -> J ',' K"},
  {"J -> K"},
  {"K -> Y '=' K"},
  {"K -> M"},
  {"M -> M 'LOGIC_OP' U"},
  {"M -> U"},
  {"U -> U 'REL_OP' N"},
  {"U -> N"},
  {"N -> N '+' O"},
  {"N -> N '-' O"},
  {"N -> O"},
  {"O -> O 'MULT_OP' L"},
  {"O -> L"},
  {"L -> '!' L"},
  {"L -> '-' L"},
  {"L -> P"},
  {"P -> 'STRING'"},
  {"P -> 'NUM'"},
  {"P -> 'BOOL'"},
  {"P -> Y"},
  {"P -> '(' K ')'"},
  {"Q -> W S T 'END'"},
  {"W -> 'IF' '(' K ')' V"},
  {"S -> X"},
  {"S -> ~"},
  {"X -> X 'ELSE_IF' '(' K ')' V"},
  {"X -> 'ELSE_IF' '(' K ')' V"},
  {"T -> 'ELSE' V"},
  {"T -> ~"},
  {"R -> 'FOR' '(' 'ID' 'IN' 'STRING' ')' V 'END'"},
  {"R -> 'FOR' '(' 'ID' 'IN' Y ')' V 'END'"},
  {"R -> 'FOR' '(' 'ID' 'IN' H ')' V 'END'"},
  {"F -> K"},
  {"F -> H"},
  {"F -> Q"},
  {"F -> R"},
  {"V -> G"},
  {"V -> ~"},
  {"G -> G F ';'"},
  {"G -> F ';'"},
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

const static std::vector<std::string> G7 {
  {"S -> C C"},
  {"C -> c C"},
  {"C -> d"}
};

const static std::vector<std::string> G8 {
  {"S -> A a A b"},
  {"S -> B b B a"},
  {"A -> ~"},
  {"B -> ~"}
};

const static std::vector<std::string> G9 {
  {"S -> S ; A"},
  {"S -> A"},
  {"A -> E"},
  {"A -> i = E"},
  {"E -> E + i"},
  {"E -> i"}
};

const static std::vector<std::string> G10 {
  {"S -> h B e"},
  {"B -> B A"},
  {"B -> ~"},
  {"A -> x"},
  {"A -> t"}
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

void print_item_sets(const std::set<std::set<LR1Item, LR1Comparator>, LR1SetComparator>& item_sets) {
  std::cout << "Item Sets: \n";
  int set_num = 0;

  for(const auto& item_set : item_sets) {
    std::cout << "  Set " << set_num << ":\n";
    for(const auto& item : item_set) {
      std::cout << "    " << item.to_string() << "\n";
    }
    std::cout << "\n";
    set_num++;
  }
  std::cout << "\n\n";
}

void print_parse_table(const std::vector<std::vector<std::string>>& parse_table, const std::vector<std::string>& symbols) {
  // print symbols
  printf("|%5s%5s", "state", "");
  for(const auto& symbol : symbols) {
    printf("|%5s%5s", symbol.c_str(), "");
  }
  printf("|\n");

  for(int i = 0; i < parse_table.size(); ++i) {
    printf("|%5d%5s", i, "");
    for(const auto& action : parse_table[i]) {
      printf("|%5s%5s", action.c_str(), "");
    }
    printf("|\n");
  }
}

// Prints the parse table where each row is like
// {"si", "", "sj", "ri", "", "", "n"}
void print_go_parse_table(const std::vector<std::vector<std::string>>& parse_table, const std::vector<std::string>& symbols) {
  printf("{");
  for(int i = 0; i < symbols.size(); ++i) {
    const auto& symbol = symbols[i];
    printf("\"%s\"", symbol.c_str());

    if(i < symbols.size() - 1) {
      printf(", ");
    }
  }

  printf("}\n");
  for(int i = 0; i < parse_table.size(); ++i) {
    printf("{");
    int row_size = parse_table[i].size();
    for(int j = 0; j < row_size; ++j) {
      const std::string& action = parse_table[i][j];

      printf("\"%s\"", action.c_str());

      if(j < row_size - 1) {
        printf(", ");
      }
    }

    printf("},\n");
  }
}

int write_table(const std::string out_path, const std::vector<std::vector<std::string>>& parse_table, const std::vector<std::string>& symbols) {
  std::ofstream out_stream(out_path);

  if(!out_stream) {
    std::cerr << "failed to open output stream " << out_path << ": " << strerror(errno) << "\n";
    return 1;
  }

  // write the symbol header
  for(int i = 0; i < symbols.size(); ++i) {
    const auto& symbol = symbols[i];
    out_stream << symbol;

    if(i < symbols.size() - 1) {
      out_stream << ",";
    }
  }

  out_stream << "\n";

  for(int i = 0; i < parse_table.size(); ++i) {
    int row_size = parse_table[i].size();
    for(int j = 0; j < row_size; ++j) {
      const std::string& action = parse_table[i][j];
      out_stream << action;

      if(j < row_size - 1) {
        out_stream << ", ";
      }
    }

    out_stream << "\n";
  }

  out_stream.close();
  return 0;
}

void print_usage() {
  std::cout << "usage: set_generator <path/to/table/output>\n";
}

int main(int argc, char **argv) {
  if(argc < 2) {
    print_usage();
    exit(0);
  }

  Grammar grammar(G4);
  grammar.add_augmented_production();

  LR1ParserTableGenerator generator(grammar);

  std::cout << "generating parse table\n";

  std::vector<std::vector<std::string>> parse_table = generator.build_parse_table();

  std::cout << "generating table symbols\n";
  std::vector<std::string> symbols = generator.get_table_columns();

  std::string output_path(argv[1]);

  std::cout << "writing table output to " << output_path << "\n";

  int write_result = write_table(output_path, parse_table, symbols);
  if(write_result == 0) {
    std::cout << "table successfully written\n";
  }

  return write_result;
}