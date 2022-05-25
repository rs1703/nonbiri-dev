#include <nonbiri/database.h>
#include <nonbiri/models/entity.h>

Entity::Entity(const std::string &name) : name(name) {}
Entity::Entity(sqlite3_stmt *stmt)
{
  deserialize(stmt);
}

Json::Value Entity::toJson()
{
  Json::Value root;
  if (id > 0)
    root["id"] = id;
  if (!name.empty())
    root["name"] = name;
  return root;
}

void Entity::deserialize(sqlite3_stmt *stmt)
{
  if (stmt == nullptr)
    throw std::invalid_argument("stmt cannot be null");
  id = sqlite3_column_int64(stmt, 0);
  name = (const char *)sqlite3_column_text(stmt, 1);
}

void Entity::save(const std::string &tableName)
{
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

void Entity::reload(const std::string &tableName)
{
  const std::string sql {"SELECT * FROM " + tableName + " WHERE id = ? OR LOWER(name) = LOWER(?)"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql.c_str(), -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  if (exit == SQLITE_ROW)
    deserialize(stmt);
  sqlite3_finalize(stmt);
  if (exit != SQLITE_ROW)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
}

Entity *Entity::find(const std::string &tableName, int64_t id)
{
  const std::string sql {"SELECT * FROM " + tableName + " WHERE id = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql.c_str(), -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  Entity *entity = nullptr;
  if (exit == SQLITE_ROW)
    entity = new Entity(stmt);
  sqlite3_finalize(stmt);
  return entity;
}

Entity *Entity::find(const std::string &tableName, const std::string &name)
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

  Entity *entity = nullptr;
  if (exit == SQLITE_ROW)
    entity = new Entity(stmt);
  sqlite3_finalize(stmt);
  return entity;
  ;
}