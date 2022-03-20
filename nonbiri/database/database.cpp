#include <nonbiri/database/database.h>
#include <nonbiri/database/schema.h>

sqlite3 *instance;

const char *db::init()
{
  int exit = sqlite3_open("nonbiri.db", &instance);
  if (exit != SQLITE_OK)
    return sqlite3_errmsg(instance);

  char *msgErr;
  exit = sqlite3_exec(instance, schema.c_str(), nullptr, 0, &msgErr);

  if (exit != SQLITE_OK)
    return msgErr;
  return nullptr;
}

void db::close()
{
  if (instance != nullptr)
    sqlite3_close(instance);
}

sqlite3 *db::getInstance()
{
  return instance;
}
