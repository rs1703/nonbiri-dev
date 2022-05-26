#include <stdexcept>

#include <nonbiri/database.h>
#include <nonbiri/models/chapter.h>

Chapter::Chapter(const std::string &sourceId, const Chapter_t &chapter) : sourceId(sourceId), Chapter_t(chapter) {}

Chapter::Chapter(sqlite3_stmt *stmt)
{
  deserialize(stmt);
}

Json::Value Chapter::toJson()
{
  Json::Value root;
  if (id > 0)
    root["id"] = id;
  if (mangaId > 0)
    root["mangaId"] = mangaId;
  if (!sourceId.empty())
    root["sourceId"] = sourceId;
  if (addedAt > 0)
    root["addedAt"] = addedAt;
  if (updatedAt > 0)
    root["updatedAt"] = updatedAt;
  if (publishedAt > 0)
    root["publishedAt"] = publishedAt;
  if (downloadedAt > 0)
    root["downloadedAt"] = downloadedAt;
  if (lastReadAt > 0)
    root["lastReadAt"] = lastReadAt;
  if (lastReadPage > 0)
    root["lastReadPage"] = lastReadPage;
  if (readCount > 0)
    root["readCount"] = readCount;
  if (!path.empty())
    root["path"] = path;
  if (!name.empty())
    root["name"] = name;
  if (pageCount > 0)
    root["pageCount"] = pageCount;
  if (isDownloaded)
    root["isDownloaded"] = isDownloaded;
  if (groups.size() > 0)
    for (auto &group : groups)
      root["groups"].append(group);
  return root;
}

void Chapter::deserialize(sqlite3_stmt *stmt)
{
  if (stmt == nullptr)
    throw std::invalid_argument("stmt cannot be null");

  id = sqlite3_column_int64(stmt, 0);
  mangaId = sqlite3_column_int64(stmt, 1);
  sourceId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
  addedAt = sqlite3_column_int64(stmt, 3);
  updatedAt = sqlite3_column_int64(stmt, 4);
  publishedAt = sqlite3_column_int64(stmt, 5);
  downloadedAt = sqlite3_column_int64(stmt, 6);
  lastReadAt = sqlite3_column_int64(stmt, 7);
  lastReadPage = sqlite3_column_int(stmt, 8);
  readCount = sqlite3_column_int(stmt, 9);
  path = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 10));
  name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 11));

  const void *blob = sqlite3_column_blob(stmt, 12);
  if (blob != nullptr) {
    auto str = std::string(reinterpret_cast<const char *>(blob), sqlite3_column_bytes(stmt, 12));
    pages = Database::deserializeArray(str);
  }

  pageCount = sqlite3_column_int(stmt, 13);
  isDownloaded = sqlite3_column_int(stmt, 14);
}

void Chapter::save()
{
  static constexpr const char *sql {
    "INSERT INTO chapter ("
    " manga_id, source_id, updated_at, published_at, downloaded_at, last_read_at, last_read_page,"
    " read_count, path, name, page_count, downloaded"
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, mangaId);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 2, sourceId.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 3, updatedAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 4, publishedAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 5, downloadedAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 6, lastReadAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 7, lastReadPage);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 8, readCount);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 9, path.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 10, name.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 11, pageCount);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 12, isDownloaded);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  if (exit == SQLITE_DONE)
    id = sqlite3_last_insert_rowid(Database::instance);
  sqlite3_finalize(stmt);
  if (exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
}

Chapter *Chapter::find(int64_t id)
{
  static constexpr const char *sql {"SELECT * FROM chapter WHERE id = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  Chapter *chapter = nullptr;
  if (exit == SQLITE_ROW)
    chapter = new Chapter(stmt);
  sqlite3_finalize(stmt);
  return chapter;
}

Chapter *Chapter::find(std::string sourceId, std::string path)
{
  static constexpr const char *sql {"SELECT * FROM chapter WHERE source_id = ? AND path = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 1, sourceId.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  Chapter *chapter = nullptr;
  if (exit == SQLITE_ROW)
    chapter = new Chapter(stmt);
  sqlite3_finalize(stmt);
  return chapter;
}
