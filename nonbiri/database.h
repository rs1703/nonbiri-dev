#ifndef NONBIRI_DATABASE_H_
#define NONBIRI_DATABASE_H_

#include <mutex>
#include <string>
#include <vector>

#include <sqlite3.h>

namespace Database
{
class Tx
{
  static std::mutex mutex;
  bool isCommitted {};
  bool isRolledBack {};

public:
  Tx();
  ~Tx();

  void commit();
  void rollback();
};

extern sqlite3 *instance;

void initialize();
std::vector<std::string> deserializeArray(const std::string &str);
std::string serializeArray(const std::vector<std::string> &array);
}  // namespace Database

#endif  // NONBIRI_DATABASE_H_