#ifndef NONBIRI_UTILITY_H_
#define NONBIRI_UTILITY_H_

#include <chrono>
#include <string>

#include <core/utility.h>

namespace Utils
{
struct ExecTime
{
  const std::string name;
  std::chrono::high_resolution_clock::time_point start;
  std::chrono::high_resolution_clock::time_point end;

  ExecTime(const std::string &name);
  ~ExecTime();
};
}; // namespace Utils

#endif  // NONBIRI_UTILITY_H_