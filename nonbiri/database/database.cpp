#include <stdexcept>

#include <nonbiri/database/database.h>
#include <nonbiri/database/schema.h>
#include <sqlite3.h>

using std::runtime_error;
using std::string;

sqlite3 *instance = nullptr;

void Database::initialize()
{
  if (instance != nullptr)
    return;

  int exit = sqlite3_open("nonbiri.db", &instance);
  if (exit != SQLITE_OK)
    throw runtime_error(sqlite3_errmsg(instance));

  char *msgErr;
  exit = sqlite3_exec(instance, schema.c_str(), nullptr, 0, &msgErr);

  if (exit != SQLITE_OK)
    throw runtime_error(msgErr);
}
