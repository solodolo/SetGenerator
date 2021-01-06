#include <stdio.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <regex>

class LR1Item {
  public:
    LR1Item(std::string production, std::string lookahead, int position) :
      production(production),
      lookahead(lookahead),
      position(position) {};

  private:
    // Production without lookahead like S -> E
    std::string production;

    // Terminal lookahead 
    std::string lookahead;

    // The current position of the marker
    // E.g. when production = S -> E and position = 0 then LR item =  S -> . E
    int position;
};

class SetGenerator {
  public:
    // Calculates the first sets for each symbol in the grammar
    // Returns a map of symbol => { '(', '+', ...}
    std::unordered_map<std::string, std::unordered_set<std::string>> build_first_sets(const std::vector<std::string>& grammar) {
      // Remove any previous sets
      first_sets.clear();

      for(const std::string& production : grammar) {
        const std::string symbol = production.substr(0, 1);
        first(grammar, symbol);
      }

      // We don't need empty set key in firsts sets
      first_sets.erase(EPSILON);

      return first_sets;
    }

  private:
    // special symbol for grammar rule serparator
    const std::string RULE_SEP = "->";

    // empty set symbol
    const std::string EPSILON = "~";

    std::unordered_map<std::string, std::unordered_set<std::string>> first_sets;

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
    void first(const std::vector<std::string>& grammar, const std::string& symbol) {

      if(first_sets.count(symbol) > 0) {
        return;
      }

      first_sets[symbol] = {};

      // first(X) is just X
      if(is_terminal(symbol) || symbol == EPSILON) {
        first_sets[symbol].insert(symbol);
        return;
      }

      std::vector<std::string> productions = get_symbol_productions(grammar, symbol);
      for(const std::string& production : productions) {
        std::string rhs = get_RHS(production);

        // If there is a Production X → ε then add ε to first(X)
        if(rhs == EPSILON) {
          first_sets[symbol].insert(EPSILON);
        }

        bool all_contain_epsilon = true;
        for(int i = 0; i < rhs.size(); ++i) {
          const std::string cur_symbol = rhs.substr(i, 1);
          if(cur_symbol == " ") {
            continue;
          }
          first(grammar, cur_symbol);

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

    // Given a production like A -> sSB, returns sSB
    std::string get_RHS(const std::string production) {
      const auto& found = production.find(RULE_SEP);

      if(found != std::string::npos) {
        return production.substr(found + 3); // skip "-> "
      }

      return "";
    }

    // Determines if symbol is a terminal
    bool is_terminal(const std::string symbol) {
      return !std::regex_match(symbol, std::regex("[A-Z]"));
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
    std::vector<std::string> get_symbol_productions(const std::vector<std::string>& grammar, const std::string& symbol) {
      std::vector<std::string> ret;

      for(const std::string& s : grammar) {
        if(s.substr(0, 1) == symbol) {
          ret.push_back(s);
        }
      }

      return ret;
    }
};
