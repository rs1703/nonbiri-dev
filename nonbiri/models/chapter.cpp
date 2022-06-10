#include <iostream>
#include <stdexcept>

#include <nonbiri/database.h>
#include <nonbiri/models/chapter.h>
#include <nonbiri/utility.h>

Chapter::Chapter(const std::string &sourceId, const Chapter_t &chapter) : Chapter_t(chapter), sourceId(sourceId) {}

Chapter::Chapter(int64_t mangaId, const std::string &sourceId, const Chapter_t &chapter) :
  Chapter_t(chapter),
  mangaId(mangaId),
  sourceId(sourceId)
{
}

Chapter::Chapter(sqlite3_stmt *stmt)
{
  deserialize(stmt);
}

Chapter::~Chapter()
{
  // std::cout << "Chapter::~Chapter()" << std::endl;
}

bool Chapter::operator==(const Chapter &other) const
{
  return id == other.id || (sourceId == other.sourceId && path == other.path);
}

Json::Value Chapter::toJson()
{
  Json::Value root {};
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
    for (const std::string &group : groups)
      root["groups"].append(group);
  return root;
}

void Chapter::save(int64_t mangaId)
{
  Utils::ExecTime execTime("Chapter::save");
  if (this->mangaId <= 0 && mangaId <= 0)
    throw std::runtime_error("Chapter::save(): mangaId is required");

  static constexpr const char *sql {
    "INSERT INTO chapter ("
    " manga_id, source_id, published_at,"
    " path, name, page_count"
    ") VALUES (?, ?, ?, ?, ?, ?)",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, mangaId > 0 ? mangaId : this->mangaId);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 2, sourceId.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 3, publishedAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 4, path.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 5, name.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 6, pageCount);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  if (exit == SQLITE_DONE)
    id = sqlite3_last_insert_rowid(Database::instance);
  sqlite3_finalize(stmt);
  if (exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));

  if (mangaId > 0)
    this->mangaId = mangaId;
}

std::shared_ptr<Chapter> Chapter::find(std::string sourceId, std::string path)
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

  std::shared_ptr<Chapter> chapter = nullptr;
  if (exit == SQLITE_ROW)
    chapter = std::make_shared<Chapter>(stmt);
  sqlite3_finalize(stmt);
  return chapter;
}

std::vector<std::shared_ptr<Chapter>> Chapter::findAll(int64_t mangaId)
{
  if (mangaId <= 0)
    return {};

  static constexpr const char *sql {"SELECT * FROM chapter WHERE manga_id = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, mangaId);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));

  std::vector<std::shared_ptr<Chapter>> chapters {};
  while (exit = sqlite3_step(stmt), exit == SQLITE_ROW)
    chapters.push_back(std::make_shared<Chapter>(stmt));
  sqlite3_finalize(stmt);
  return chapters;
}

void Chapter::saveAll(const std::vector<std::shared_ptr<Chapter>> &chapters, int64_t mangaId)
{
  Utils::ExecTime execTime("Chapter::saveAll");
  if (chapters.empty())
    return;

  Database::Tx t;
  try {
    for (std::shared_ptr<Chapter> chapter : chapters) {
      if (chapter->id)
        continue;
      chapter->save(mangaId);
    }
  } catch (...) {
    t.rollback();
    throw;
  }
}

void Chapter::deserialize(sqlite3_stmt *stmt)
{
  if (stmt == nullptr)
    throw std::invalid_argument("stmt cannot be null");

  id           = sqlite3_column_int64(stmt, 0);
  mangaId      = sqlite3_column_int64(stmt, 1);
  sourceId     = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
  addedAt      = sqlite3_column_int64(stmt, 3);
  updatedAt    = sqlite3_column_int64(stmt, 4);
  publishedAt  = sqlite3_column_int64(stmt, 5);
  downloadedAt = sqlite3_column_int64(stmt, 6);
  lastReadAt   = sqlite3_column_int64(stmt, 7);
  lastReadPage = sqlite3_column_int(stmt, 8);
  readCount    = sqlite3_column_int(stmt, 9);
  path         = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 10));
  name         = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 11));

  const void *blob = sqlite3_column_blob(stmt, 12);
  if (blob != nullptr) {
    auto str = std::string(reinterpret_cast<const char *>(blob), sqlite3_column_bytes(stmt, 12));
    pages    = Database::deserializeArray(str);
  }

  pageCount    = sqlite3_column_int(stmt, 13);
  isDownloaded = sqlite3_column_int(stmt, 14);
}