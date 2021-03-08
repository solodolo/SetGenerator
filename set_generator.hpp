#ifndef _SET_GENERATOR_HPP_
#define _SET_GENERATOR_HPP_

#include <stdio.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>

#include "lr1_item.hpp"
#include "grammar.hpp"

/**
 * Generates LR(1) first, goto, closure, and item sets for a grammar
 * 
 */ 
class SetGenerator {
  public:
    SetGenerator(Grammar grammar) : grammar(grammar) {}

    // Calculates the first sets for each symbol in the grammar
    // Returns a map of symbol => { '(', '+', ...}
    std::unordered_map<std::string, std::unordered_set<std::string>> build_first_sets() {
      // Remove any previous sets
      first_sets.clear();

      for(const auto& symbol : grammar.get_all_symbols()) {
        first_of(symbol);
      }

      // Make sure we add a first set for our EOF symbol
      first_sets[DOLLAR] = { DOLLAR };

      // We don't need empty set key in firsts sets
      first_sets.erase(EPSILON);

      return first_sets;
    }

    /**
     * Builds the closure set for the items in s
     * 
     * closure(S)
     * For each item [A → α ⋅ B β, t] in S,
     *   For each production B → γ in G,
     *     For each token b in FIRST(βt),
     *       Add [B → ⋅ γ, b] to S
     */
    std::set<LR1Item, LR1Comparator> build_closure_set(const std::set<LR1Item, LR1Comparator>& s) {
      std::queue<LR1Item> q;

      for(LR1Item item : s) {
        q.push(item);
      }

      std::set<LR1Item, LR1Comparator> closure;

      // For each item [A → α ⋅ B β, t] in S
      while(!q.empty()) {
        const LR1Item item = q.front(); 
        q.pop();

        if(item.next_is_non_terminal()) {
          std::string B = item.get_next_symbol();
          std::vector<std::string> beta_t = item.get_beta_symbols();
          std::string t = item.get_lookahead();
          beta_t.push_back(t);

          std::vector<int> production_indices = get_production_indices(B);
          std::unordered_set<std::string> first_tokens = first(beta_t); // FIRST(βt)
          for(int pi : production_indices) { // For each production B → γ in G
            const std::string& production = grammar[pi];

            // For each token b in FIRST(βt)
            for(std::string b : first_tokens) {
              LR1Item closure_item(production, pi, b, 0);
              // Add [B → ⋅ γ, b] to S
              auto result = closure.insert(closure_item);

              if(result.second) {
                q.push(closure_item);
              }
            }
          }
        }
      }

      return closure;
    }

    // Augments grammar then builds closure from the augmented item
    std::set<LR1Item, LR1Comparator> build_initial_closure() {
      // Create set with augmented item
      std::set<LR1Item, LR1Comparator> s;
      s.insert(LR1Item(grammar[0], 0, DOLLAR, 0));

      // Build closure from augmented item
      std::set<LR1Item, LR1Comparator> closure = build_closure_set(s);
      s.merge(closure);

      return s;
    }

    /**
     * Returns the closure of the set of all items [A → α X ⋅ β, t] such that
     * [A → α ⋅ X β, t] is in item_set and where X == symbol
     * Each of these items is part of the kernel set
     * 
     * psudocode from Dragon book 4.7.2
     * GOTO(I,X)
     *  init J to be the empty set
     *  for each item [A → α ⋅ X β, t] in I
     *    add item [A → α X ⋅ β, t] to J
     *  return closure(J)
     * 
     */ 
    std::set<LR1Item, LR1Comparator> build_goto(const std::set<LR1Item, LR1Comparator>& item_set, const std::string& symbol) {
      // init j to be empty set
      std::set<LR1Item, LR1Comparator> j = get_kernel_items(item_set, symbol);
      std::set<LR1Item, LR1Comparator> closure = build_closure_set(j);
      j.merge(closure);
      return j;
    }

    /**
     * Builds all item sets for the augmented grammar
     *
     * psudocode from Dragon book 4.7.2
     * 
     * ITEMS(G)
     *  init C to {closure(augmented_item)}
     *  repeat until no items are added to C
     *    for each set I in C
     *      for each grammar symbol X
     *        if GOTO(I,X) not empty and not in C
     *          add GOTO(I,X) to C
     */
    std::set<std::set<LR1Item, LR1Comparator>, LR1SetComparator> build_item_sets() {
      // init c to {closure(augmented_item)}
      std::set<LR1Item, LR1Comparator> i0 = build_initial_closure();
      item_sets = {i0};

      // Track the gotos we have already done so we don't
      // duplicate sets in c
      //
      // If all items in the kernel set exist in completed_gotos
      // skip GOTO(I,X)
      // Otherwise
      // add them and compute GOTO(I,X)
      std::set<LR1Item, LR1Comparator> completed_gotos;

      int prev_size = 0;
      while(true) {
        // The current set number
        int i = 0;

        // for each set i in c
        for(const std::set<LR1Item, LR1Comparator>& Ii : item_sets) {
          // for each grammar symbol X
          for(auto it = first_sets.begin(); it != first_sets.end(); ++it) {
            std::string x = (*it).first;
            std::set<LR1Item, LR1Comparator> gotos = build_goto(Ii, x);
            // if GOTO(I,X) not empty
            if(!gotos.empty()) {
              // The goto mapping key
              std::string goto_key = std::to_string(i) + "," + x;
              // add GOTO(I,X) to c
              // c is a set so GOTO will only be added
              // if it is not in c already
              auto result = item_sets.insert(gotos);
              if(result.second) { // gotos wasn't in c
                goto_indices[goto_key] = item_sets.size() - 1;
              }
              else { // gotos was in c
                goto_indices[goto_key] = std::distance(item_sets.begin(), result.first);
              }
            }
          }

          ++i;
        }

        // Break if c hasn't changed
        if(prev_size == item_sets.size()) {
          break;
        }
        prev_size = item_sets.size();
      }

      return item_sets;
    }

    // Return the cached goto_indices
    const std::unordered_map<std::string, int>& get_goto_indices() {
      return goto_indices;
    }

    // Return the cached item_sets
    const std::set<std::set<LR1Item, LR1Comparator>, LR1SetComparator>& get_item_sets() {
      return item_sets;
    }

  private:
    // The provided grammar
    Grammar grammar;

    // Holds the FIRST(X) sets for each grammar item X
    std::unordered_map<std::string, std::unordered_set<std::string>> first_sets;

    // Holds the item sets calculated in build_item_sets
    std::set<std::set<LR1Item, LR1Comparator>, LR1SetComparator> item_sets;

    // Holds mappings of the form "<input set index>,<input symbol>" => "<output set index>"
    std::unordered_map<std::string, int> goto_indices;

  private:
    /**
     * Calculates the first set of the given symbol
     * 
     * Rules for First Sets
     *
     * - If X is a terminal then First(X) is just X!
     * - If there is a Production X → ε then add ε to first(X)
     * - If there is a Production X → Y1Y2..Yk then add first(Y1Y2..Yk) to first(X)
     * - First(Y1Y2..Yk) is either
     *     - First(Y1) (if First(Y1) doesn't contain ε)
     *     - OR (if First(Y1) does contain ε) then First (Y1Y2..Yk) is everything
     *       in First(Y1) <except for ε > as well as everything in First(Y2..Yk)
     *     - If First(Y1) First(Y2)..First(Yk) all contain ε then add ε
     *       to First(Y1Y2..Yk) as well.
     */
    void first_of(const std::string& symbol) {

      if(first_sets.count(symbol) > 0) {
        return;
      }

      first_sets[symbol] = {};

      // first(X) is just X
      if(is_terminal(symbol) || symbol == EPSILON || symbol == DOLLAR) {
        first_sets[symbol].insert(symbol);
        return;
      }

      std::vector<int> production_indices = get_production_indices(symbol);
      for(int pi : production_indices) {
        const std::string& production = grammar[pi];
        std::vector<std::string> rhs = Grammar::extract_symbols(get_RHS(production));

        // If there is a Production X → ε then add ε to first(X)
        if(rhs.size() == 1 && rhs[0] == EPSILON) {
          first_sets[symbol].insert(EPSILON);
        }

        bool all_contain_epsilon = true;
        for(const std::string& cur_symbol : rhs) {
          if(cur_symbol == " ") {
            continue;
          }
          first_of(cur_symbol);

          std::unordered_set<std::string> y = first_sets[cur_symbol];

          // first(Y1Y2..Yk) = first(Y1) (if first(Y1) doesn't contain ε)
          if(y.count(EPSILON) == 0) {
            all_contain_epsilon = false;
            first_sets[symbol].insert(y.begin(), y.end());
            break;
          }
          else {
            // first(Y1) does contain ε then first(Y1Y2..Yk) is everything
            // in first(Y1) <except for ε > as well as everything in first(Y2..Yk)
            y.erase(EPSILON);
            first_sets[symbol].insert(y.begin(), y.end());
          }
        }

        // If First(Y1) First(Y2)..First(Yk) all contain ε then add ε
        // to First(Y1Y2..Yk) as well
        if(all_contain_epsilon) {
          first_sets[symbol].insert(EPSILON);
        }
      }
    }

    /**
     * Compute the first set for a string of symbols
     * 
     * Rules taken from Dragon 4.4.2:
     * 
     * Add to FIRST(X1X2...Xn) all non-epsilon symbols of FIRST(X1)
     * Also add all non-epsilon symbols of FIRST(X2) if epsilon in FIRST(X1),
     * the non-epsilon symbols of FIRST(X3) if epsilon in FIRST(X1) and FIRST(X2).
     * Finally add epsilon to FIRST(X1X2...Xn) if epsilon in FIRST(Xi), 1 <= i <= n
     */
    std::unordered_set<std::string> first(const std::vector<std::string>& symbols) {
      std::unordered_set<std::string> first_set;

      bool last_had_epsilon = true;
      for(const std::string& symbol : symbols) {
        // Do nothing if symbol isn't in first_sets table
        if(symbol == EPSILON || first_sets.count(symbol) == 0) {
          continue;
        }

        // Merge FIRST(symbol) which may include epsilon
        std::unordered_set<std::string> symbol_first_set = first_sets[symbol];
        first_set.merge(symbol_first_set);

        if(first_set.count(EPSILON) > 0) {
          // Remove epsilon and continue to next symbol
          first_set.erase(EPSILON);
        }
        else {
          // Epsilon not in FIRST(symbol) so we are done
          last_had_epsilon = false;
          break;
        }
      }

      // epsilon in FIRST(Xi), 1 <= i <= n so add it to to FIRST(X1X2...Xn)
      if(last_had_epsilon) {
        first_set.insert(EPSILON);
      }

      return first_set;
    }

    /**
      * Returns the productions of symbol
      * With grammar
      * 
      * A -> B
      * A -> d
      * B -> e
      * 
      * when given A will return ['A -> B', 'A -> d']
      */
    std::vector<int> get_production_indices(const std::string& symbol) {
      std::vector<int> ret;

      for(int i = 0; i < grammar.size(); ++i) {
        const std::string& production = grammar[i];

        if(get_LHS(production) == symbol) {
          ret.push_back(i);
        }
      }

      return ret;
    }

    /**
     * Gets a set of kernel items
     * From a set containing items like [A → α ⋅ X β, t],
     * the kernel items are those of the form [A → α X ⋅ β, t]
     * where symbol == x
     * 
     * They are the items in the goto set before the closure items are added
     * 
     */
    std::set<LR1Item, LR1Comparator> get_kernel_items(const std::set<LR1Item, LR1Comparator>& item_set, const std::string& symbol) {
      std::set<LR1Item, LR1Comparator> kernel_items;

      // for each item in I
      for(const LR1Item& item : item_set) {
        // check that the item's next symbol == X
        if(item.get_next_symbol() == symbol) {
          LR1Item next_item(item);
          // add item [A → α X ⋅ β, t] to j
          next_item.increment_position();
          kernel_items.insert(next_item);
        }
      }

      return kernel_items;
    }
};

#endif /*  _SET_GENERATOR_HPP_ */
