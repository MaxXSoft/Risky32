#ifndef RISKY32_DEBUGGER_EXPREVAL_H_
#define RISKY32_DEBUGGER_EXPREVAL_H_

#include <string_view>
#include <ostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cstdint>

#include "core/core.h"

// expression evaluator
class ExprEvaluator {
 public:
  ExprEvaluator(Core &core) : core_(core), next_id_(0) {}

  // evaluate expression with record
  bool Eval(std::string_view expr, std::uint32_t &ans);
  // evaluate expression
  bool Eval(std::string_view expr, std::uint32_t &ans, bool record);
  // evaluate recorded expression by specific record id
  bool Eval(std::uint32_t id, std::uint32_t &ans);
  // show register information
  void PrintRegInfo(std::ostream &os);
  // show CSR information
  void PrintCSRInfo(std::ostream &os);
  // show expression by specific record id
  void PrintExpr(std::ostream &os, std::uint32_t id);

  // remove specific record
  void RemoveRecord(std::uint32_t id);
  // remove all records
  void Clear();

  // getters
  // next record id
  std::uint32_t next_id() const { return records_.size(); }

 private:
  enum class Token {
    End, Error, Char,
    Num, RegName, ValRef,
    Operator,
  };

  enum class Operator {
    Add, Sub, Mul, Div, Mod,
    And, Or, Not, Xor, Shl, Shr,
    LogicAnd, LogicOr, LogicNot,
    Equal, NotEqual,
    LessThan, LessEqual, GreaterThan, GreaterEqual,
  };

  // lexer
  void NextChar() { iss_ >> last_char_; }
  Token NextToken();
  Token HandleNum();
  Token HandleRegRef();
  Token HandleOperator();

  // parser
  bool ParseBinary(std::uint32_t &ans);
  bool ParseUnary(std::uint32_t &ans);
  bool ParseValue(std::uint32_t &ans);

  // reference of emulation core
  Core &core_;
  // all stored records
  std::unordered_map<std::uint32_t, std::string> records_;
  // next record id
  std::uint32_t next_id_;

  // current expression
  std::istringstream iss_;
  // last character
  char last_char_;
  // last character value
  char char_val_;
  // last number/regname/valref
  std::uint32_t num_val_;
  // last operator
  Operator op_val_;

  // current token
  Token cur_token_;
};

#endif  // RISKY32_DEBUGGER_EXPREVAL_H_
