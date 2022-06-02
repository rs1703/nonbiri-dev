#include <iostream>
#include <stdexcept>

#include <nonbiri/database.h>
#include <nonbiri/models/entity.h>
#include <nonbiri/utility.h>

Entity::Entity(const std::string &name) : name(name) {}

Entity::Entity(sqlite3_stmt *stmt)
{
  deserialize(stmt);
}

Entity::~Entity()
{
  // std::cout << "Entity::~Entity()" << std::endl;
}

bool Entity::operator==(const Entity &other) const
{
  return id == other.id;
}

Json::Value Entity::toJson()
{
  Json::Value root {};
  if (id > 0)
    root["id"] = id;
  if (!name.empty())
    root["name"] = name;
  return root;
}

void Entity::save(const std::string &tableName)
{
  Utils::ExecTime execTime("Entity::save(tableName)");
  const std::string sql {"INSERT INTO " + tableName + " (name) VALUES (?)"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql.c_str(), -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  if (exit == SQLITE_DONE)
    id = sqlite3_last_insert_rowid(Database::instance);
  sqlite3_finalize(stmt);
  if (exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
}

std::shared_ptr<Entity> Entity::find(const std::string &tableName, const std::string &name)
{
  const std::string sql {"SELECT * FROM " + tableName + " WHERE LOWER(name) = LOWER(?)"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql.c_str(), -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  std::shared_ptr<Entity> entity = nullptr;
  if (exit == SQLITE_ROW)
    entity = std::make_shared<Entity>(stmt);
  sqlite3_finalize(stmt);
  return entity;
}

void Entity::deserialize(sqlite3_stmt *stmt)
{
  if (stmt == nullptr)
    throw std::invalid_argument("stmt cannot be null");

  id = sqlite3_column_int64(stmt, 0);
  name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
}