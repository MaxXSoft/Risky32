#include "debugger/expreval.h"

#include <iostream>
#include <iomanip>
#include <stack>
#include <utility>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>

#include "define/csr.h"

namespace {

/*

EBNF of expressions:

binary  ::= unary bin_op unray
unary   ::= una_op value
value   ::= NUM | REG_NAME | '(' binary ')'

*/

// name of all GPRs in 'info reg' command
constexpr const char *kRegNames[] = {
  "ra", "sp",   "gp",   "tp", "t0", "t1", "t2", "fp",
  "s1", "a0",   "a1",   "a2", "a3", "a4", "a5", "a6",
  "a7", "s2",   "s3",   "s4", "s5", "s6", "s7", "s8",
  "s9", "s10",  "s11",  "t3", "t4", "t5", "t6", "pc",
};

// name of all CSRs in 'info csr' command
constexpr const char *kCSRNames[] = {
  "sstatus",  "sscratch", "sepc",     "satp",
  "mstatus",  "misa",     "mie",      "mtvec",  "mscratch",
  "mepc",     "mcause",   "mtval",    "mip",
  "mcycle",   "minstret", "mcycleh",  "minstreth",
};

// hashmap of name of all GPRs & CSRs
const std::unordered_map<std::string_view, std::uint32_t> kRegCSRMap = {
  // GPRs
  {"x0", 0}, {"zero", 0},           {"x1", 1}, {"ra", 1},
  {"x2", 2}, {"sp", 2},             {"x3", 3}, {"gp", 3},
  {"x4", 4}, {"tp", 4},             {"x5", 5}, {"t0", 5},
  {"x6", 6}, {"t1", 6},             {"x7", 7}, {"t2", 7},
  {"x8", 8}, {"s0", 8}, {"fp", 8},  {"x9", 9}, {"s1", 9},
  {"x10", 10}, {"a0", 10},          {"x11", 11}, {"a1", 11},
  {"x12", 12}, {"a2", 12},          {"x13", 13}, {"a3", 13},
  {"x14", 14}, {"a4", 14},          {"x15", 15}, {"a5", 15},
  {"x16", 16}, {"a6", 16},          {"x17", 17}, {"a7", 17},
  {"x18", 18}, {"s2", 18},          {"x19", 19}, {"s3", 19},
  {"x20", 20}, {"s4", 20},          {"x21", 21}, {"s5", 21},
  {"x22", 22}, {"s6", 22},          {"x23", 23}, {"s7", 23},
  {"x24", 24}, {"s8", 24},          {"x25", 25}, {"s9", 25},
  {"x26", 26}, {"s10", 26},         {"x27", 27}, {"s11", 27},
  {"x28", 28}, {"t3", 28},          {"x29", 29}, {"t4", 29},
  {"x30", 30}, {"t5", 30},          {"x31", 31}, {"t6", 31},
  {"pc", 32},
  // U-mode CSRs
  {"cycle", kCSRCycle}, {"instret", kCSRInstRet},
  {"cycleh", kCSRCycleH}, {"instreth", kCSRInstRetH},
  // S-mode CSRs
  {"sstatus", kCSRSStatus}, {"sie", kCSRSIE}, {"stvec", kCSRSTVec},
  {"scounteren", kCSRSCounterEn}, {"sscratch", kCSRSScratch},
  {"sepc", kCSRSEPC}, {"scause", kCSRSCause}, {"stval", kCSRSTVal},
  {"sip", kCSRSIP}, {"satp", kCSRSATP},
  // M-mode CSRs
  {"mvenderid", kCSRMVenderId}, {"marchid", kCSRMArchId},
  {"mimpid", kCSRMImpId}, {"mhartid", kCSRMHartId},
  {"mstatus", kCSRMStatus}, {"misa", kCSRMISA}, {"mie", kCSRMIE},
  {"mtvec", kCSRMTVec}, {"mcounteren", kCSRMCounterEn},
  {"mscratch", kCSRMScratch}, {"mepc", kCSRMEPC}, {"mcause", kCSRMCause},
  {"mtval", kCSRMTVal}, {"mip", kCSRMIP},
  {"pmpcfg0", kCSRPMPCfg0}, {"pmpcfg1", kCSRPMPCfg1},
  {"pmpcfg2", kCSRPMPCfg2}, {"pmpcfg3", kCSRPMPCfg3},
  {"pmpaddr0", kCSRPMPAddr0}, {"pmpaddr1", kCSRPMPAddr1},
  {"pmpaddr2", kCSRPMPAddr2}, {"pmpaddr3", kCSRPMPAddr3},
  {"pmpaddr4", kCSRPMPAddr4}, {"pmpaddr5", kCSRPMPAddr5},
  {"pmpaddr6", kCSRPMPAddr6}, {"pmpaddr7", kCSRPMPAddr7},
  {"pmpaddr8", kCSRPMPAddr8}, {"pmpaddr9", kCSRPMPAddr9},
  {"pmpaddr10", kCSRPMPAddr10}, {"pmpaddr11", kCSRPMPAddr11},
  {"pmpaddr12", kCSRPMPAddr12}, {"pmpaddr13", kCSRPMPAddr13},
  {"pmpaddr14", kCSRPMPAddr14}, {"pmpaddr15", kCSRPMPAddr15},
  {"mcycle", kCSRMCycle}, {"minstret", kCSRMInstRet},
  {"mcycleh", kCSRMCycleH}, {"minstreth", kCSRMInstRetH},
  {"mcountinhibit", kCSRMCountInhibit},
};

// all supported operators
constexpr std::string_view kOpList[] = {
  "+", "-", "*", "/", "%",
  "&", "|", "~", "^", "<<", ">>",
  "&&", "||", "!",
  "==", "!=",
  "<", "<=", ">", ">=",
};

// precedence of operators
constexpr int kOpPrec[] = {
  90, 90, 100, 100, 100,
  50, 30, -1, 40, 80, 80,
  20, 10, -1,
  60, 60,
  70, 70, 70, 70,
};

// check if specific character can appear in operators
inline bool IsOperatorChar(char c) {
  assert(c);
  constexpr const char kOpChar[] = "+-*/%&|~^!=<>";
  for (const auto &i : kOpChar) {
    if (c == i) return true;
  }
  return false;
}

}  // namespace

ExprEvaluator::Token ExprEvaluator::NextToken() {
  // skip spaces
  while (!iss_.eof() && std::isspace(last_char_)) NextChar();
  // end of stream
  if (iss_.eof()) return cur_token_ = Token::End;
  // numbers
  if (std::isdigit(last_char_)) return HandleNum();
  // register name or value reference
  if (last_char_ == '$') return HandleRegRef();
  // operator
  if (IsOperatorChar(last_char_)) return HandleOperator();
  // other characters
  char_val_ = last_char_;
  NextChar();
  return cur_token_ = Token::Char;
}

ExprEvaluator::Token ExprEvaluator::HandleNum() {
  std::string num;
  bool is_hex = false;
  // check if is hexadecimal number
  if (last_char_ == '0') {
    NextChar();
    if (std::tolower(last_char_) == 'x') {
      // is hexadecimal
      is_hex = true;
      NextChar();
    }
    else if (!std::isdigit(last_char_)) {
      // just zero
      num_val_ = 0;
      return cur_token_ = Token::Num;
    }
  }
  // read number string
  while (!iss_.eof() && std::isxdigit(last_char_)) {
    num += last_char_;
    NextChar();
  }
  // convert to number
  char *end_pos;
  num_val_ = std::strtol(num.c_str(), &end_pos, is_hex ? 16 : 10);
  if (*end_pos) return LogLexerError("invalid number literal");
  return cur_token_ = Token::Num;
}

ExprEvaluator::Token ExprEvaluator::HandleRegRef() {
  std::string ref;
  // eat '$'
  NextChar();
  if (std::isalpha(last_char_)) {
    // get register name
    while (!iss_.eof() && std::isalpha(last_char_)) {
      ref += last_char_;
      NextChar();
    }
    // get register address
    auto it = kRegCSRMap.find(ref);
    if (it == kRegCSRMap.end()) {
      return LogLexerError("invalid register name");
    }
    num_val_ = it->second;
    return cur_token_ = Token::RegName;
  }
  else if (std::isdigit(last_char_)) {
    // get value reference number (record id)
    while (!iss_.eof() && std::isdigit(last_char_)) {
      ref += last_char_;
      NextChar();
    }
    // convert to number
    char *end_pos;
    num_val_ = std::strtol(ref.c_str(), &end_pos, 10);
    if (*end_pos || records_.find(num_val_) == records_.end()) {
      return LogLexerError("invalid value reference");
    }
    return cur_token_ = Token::ValRef;
  }
  else {
    return LogLexerError("invalid '$' expression");
  }
}

ExprEvaluator::Token ExprEvaluator::HandleOperator() {
  std::string op;
  // get operator string
  do {
    op += last_char_;
    NextChar();
  } while (!iss_.eof() && IsOperatorChar(last_char_));
  // check is a valid operator
  for (int i = 0; i < sizeof(kOpList) / sizeof(std::string_view); ++i) {
    if (kOpList[i] == op) {
      op_val_ = static_cast<Operator>(i);
      return cur_token_ = Token::Operator;
    }
  }
  return LogLexerError("invalid operator");
}

bool ExprEvaluator::Parse(std::uint32_t &ans) {
  if (cur_token_ == Token::End) return false;
  if (!ParseBinary(ans)) return false;
  return cur_token_ == Token::End;
}

bool ExprEvaluator::ParseBinary(std::uint32_t &ans) {
  std::stack<std::uint32_t> oprs;
  std::stack<Operator> ops;
  std::uint32_t val;
  // get the first value
  if (!ParseUnary(val)) return false;
  oprs.push(val);
  // calculate using stack
  while (cur_token_ == Token::Operator) {
    // get operator
    auto op = op_val_;
    if (kOpPrec[static_cast<int>(op)] < 0) break;
    NextToken();
    // handle operator
    while (!ops.empty() && GetOpPrec(ops.top()) >= GetOpPrec(op)) {
      // get current operator & operand
      auto cur_op = ops.top();
      ops.pop();
      auto rhs = oprs.top();
      oprs.pop();
      auto lhs = oprs.top();
      oprs.pop();
      // calculate
      oprs.push(CalcByOperator(cur_op, lhs, rhs));
    }
    // push & get next value
    ops.push(op);
    if (!ParseUnary(val)) return false;
    oprs.push(val);
  }
  // clear stacks
  while (!ops.empty()) {
    auto cur_op = ops.top();
    ops.pop();
    auto rhs = oprs.top();
    oprs.pop();
    auto lhs = oprs.top();
    oprs.pop();
    oprs.push(CalcByOperator(cur_op, lhs, rhs));
  }
  ans = oprs.top();
  return true;
}

bool ExprEvaluator::ParseUnary(std::uint32_t &ans) {
  // check if need to get operator
  if (cur_token_ == Token::Operator) {
    auto op = op_val_;
    // get operand
    std::uint32_t operand;
    if (!ParseUnary(operand)) return false;
    // calculate
    switch (op) {
      case Operator::Add: ans = operand; break;
      case Operator::Sub: ans = -operand; break;
      case Operator::LogicNot: ans = !operand; break;
      case Operator::Not: ans = ~operand; break;
      case Operator::Mul: {
        if (operand & 0b11) return LogParserError("address misaligned");
        ans = core_.raw_bus()->ReadWord(operand);
        break;
      }
      default: return LogParserError("invalid unary operator");
    }
    return true;
  }
  else {
    return ParseValue(ans);
  }
}

bool ExprEvaluator::ParseValue(std::uint32_t &ans) {
  switch (cur_token_) {
    case Token::Num: {
      // just number
      ans = num_val_;
      break;
    }
    case Token::RegName: {
      // get GPR/CSR value from core
      if (num_val_ <= 32) {
        ans = core_.regs(num_val_);
      }
      else {
        ans = core_.csr().ReadDataForce(num_val_);
      }
      break;
    }
    case Token::ValRef: {
      // store current state
      auto iss = std::move(iss_);
      auto last_char = last_char_;
      auto cur_token = cur_token_;
      // evaluate record
      auto ret = Eval(num_val_, ans);
      assert(ret);
      // restore current state
      iss_ = std::move(iss);
      last_char_ = last_char;
      cur_token_ = cur_token;
      break;
    }
    case Token::Char: {
      // check & eat '('
      if (char_val_ != '(') return LogParserError("expected '('");
      NextToken();
      // parse inner binary expression
      if (!ParseBinary(ans)) return false;
      // check ')'
      if (cur_token_ != Token::Char || char_val_ != ')') {
        return LogParserError("expected ')'");
      }
      break;
    }
    default: return LogParserError("invalid value");
  }
  NextToken();
  return true;
}

std::uint32_t ExprEvaluator::GetOpPrec(Operator op) {
  return kOpPrec[static_cast<int>(op)];
}

std::uint32_t ExprEvaluator::CalcByOperator(Operator op, std::uint32_t lhs,
                                            std::uint32_t rhs) {
  switch (op) {
    case Operator::Add:           return lhs + rhs;
    case Operator::Sub:           return lhs - rhs;
    case Operator::Mul:           return lhs * rhs;
    case Operator::Div:           return lhs / rhs;
    case Operator::And:           return lhs & rhs;
    case Operator::Or:            return lhs | rhs;
    case Operator::Xor:           return lhs ^ rhs;
    case Operator::Shl:           return lhs << rhs;
    case Operator::Shr:           return lhs >> rhs;
    case Operator::LogicAnd:      return lhs && rhs;
    case Operator::LogicOr:       return lhs || rhs;
    case Operator::Equal:         return lhs == rhs;
    case Operator::NotEqual:      return lhs != rhs;
    case Operator::LessThan:      return lhs < rhs;
    case Operator::LessEqual:     return lhs <= rhs;
    case Operator::GreaterThan:   return lhs > rhs;
    case Operator::GreaterEqual:  return lhs >= rhs;
    default: return 0;
  }
}

ExprEvaluator::Token ExprEvaluator::LogLexerError(std::string_view msg) {
  std::cout << "ERROR (expr.lexer): " << msg << std::endl;
  return cur_token_ = Token::Error;
}

bool ExprEvaluator::LogParserError(std::string_view msg) {
  std::cout << "ERROR (expr.parser): " << msg << std::endl;
  return false;
}

bool ExprEvaluator::Eval(std::string_view expr, std::uint32_t &ans) {
  return Eval(expr, ans, true);
}

bool ExprEvaluator::Eval(std::string_view expr, std::uint32_t &ans,
                         bool record) {
  // reset string stream
  iss_.str({expr.data(), expr.size()});
  iss_.clear();
  last_char_ = ' ';
  // call lexer & parser
  NextToken();
  if (!Parse(ans)) return false;
  // record expression
  if (record) {
    expr.remove_prefix(std::min(expr.find_first_not_of(" "), expr.size()));
    expr.remove_suffix(std::min(expr.find_last_not_of(" "), expr.size()));
    records_.insert({next_id_++, {expr.data(), expr.size()}});
  }
  return true;
}

bool ExprEvaluator::Eval(std::uint32_t id, std::uint32_t &ans) {
  auto it = records_.find(id);
  if (it == records_.end()) return false;
  return Eval(it->second, ans, false);
}

void ExprEvaluator::PrintRegInfo(std::ostream &os) {
  int count = 0;
  for (const auto &i : kRegNames) {
    // get register number
    auto it = kRegCSRMap.find(i);
    assert(it != kRegCSRMap.end());
    // print value of register
    auto val = core_.regs(it->second);
    os << std::setw(4) << std::setfill(' ') << std::left << i << std::hex
       << std::setw(8) << std::setfill('0') << std::right << val << "   ";
    // print new line
    if (count++ == 3) {
      count = 0;
      os << std::endl;
    }
  }
  if (count) os << std::endl;
}

void ExprEvaluator::PrintCSRInfo(std::ostream &os) {
  int count = 0;
  for (const auto &i : kCSRNames) {
    // get CSR address
    auto it = kRegCSRMap.find(i);
    assert(it != kRegCSRMap.end());
    // print value of CSR
    auto val = core_.csr().ReadDataForce(it->second);
    os << std::setw(10) << std::setfill(' ') << std::left << i << std::hex
       << std::setw(8) << std::setfill('0') << std::right << val << "   ";
    // print new line
    if (count++ == 2) {
      count = 0;
      os << std::endl;
    }
  }
  if (count) os << std::endl;
}

void ExprEvaluator::PrintExpr(std::ostream &os, std::uint32_t id) {
  auto it = records_.find(id);
  assert(it != records_.end());
  os << it->second;
}

void ExprEvaluator::RemoveRecord(std::uint32_t id) {
  records_.erase(id);
}

void ExprEvaluator::Clear() {
  records_.clear();
}
