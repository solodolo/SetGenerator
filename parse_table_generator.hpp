#ifndef _PARSE_TABLE_GENERATOR_HPP_
#define _PARSE_TABLE_GENERATOR_HPP_

#include "lr1_item.hpp"
#include "set_generator.hpp"

class LR1ParseTableGenerator {
  public:
    LR1ParseTableGenerator(const std::vector<std::string>& grammar) : grammar(grammar), set_generator(grammar) {
      std::unordered_map<std::string, std::unordered_set<std::string>> first_sets = set_generator.build_first_sets();
      std::vector<std::string> all_symbols;
      for(auto it = first_sets.begin(); it != first_sets.end(); ++it) {
        all_symbols.push_back((*it).first);
      }

      // Get list of terminals and non-terminals to fill out
      // indices of symbol_map
      std::unordered_set<std::string> terminals = get_terminals(all_symbols);
      std::unordered_set<std::string> non_terminals = get_nonterminals(all_symbols);

      // Terminals will occupy 0...n - 1 table cols
      int n = 0;
      for(auto it = terminals.begin(); it != terminals.end(); ++it) {
        symbol_map[(*it)] = n;
        ++n;
      }

      // Non-terminals will occupy n...m - 1 table cols
      int m = 0;
      for(auto it = non_terminals.begin(); it != non_terminals.end(); ++it) {
        symbol_map[(*it)] = m + n;
        ++m;
      }
    };

    /** 
     * The 2D parse table
     * 
     * Row indices represent the states.
     * 
     * Column indices are translated from the symbol mapping.
     * 
     * If there are n terminal symbols in the grammar
     * cols [0...n) represent the action table (either sN, rN, or accept).
     * 
     * If there are m non-terminals in the grammar
     * cols [n...n+m) represent the goto table.
     */
    std::vector<std::vector<std::string>> table;

    // Maps symbols to their column indices in table.
    std::unordered_map<std::string, int> symbol_map;
  private:
    const std::vector<std::string> grammar;
    SetGenerator set_generator;
};

#endif /* _PARSE_TABLE_GENERATOR_HPP_ */