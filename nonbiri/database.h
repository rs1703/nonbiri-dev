#ifndef NONBIRI_DATABASE_H_
#define NONBIRI_DATABASE_H_

#include <string>
#include <vector>

#include <sqlite3.h>

namespace Database
{
extern sqlite3 *instance;

void initialize();
std::vector<std::string> deserializeArray(const std::string &str);
std::string serializeArray(const std::vector<std::string> &array);
}  // namespace Database

#endif  // NONBIRI_DATABASE_H_