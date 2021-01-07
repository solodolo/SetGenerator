#include <stdio.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <regex>

// special symbol for grammar rule serparator
const static std::string RULE_SEP = "->";

// empty set symbol
const static std::string EPSILON = "~";

// Determines if symbol is a terminal
// TODO : Find a better spot for this
bool is_terminal(const std::string symbol) {
  return !std::regex_match(symbol, std::regex("[A-Z]"));
}

// Remove space characters in string
// TODO : Find a better spot for this
std::string remove_whitespace(const std::string& s) {
  std::string ret = "";
  for(const char& c : s) {
    if(c == ' ') {
      continue;
    }

    ret += c;
  }

  return ret;
}

class LR1Item {
  public:
    LR1Item(std::string production, std::string lookahead, int position) :
      lookahead(lookahead),
      position(position) {
        // Split production at "->" into LHS and RHS
        const auto found = production.find("->");
        if(found == std::string::npos) {
          throw std::runtime_error("Invalid LR1Item production: " + production);
        }

        // Given S -> A, found = 2
        // lhs = "S "
        // rhs = " A"
        lhs = production.substr(0, found);
        rhs = production.substr(found + 2, std::string::npos);

        // Trim leading and trailing whitespace from both strings
        lhs = remove_whitespace(lhs);
        rhs = remove_whitespace(rhs);
      };

    // Returns true if the symbol to the right of position is
    // a non-terminal
    // S -> . E returns true
    // S -> . "(" returns false
    bool next_is_non_terminal() const {
      if(position >= rhs.size()) {
        return false;
      }

      return !is_terminal(rhs.substr(position, 1));
    }

    // Gets the symbol to the right of position
    // on the RHS
    // S -> A . B returns B
    // S -> A . returns empty string
    std::string get_next_symbol() const {
      if(position < rhs.size()) {
        return rhs.substr(position, 1);
      }

      return "";
    }

    // Given item [A → α ⋅ B β, t], returns β
    // S -> A . B C returns [C]
    std::string get_beta_symbols() const {
      if(position >= rhs.size() - 1) {
        return "";
      }

      return rhs.substr(position + 1, std::string::npos);
    }

    // Gets the lookahead token
    std::string get_lookahead() const {
      return lookahead;
    }

    // Returns a string of the members that can be used
    // for hashing
    std::string get_str_for_hash() const {
      return lhs+rhs+lookahead+std::to_string(position);
    }

    // Move the position marker to the right
    // S -> A . B becomes S -> A B .
    void increment_position() {
      if(position < rhs.size()) {
        ++position;
      }
    }

    bool operator==(const LR1Item& rhs) const {
      return this->get_str_for_hash() == rhs.get_str_for_hash();
    }

    // Returns string version of this item
    // i.e S->A.B,$
    std::string to_string() const {
      std::string rhs_with_pos = rhs;
      rhs_with_pos.insert(position, ".");

      return lhs + RULE_SEP + rhs_with_pos + "," + lookahead;
    }

  private:
    // LHS of production
    // With S -> E, lhs = S
    std::string lhs;

    // RHS of production
    // With S -> E, rhs = E
    std::string rhs;

    // Terminal lookahead 
    std::string lookahead;

    // The current position of the marker
    // E.g. when production = S -> E and position = 0 then LR item =  S -> . E
    int position;
};

struct LR1ItemHash {
  // Return a hash for this item
  std::size_t operator()(const LR1Item& item) const noexcept {
    return std::hash<std::string>{}(item.get_str_for_hash());
  }
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
        first_of(grammar, symbol);
      }

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
    std::unordered_set<LR1Item, LR1ItemHash> build_closure_set(const std::vector<std::string>& grammar, const std::unordered_set<LR1Item, LR1ItemHash>& s) {
      std::queue<LR1Item> q;

      for(const LR1Item& item : s) {
        q.push(item);
      }

      std::unordered_set<LR1Item, LR1ItemHash> closure;

      // For each item [A → α ⋅ B β, t] in S
      while(!q.empty()) {
        const LR1Item& item = q.front(); q.pop();

        if(item.next_is_non_terminal()) {
          // For each production B → γ in G
          std::string B = item.get_next_symbol();
          std::vector<std::string> productions = get_symbol_productions(grammar, B);
          for(const std::string production : productions) {
            // For each token b in FIRST(βt)
            std::string beta = item.get_beta_symbols();
            std::string t = item.get_lookahead();
            
            // For each token b in FIRST(βt)
            std::unordered_set<std::string> first_tokens = first(beta+t);
            for(std::string b : first_tokens) {
              LR1Item closure_item(production, b, 0);
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
    std::unordered_set<LR1Item, LR1ItemHash> build_initial_closure(const std::vector<std::string>& grammar) {
      // Create set with initial item
      LR1Item augmented("S' -> E", "$", 0);
      std::unordered_set<LR1Item, LR1ItemHash> s;
      s.insert(augmented);

      std::unordered_set<LR1Item, LR1ItemHash> closure = build_closure_set(grammar, s);
      s.merge(closure);

      return s;
    }

  private:
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
    void first_of(const std::vector<std::string>& grammar, const std::string& symbol) {

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
          first_of(grammar, cur_symbol);

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
    std::unordered_set<std::string> first(const std::string& symbols) {
      std::unordered_set<std::string> first_set;

      bool last_had_epsilon = true;
      for(int i = 0; i < symbols.size(); ++i) {
        // Get Xi from symbols
        std::string symbol = symbols.substr(i, 1);

        // Do nothing if symbol isn't in first_sets table
        if(symbol == EPSILON || first_sets.count(symbol) == 0) {
          continue;
        }

        // Merge FIRST(symbol) which may include epsilon
        first_set.merge(first_sets[symbol]);

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

    // Given a production like A -> sSB, returns sSB
    std::string get_RHS(const std::string production) {
      const auto& found = production.find(RULE_SEP);

      if(found != std::string::npos) {
        return production.substr(found + 3); // skip "-> "
      }

      return "";
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

    // Looks up each symbol in beta + t in first_sets
    // then returns the merged set
    // Returns FIRST(βt)
    std::unordered_set<std::string> get_first_tokens(const std::string& beta, const std::string& t) {
      std::unordered_set<std::string> first_tokens;

      // Get first_set of each symbol in beta
      for(int i = 0; i < beta.size(); ++i) {
        std::string symbol = beta.substr(i, 1);
        
        if(first_sets.count(symbol) == 0) {
          continue;
        }

        first_tokens.merge(first_sets[symbol]);
      }

      // Merge first set of lookahead t
      if(first_sets.count(t) > 0) {
        first_tokens.merge(first_sets[t]);
      }

      return first_tokens;
    }
};
