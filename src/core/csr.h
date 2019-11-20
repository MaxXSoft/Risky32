#ifndef RISKY32_CORE_CSR_H_
#define RISKY32_CORE_CSR_H_

#include <unordered_map>
#include <cstdint>

class CSR {
 public:
  CSR();

  std::uint32_t ReadData(std::uint32_t addr);
  void WriteData(std::uint32_t addr, std::uint32_t value);

  // setters
  void set_ext_int(bool ext_int) { ext_int_ = ext_int; }

  // getters
  std::uint32_t mtvec() const { return *mtvec_; }
  std::uint32_t mepc() const { return *mepc_; }

 private:
  bool ext_int_;
  std::unordered_map<std::uint32_t, std::uint32_t> csrs_;
  std::uint32_t *mtvec_, *mepc_;
};

#endif  // RISKY32_CORE_CSR_H_
