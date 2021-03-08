#ifndef _GRAMMAR_HPP_
#define _GRAMMAR_HPP_

#include <regex>
#include <string>
#include <sstream>
#include <unordered_set>

// special symbol for grammar rule serparator
const static std::string RULE_SEP = "->";

// empty set symbol
const static std::string EPSILON = "~";

// EOF symbol
const static std::string DOLLAR = "$";

// The lhs to use for the augmented grammar rule
// i.e. the S' in S' -> S
const static std::string AUGMENTED_LHS = "S'";

// Determines if symbol is a terminal
// TODO : Find a better spot for this
bool is_terminal(const std::string symbol) {
  // return !std::regex_match(symbol, std::regex("[A-Z]"));
  return std::regex_match(symbol, std::regex("'[^[:space:]]+'"));
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

// Given a production like A -> sSB, returns sSB
std::string get_RHS(const std::string production) {
  const auto& found = production.find(RULE_SEP);

  if(found != std::string::npos) {
    return production.substr(found + 2); // skip "->"
  }

  return "";
}

// Given production A -> sSB, returns A
std::string get_LHS(const std::string production) {
  const auto& found = production.find(RULE_SEP);

  if(found != std::string::npos) {
    return remove_whitespace(production.substr(0, found));
  }

  return "";
}

/**
 *  A class to store and extract grammar information
 * 
 * Takes a vector of strings like S -> Ab, representing
 * the production rules of the grammar
 * 
 * Wraps std::vector<std::string>
 */
class Grammar {
  public:
    Grammar(const std::vector<std::string>& g) : productions(g) {
      // Extract the terminals and non-terminals from g
      for(const auto& production : g) {
        // LHS should be a single non-terminal
        std::string lhs = get_LHS(production);
        non_terminals.insert(lhs);
        all_symbols.insert(lhs);

        // Get rhs symbols and store them
        std::vector<std::string> rhs_symbols = Grammar::extract_symbols(get_RHS(production));
        for(const auto& rh_symbol : rhs_symbols) {
          // Skip empty set symbol since it is just a placeholder
          if(rh_symbol == EPSILON) {
            continue;
          }
          is_terminal(rh_symbol) ? terminals.insert(rh_symbol) : non_terminals.insert(rh_symbol);
          all_symbols.insert(rh_symbol);
        }
      }
    };

    // Return the const iterator to the underlying productions
    std::vector<std::string>::const_iterator begin() {
      return productions.cbegin();
    }

    // Return the const iterator to the underlying productions
    std::vector<std::string>::const_iterator end() {
      return productions.cend();
    }

    const std::set<std::string>& get_all_symbols() {
      return all_symbols;
    }

    const std::set<std::string>& get_terminals() {
      return terminals;
    }

    const std::set<std::string>& get_non_terminals() {
      return non_terminals;
    }

    const std::string& at(const size_t n) const {
      return productions[n];
    }

    // allow bracket access to productions
    const std::string& operator[](size_t n) {
      return productions[n];
    }

    size_t size() {
      return productions.size();
    }

    bool empty() {
      return productions.empty();
    }

    /** 
     * Inserts a new augmented production into this grammar
     * Does nothing if the grammar has already been augmented
     * 
     * Given grammar[0] as S -> E, will add S' -> S to productions
     * and will add S' to non_terminals and all_symbols
     * 
     * Will also add EOF to terminal symbols
     */
    void add_augmented_production() {
      if(is_augmented()) {
        return;
      }

      productions.insert(productions.begin(), get_augmented_production());
      all_symbols.insert(AUGMENTED_LHS);
      non_terminals.insert(AUGMENTED_LHS);
      terminals.insert(DOLLAR);
    }

    /** 
     * Returns a set containing all the symbols in str
     * Assumes symbols are space deliminated
     * 
     * Given "a B S '+'", returns {a,B,S,+}
     * Given "A a '{{' c", returns {A,a,{{,c}
     * 
     * Assumes symbols are length 1. May or may not
     * be seperated by spaces.
     */
    static std::vector<std::string> extract_symbols(std::string str) {
      std::vector<std::string> symbols;
      std::string piece;
      char delim = ' ';
      std::stringstream stream(str);

      while(std::getline(stream, piece, delim)) {
        if(piece.length() == 0) {
          continue;
        }
        symbols.push_back(piece);
      }

      return symbols;
    }
  private:
    std::vector<std::string> productions;

    std::set<std::string> all_symbols;
    std::set<std::string> non_terminals;
    std::set<std::string> terminals;

    /** Creates an augmented grammar rule for the given grammar
     * Assumes grammar[0] is the original starting rule
     * Assumes S' is not already part of the grammar and
     * can be used as the augmented lhs
     *
     * Given grammar[0] as S -> E, will return S' -> S
     * 
     */
    std::string get_augmented_production() {
      // Nothing to do with no rules
      if(empty()) {
        return "";
      }

      std::string lhs = get_LHS(productions[0]);
      return AUGMENTED_LHS + " " + RULE_SEP + " " + lhs;
    }

    // Checks for a AUGMENTED_LHS symbol in non_terminals
    // If found, assumes this grammar has already been augmented
    bool is_augmented() {
      return non_terminals.count(AUGMENTED_LHS) > 0;
    }
};
#endif /* _GRAMMAR_HPP_ */