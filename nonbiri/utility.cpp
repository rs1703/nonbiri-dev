#include <iostream>

#include <nonbiri/utility.h>

namespace Utils
{
ExecTime::ExecTime(const std::string &name) : name(name), start(std::chrono::high_resolution_clock::now()) {}

ExecTime::~ExecTime()
{
  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << name << " " << duration.count() << "ms" << std::endl;
}
}  // namespace Utils
