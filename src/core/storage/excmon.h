#ifndef RISKY32_CORE_STORAGE_EXCMON_H_
#define RISKY32_CORE_STORAGE_EXCMON_H_

#include <cstdint>

// exclusive monitor
class ExclusiveMonitor {
 public:
  ExclusiveMonitor() { ClearFlag(); }

  void SetFlag(std::uint32_t addr) {
    flag_ = true;
    addr_ = addr;
  }

  void ClearFlag() {
    flag_ = false;
    addr_ = 0;
  }

  bool CheckFlag(std::uint32_t addr) const {
    return flag_ && addr_ == addr;
  }

 private:
  bool flag_;
  std::uint32_t addr_;
};

#endif  // RISKY32_CORE_STORAGE_EXCMON_H_
