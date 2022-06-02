#include <iostream>

#include <nonbiri/utility.h>

Utils::ExecTime::ExecTime(const std::string &name) : name(name), start(std::chrono::high_resolution_clock::now()) {}

Utils::ExecTime::~ExecTime()
{
  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << name << " " << duration.count() << "ms" << std::endl;
}
