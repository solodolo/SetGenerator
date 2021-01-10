#ifndef _LR1_ITEM_HPP_
#define _LR1_ITEM_HPP_

#include <stdio.h>
#include <string>
#include <regex>

#include "grammar.hpp"

/**
 * Represents an item in an LR(1) grammar
 * For example S -> .E, $
 */ 
class LR1Item {
  public:
    LR1Item(std::string production, int production_num, std::string lookahead, int position) :
      production_num(production_num),
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

    int get_production_num() const {
      return production_num;
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

    // If this rule's lhs == S'
    bool is_augmented_production() const {
      return lhs == AUGMENTED_LHS;
    }

    // TODO : Deprecated. Was used for storing in unordered_set.
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
    // The number of the production of this item in the grammar
    // Useful for determining which state to reduce to when building parse table
    int production_num;

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
// TODO : This method is deprecated in favor of LR1Comparator
struct LR1ItemHash {
  // Return a hash for this item
  std::size_t operator()(const LR1Item& item) const noexcept {
    std::size_t h = std::hash<std::string>{}(item.get_str_for_hash());
    return h;
  }
};

// lhs == rhs if their hashing strings are the same
// i.e. if each item has the same lhs,rhs,lookahead, and position
struct LR1Comparator {
  bool operator()(const LR1Item& lhs, const LR1Item& rhs) const {
    return lhs.get_str_for_hash() < rhs.get_str_for_hash();
  }
};

// Builds a string consiting of the hashing strings of each item
// in lhs and rhs respectively.
// These strings are then compared to determine order.
struct LR1SetComparator {
  bool operator()(const std::set<LR1Item, LR1Comparator>& lhs, const std::set<LR1Item, LR1Comparator>& rhs) const {
    std::string lhs_str;
    for(const LR1Item& item : lhs) {
      lhs_str += item.get_str_for_hash();
    }

    std::string rhs_str;
    for(const LR1Item& item : rhs) {
      rhs_str += item.get_str_for_hash();
    }

    return lhs_str < rhs_str;
  }
};

#endif /* _LR1_ITEM_HPP_ */