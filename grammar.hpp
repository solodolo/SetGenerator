#ifndef _GRAMMAR_HPP_
#define _GRAMMAR_HPP_

#include <regex>
#include <string>

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

// Given a production like A -> sSB, returns sSB
std::string get_RHS(const std::string production) {
  const auto& found = production.find(RULE_SEP);

  if(found != std::string::npos) {
    return remove_whitespace(production.substr(found + 2)); // skip "->"
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

  // Return a list of the non-terminals in the provided list of symbols
  std::unordered_set<std::string> get_nonterminals(std::vector<std::string> symbols) {
    std::unordered_set<std::string> non_terminals;

    for(const std::string& symbol : symbols) {
      if(!is_terminal(symbol)) {
        non_terminals.insert(symbol);
      }
    }

    return non_terminals;
  }

  // Return a list of the terminals in the provided list of symbols
  std::unordered_set<std::string> get_terminals(std::vector<std::string> symbols) {
    std::unordered_set<std::string> terminals;

    for(const std::string& symbol : symbols) {
      if(is_terminal(symbol)) {
        terminals.insert(symbol);
      }
    }

    return terminals;
  }

#endif /* _GRAMMAR_HPP_ */