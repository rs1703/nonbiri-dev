#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <nonbiri/database.h>

sqlite3 *Database::instance = nullptr;

void Database::initialize()
{
  if (instance != nullptr)
    return;

  std::cout << "Initializing database..." << std::endl;
  int exit = sqlite3_open("nonbiri.db", &instance);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(instance));

  std::ifstream file("nonbiri.sql");
  if (!file.is_open())
    throw std::runtime_error("Could not open nonbiri.sql");

  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();

  char *msgErr = nullptr;
  exit = sqlite3_exec(instance, buffer.str().c_str(), nullptr, 0, &msgErr);

  if (exit != SQLITE_OK)
    throw std::runtime_error(msgErr);
}

std::vector<std::string> Database::deserializeArray(const std::string &str)
{
  std::istringstream parse(str);
  std::vector<std::string> ret;
  for (std::string token; std::getline(parse, token, ',');)
    ret.push_back(token);
  return ret;
}

std::string Database::serializeArray(const std::vector<std::string> &array)
{
  std::ostringstream cat;
  for (auto const &str : array)
    cat << str << ',';
  std::string ret = cat.str();
  return ret.substr(0, ret.size() - 1);
}