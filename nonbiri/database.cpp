#include <iostream>
#include <stdexcept>

#include <nonbiri/database.h>
#include <nonbiri/schema.h>
#include <sqlite3.h>

sqlite3 *instance = nullptr;

void Database::initialize()
{
  if (instance != nullptr)
    return;

  std::cout << "Initializing database..." << std::endl;
  int exit = sqlite3_open("nonbiri.db", &instance);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(instance));

  char *msgErr;
  exit = sqlite3_exec(instance, schema.c_str(), nullptr, 0, &msgErr);

  if (exit != SQLITE_OK)
    throw std::runtime_error(msgErr);
}
