#ifndef _LR1_ITEM_HPP_
#define _LR1_ITEM_HPP_

#include <stdio.h>
#include <string>
#include <regex>

// special symbol for grammar rule serparator
const static std::string RULE_SEP = "->";

// empty set symbol
const static std::string EPSILON = "~";

// EOF symbol
const static std::string DOLLAR = "$";

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

/**
 * Represents an item in an LR(1) grammar
 * For example S -> .E, $
 */ 
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
    // E.g. the "$" in S -> .E, $
    std::string lookahead;

    // The current position of the marker
    // E.g. when production = S -> E and position = 0 then marker =  S -> . E
    int position;
};

// Hash to store LR1 items in unordered_set
struct LR1ItemHash {
  // Return a hash for this item
  std::size_t operator()(const LR1Item& item) const noexcept {
    std::size_t h = std::hash<std::string>{}(item.get_str_for_hash());
    return h;
  }
};

#endif /* _LR1_ITEM_HPP_ */