#include <iostream>
#include <stdexcept>

#include <nonbiri/cache.h>
#include <nonbiri/database.h>
#include <nonbiri/models/chapter.h>
#include <nonbiri/models/entity.h>
#include <nonbiri/models/manga.h>
#include <nonbiri/utility.h>

Manga::Manga(const std::string &domain, const Manga_t &manga) : Manga_t(manga), domain(domain) {}

Manga::Manga(sqlite3_stmt *stmt)
{
  deserialize(stmt);
}

Manga::~Manga()
{
  // std::cout << "Manga::~Manga()" << std::endl;
}

bool Manga::operator==(const Manga &other) const
{
  return id == other.id || (domain == other.domain && path == other.path);
}

Json::Value Manga::toJson()
{
  Json::Value root {};
  if (id > 0)
    root["id"] = id;
  if (!domain.empty())
    root["domain"] = domain;
  if (addedAt > 0)
    root["addedAt"] = addedAt;
  if (updatedAt > 0)
    root["updatedAt"] = updatedAt;
  if (lastReadAt > 0)
    root["lastReadAt"] = lastReadAt;
  if (lastViewedAt > 0)
    root["lastViewedAt"] = lastViewedAt;
  if (!path.empty())
    root["path"] = path;
  if (!coverUrl.empty())
    root["coverUrl"] = coverUrl;
  if (!customCoverUrl.empty())
    root["customCoverUrl"] = customCoverUrl;
  if (!bannerUrl.empty())
    root["bannerUrl"] = bannerUrl;
  if (!title.empty())
    root["title"] = title;
  if (!description.empty())
    root["description"] = description;
  if (static_cast<int>(status) > 0)
    root["status"] = static_cast<int>(status);
  if (static_cast<int>(readingStatus) > 0)
    root["readingStatus"] = static_cast<int>(readingStatus);
  if (artists.size() > 0)
    for (const std::string &artist : artists)
      root["artists"].append(artist);
  if (authors.size() > 0)
    for (const std::string &author : authors)
      root["authors"].append(author);
  if (genres.size() > 0)
    for (const std::string &genre : genres)
      root["genres"].append(genre);
  return root;
}

std::vector<std::shared_ptr<Chapter>> Manga::getChapters()
{
  return Chapter::findAll(id);
}

ReadingStatus Manga::getReadState()
{
  if (static_cast<int>(readingStatus) < 0)
    readingStatus = getReadState(domain, path);
  return readingStatus;
}

void Manga::setReadState(ReadingStatus status)
{
  updatedAt = setReadState(status, domain, path);
}

void Manga::save()
{
  Utils::ExecTime execTime("Manga::save()");
  static constexpr const char *sql {
    "INSERT INTO manga ("
    " domain, path, cover_url,"
    " title, description, status"
    ") VALUES (?, ?, ?, ?, ?, ?)",
  };
  sqlite3_stmt *stmt = nullptr;

  Database::Tx t;
  try {
    int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 1, domain.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 3, coverUrl.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 4, title.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 5, description.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int(stmt, 6, static_cast<int>(status));
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);

    if (exit == SQLITE_DONE)
      id = sqlite3_last_insert_rowid(Database::instance);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));

    saveArtists();
    saveAuthors();
    saveGenres();
  } catch (...) {
    t.rollback();
    throw;
  }
  Cache::manga.remove(domain + path);
}

void Manga::update()
{
  Utils::ExecTime execTime("Manga::update()");
  static constexpr const char *sql {
    "UPDATE manga SET updated_at = ?, path = ?, cover_url = ?,"
    " custom_cover_url = ?, banner_url = ?, title = ?, description = ?, "
    " status = ?, reading_status = ? "
    "WHERE id = ?",
  };
  sqlite3_stmt *stmt = nullptr;
  int64_t now {time(nullptr)};

  Database::Tx t;
  try {
    int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 1, now);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 3, coverUrl.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 4, customCoverUrl.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 5, bannerUrl.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 6, title.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 7, description.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int(stmt, 8, static_cast<int>(status));
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int(stmt, 9, static_cast<int>(readingStatus));
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 10, id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));

    updatedAt = now;
    saveArtists();
    saveAuthors();
    saveGenres();
  } catch (...) {
    t.rollback();
    throw;
  }
}

void Manga::remove()
{
  remove(domain, path);
}

bool Manga::exists(const std::string &domain, const std::string &path)
{
  static constexpr const char *sql {"SELECT 1 FROM manga WHERE domain = ? AND path = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 1, domain.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  if (exit != SQLITE_ROW && exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  return exit == SQLITE_ROW;
}

ReadingStatus Manga::getReadState(const std::string &domain, const std::string &path)
{
  static constexpr const char *sql {"SELECT reading_status FROM manga WHERE domain = ? AND path = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 1, domain.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  ReadingStatus readingStatus {ReadingStatus::None};
  if (exit == SQLITE_ROW)
    readingStatus = (ReadingStatus)sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
  return readingStatus;
}

std::shared_ptr<Manga> Manga::find(const std::string &domain, const std::string &path)
{
  static constexpr const char *sql {"SELECT * FROM manga WHERE domain = ? AND path = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 1, domain.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  std::shared_ptr<Manga> manga = nullptr;
  if (exit == SQLITE_ROW)
    manga = std::make_shared<Manga>(stmt);
  sqlite3_finalize(stmt);
  if (manga != nullptr) {
    manga->loadArtists();
    manga->loadAuthors();
    manga->loadGenres();
  }
  return manga;
}

int64_t Manga::setReadState(ReadingStatus status, const std::string &domain, const std::string &path)
{
  Utils::ExecTime execTime("Manga::setReadState(status, domain, path, manga)");
  static constexpr const char *sql {
    "UPDATE manga SET reading_status = ?, updated_at = ?"
    " WHERE domain = ? AND path = ?",
  };
  sqlite3_stmt *stmt = nullptr;
  int64_t now {time(nullptr)};

  Database::Tx t;
  try {
    int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int(stmt, 1, static_cast<int>(status));
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 2, now);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 3, domain.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 4, path.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
  } catch (...) {
    t.rollback();
    throw;
  }
  return now;
}

void Manga::remove(const std::string &domain, const std::string &path)
{
  Utils::ExecTime execTime("Manga::remove(domain, path)");
  static constexpr const char *sql {"DELETE FROM manga WHERE domain = ? AND path = ?"};
  sqlite3_stmt *stmt = nullptr;

  Database::Tx t;
  try {
    int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 1, domain.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
  } catch (...) {
    t.rollback();
    throw;
  }
}

void Manga::loadArtists()
{
  static constexpr const char *sql {
    "SELECT name FROM author"
    " WHERE id IN (SELECT author_id FROM manga_artists WHERE manga_id = ?)",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  while (sqlite3_step(stmt) == SQLITE_ROW)
    artists.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
  sqlite3_finalize(stmt);
}

void Manga::loadAuthors()
{
  static constexpr const char *sql {
    "SELECT name FROM author"
    " WHERE id IN (SELECT author_id FROM manga_authors WHERE manga_id = ?)",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  while (sqlite3_step(stmt) == SQLITE_ROW)
    authors.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
  sqlite3_finalize(stmt);
}

void Manga::loadGenres()
{
  static constexpr const char *sql {
    "SELECT name FROM genre"
    " WHERE id IN (SELECT genre_id FROM manga_genres WHERE manga_id = ?)",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  while (sqlite3_step(stmt) == SQLITE_ROW)
    genres.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
  sqlite3_finalize(stmt);
}

void Manga::saveArtists()
{
  Utils::ExecTime execTime("Manga::saveArtists()");
  static constexpr const char *deleteSql {"DELETE FROM manga_artists WHERE manga_id = ?"};
  static constexpr const char *insertSql {
    "INSERT INTO manga_artists (manga_id, author_id)"
    " VALUES (?, ?)"
    "ON CONFLICT DO NOTHING",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, deleteSql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));

  static constexpr const char *k {"author"};
  for (const std::string &name : artists) {
    std::shared_ptr<Entity> artist = Entity::find(k, name);
    if (artist == nullptr) {
      artist = std::make_shared<Entity>(name);
      artist->save(k);
    }

    exit = sqlite3_prepare_v2(Database::instance, insertSql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 1, id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 2, artist->id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
  }
}

void Manga::saveAuthors()
{
  Utils::ExecTime execTime("Manga::saveAuthors()");
  static constexpr const char *deleteSql {"DELETE FROM manga_authors WHERE manga_id = ?"};
  static constexpr const char *insertSql {
    "INSERT INTO manga_authors (manga_id, author_id)"
    " VALUES (?, ?)"
    "ON CONFLICT DO NOTHING",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, deleteSql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));

  static constexpr const char *k {"author"};
  for (const std::string &name : authors) {
    std::shared_ptr<Entity> author = Entity::find(k, name);
    if (author == nullptr) {
      author = std::make_shared<Entity>(name);
      author->save(k);
    }

    exit = sqlite3_prepare_v2(Database::instance, insertSql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 1, id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 2, author->id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
  }
}

void Manga::saveGenres()
{
  Utils::ExecTime execTime("Manga::saveGenres()");
  static constexpr const char *deleteSql {"DELETE FROM manga_genres WHERE manga_id = ?"};
  static constexpr const char *insertSql {
    "INSERT INTO manga_genres (manga_id, genre_id)"
    " VALUES (?, ?)"
    "ON CONFLICT DO NOTHING",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, deleteSql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));

  static constexpr const char *k {"genre"};
  for (const std::string &name : genres) {
    std::shared_ptr<Entity> genre = Entity::find(k, name);
    if (genre == nullptr) {
      genre = std::make_shared<Entity>(name);
      genre->save(k);
    }

    exit = sqlite3_prepare_v2(Database::instance, insertSql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 1, id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 2, genre->id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
  }
}

void Manga::deserialize(sqlite3_stmt *stmt)
{
  if (stmt == nullptr)
    throw std::invalid_argument("stmt cannot be null");

  id = sqlite3_column_int64(stmt, 0);
  domain = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
  addedAt = sqlite3_column_int64(stmt, 2);
  updatedAt = sqlite3_column_int64(stmt, 3);
  lastReadAt = sqlite3_column_int64(stmt, 4);
  lastViewedAt = sqlite3_column_int64(stmt, 5);
  path = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
  coverUrl = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
  customCoverUrl = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));
  bannerUrl = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 9));
  title = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 10));
  description = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 11));
  status = static_cast<MangaStatus>(sqlite3_column_int(stmt, 12));
  readingStatus = static_cast<ReadingStatus>(sqlite3_column_int(stmt, 13));
}
