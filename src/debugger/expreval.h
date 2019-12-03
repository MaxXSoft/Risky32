#ifndef RISKY32_DEBUGGER_EXPREVAL_H_
#define RISKY32_DEBUGGER_EXPREVAL_H_

#include <string_view>
#include <ostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>

#include "core/core.h"

// expression evaluator
class ExprEvaluator {
 public:
  ExprEvaluator(Core &core) : core_(core) {}

  // evaluate expression with record
  bool Eval(std::string_view expr, std::uint32_t &ans);
  // evaluate expression
  bool Eval(std::string_view expr, std::uint32_t &ans, bool record);
  // show register information
  void PrintRegInfo(std::ostream &os);
  // show CSR information
  void PrintCSRInfo(std::ostream &os);

 private:
  enum class Token {
    End, Error, Char,
    Num, Id, RegName, ValRef,
    Operator,
  };

  enum class Operator {
    Add, Sub, Mul, Div, Mod,
    And, Or, Not, Xor,
    LogicAnd, LogicOr, LogicNot,
    Equal, NotEqual,
    LessThan, LessEqual, GreaterThan, GreaterEqual,
  };

  Token NextToken();

  // reference of emulation core
  Core &core_;
  // all stored records
  std::vector<std::string> records_;
  // current expression
  std::istringstream iss;
  // last character
  char last_char_;
  // last number/valref
  std::uint32_t last_num_;
  // last id/regname
  std::string last_str_;
  // last operator
  Operator last_op_;
  // current token
  Token cur_token_;
};

#endif  // RISKY32_DEBUGGER_EXPREVAL_H_
