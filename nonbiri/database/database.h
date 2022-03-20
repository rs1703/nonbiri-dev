#ifndef NONBIRI_DATABASE_DATABASE_H_
#define NONBIRI_DATABASE_DATABASE_H_

#include <sqlite3.h>

namespace db
{

const char *init();
void close();

sqlite3 *getInstance();

}  // namespace db

#endif  // NONBIRI_DATABASE_DATABASE_H_