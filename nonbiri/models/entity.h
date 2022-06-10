#ifndef NONBIRI_MODELS_ENTITY_H_
#define NONBIRI_MODELS_ENTITY_H_

#include <memory>
#include <string>

#include <json/json.h>
#include <sqlite3.h>

class Entity
{
public:
  int64_t id {};
  std::string name {};

public:
  Entity() = default;
  Entity(const std::string &name);
  Entity(sqlite3_stmt *stmt);
  ~Entity();

  bool operator==(const Entity &other) const;
  Json::Value toJson();
  void save(const std::string &tableName);

  static std::shared_ptr<Entity> find(const std::string &tableName, const std::string &name);

private:
  void deserialize(sqlite3_stmt *stmt);
};

#endif  // NONBIRI_MODELS_ENTITY_H_