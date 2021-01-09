#ifndef _PARSE_TABLE_GENERATOR_HPP_
#define _PARSE_TABLE_GENERATOR_HPP_

#include "grammar.hpp"
#include "lr1_item.hpp"
#include "set_generator.hpp"

// Table action for accepting the parse
const static std::string ACCEPT_ACTION = "accept";

// Table action for shifting to state i
const static std::string SHIFT_ACTION = "s";

// Table action for reducing symbols by production j
const static std::string REDUCE_ACTION = "r";

class LR1ParserTableGenerator {
  public:
    LR1ParserTableGenerator(const std::vector<std::string>& grammar) : grammar(grammar), set_generator(grammar) {
      std::unordered_map<std::string, std::unordered_set<std::string>> first_sets = set_generator.build_first_sets();
      for(auto it = first_sets.begin(); it != first_sets.end(); ++it) {
        if((*it).first == AUGMENTED_LHS) {
          continue;
        }
        all_symbols.push_back((*it).first);
      }

      // Get list of terminals and non-terminals to fill out
      // indices of symbol_cols
      build_symbol_cols(get_terminals(all_symbols), get_nonterminals(all_symbols));
    };

    /**
     * Builds the Action and GOTO LR(1) tables for the provided grammar.
     * 
     * Given N terminals and M non-terminals
     * in a given table row, cols 0...N-1 are the actions and N...M-1 are
     * the gotos
     * 
     * Algorithm from Dragon book 4.7.3
     * 1. Construct C' = {I0, I1, I2,...,In}, the item sets for the grammar
     * 2. Calculate action table state i from set Ii with the following rules
     * 2a. If [A → α ⋅ a B β, t] is in Ii and GOTO(Ii, a) = Ij and a is a terminal,
     *      set table[i, symbol_cols[a]] = shift j.
     * 2b. If [A → α ⋅, t] is in Ii and A != S', set table[i, symbol_cols[t]] = reduce A → α
     * 2c. If [S' -> S ⋅, $] is in Ii, set table[i, symbol_cols[$]] = accept
     * 3. If [A → α ⋅ A β, t] in Ii and A is a non-terminal and GOTO(Ii, A) = Ij,
     *      set table[i, symbol_cols[A]] = j
     * 4. Entires not filled in by above rules are errors
     */ 
    std::vector<std::vector<std::string>> build_parse_table() {
      std::set<std::set<LR1Item, LR1Comparator>, LR1SetComparator> item_sets = set_generator.build_item_sets();
      const std::unordered_map<std::string, int>& goto_indices = set_generator.get_goto_indices();

      int state = 0;
      for(const auto& item_set : item_sets) {
        // Add state row if needed
        if(state >= table.size()) {
          std::vector<std::string> row(symbol_cols.size());
          table.push_back(row);
        }
        // item_set == Ii
        for(const auto& item : item_set) {
          // item is of the form [A → α ⋅ a B β, t]

          // If item is [A → α ⋅ a β, t], next_symbol == a
          std::string next_symbol = item.get_next_symbol();

          // The marker is to the right of the rhs
          // item is [A → α ⋅, t] or [S' -> S ⋅, $]
          if(next_symbol.empty()) {
            if(item.is_augmented_production()) { //[S' -> S ⋅, $]
              table[state][symbol_cols[DOLLAR]] = ACCEPT_ACTION;
            }
            else {
              table[state][symbol_cols[item.get_lookahead()]] = REDUCE_ACTION + grammar[item.get_production_num()];
            }
          }
          else {
            // item is [A → α ⋅ a B β, t] or [A → α ⋅ A B β, t]
            std::string goto_key = std::to_string(state) + "," + next_symbol;
            if(goto_indices.count(goto_key) == 0) {
              throw std::runtime_error(goto_key + " not found in goto_indices map.");
            }

            // Given GOTO(Ii, a) = Ij, goto_indices maps "i,a" => j
            std::string j = std::to_string(goto_indices.at(goto_key));
            if(is_terminal(next_symbol)) {
              table[state][symbol_cols[next_symbol]] = SHIFT_ACTION + j;
            }
            else {
              table[state][symbol_cols[next_symbol]] = j;
            }
          }
        }
        
        ++state;
      }

      return table;
    };

    const std::vector<std::string>& get_symbol_cols() {
      return all_symbols;
    }

  private:
    const std::vector<std::string> grammar;
    SetGenerator set_generator;

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
    std::unordered_map<std::string, int> symbol_cols;

    // All symbols in left-right order in table
    std::vector<std::string> all_symbols;

    // Initialize symbol_cols by mapping grammar symbols to their column indices in table
    void build_symbol_cols(const std::unordered_set<std::string>& terminals, const std::unordered_set<std::string>& non_terminals) {
      // Terminals will occupy 0...n - 1 table cols (action table)
      int n = 0;
      for(auto it = terminals.begin(); it != terminals.end(); ++it) {
        symbol_cols[(*it)] = n;
        ++n;
      }

      // Non-terminals will occupy n...m - 1 table cols (goto table)
      int m = 0;
      for(auto it = non_terminals.begin(); it != non_terminals.end(); ++it) {
        symbol_cols[(*it)] = m + n;
        ++m;
      }
    }
};

#endif /* _PARSE_TABLE_GENERATOR_HPP_ */