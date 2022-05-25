#ifndef NONBIRI_MODELS_ENTITY_H_
#define NONBIRI_MODELS_ENTITY_H_

#include <string>

#include <json/json.h>
#include <sqlite3.h>

struct Library;
class Manga;
class Chapter;

class Entity
{
public:
  friend class Library;
  friend class Manga;
  friend class Chapter;

  int64_t id {};
  std::string name {};

public:
  Entity(const std::string &name);
  Entity(sqlite3_stmt *stmt);

  Json::Value toJson();

protected:
  void deserialize(sqlite3_stmt *stmt);

private:
  void save(const std::string &tableName);
  void reload(const std::string &tableName);

  static Entity *find(const std::string &tableName, int64_t id);
  static Entity *find(const std::string &tableName, const std::string &name);
};

#endif  // NONBIRI_MODELS_ENTITY_H_