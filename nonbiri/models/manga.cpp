#include <stdexcept>

#include <nonbiri/database.h>
#include <nonbiri/models/manga.h>

Manga::Manga(const std::string &sourceId, const Manga_t &manga) : sourceId(sourceId), Manga_t(manga) {}

Manga::Manga(sqlite3_stmt *stmt)
{
  deserialize(stmt);
}

Json::Value Manga::toJson()
{
  Json::Value root;
  if (id > 0)
    root["id"] = id;
  if (!sourceId.empty())
    root["sourceId"] = sourceId;
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
  if ((int)status > 0)
    root["status"] = (int)status;
  if ((int)readingStatus > 0)
    root["readingStatus"] = (int)readingStatus;
  if (artists.size() > 0)
    for (auto &artist : artists)
      root["artists"].append(artist);
  if (authors.size() > 0)
    for (auto &author : authors)
      root["authors"].append(author);
  if (genres.size() > 0)
    for (auto &genre : genres)
      root["genres"].append(genre);
  return root;
}

void Manga::deserialize(sqlite3_stmt *stmt)
{
  if (stmt == nullptr)
    throw std::invalid_argument("stmt cannot be null");

  id = sqlite3_column_int64(stmt, 0);
  sourceId = std::string((const char *)sqlite3_column_text(stmt, 1));
  addedAt = sqlite3_column_int64(stmt, 2);
  updatedAt = sqlite3_column_int64(stmt, 3);
  lastReadAt = sqlite3_column_int64(stmt, 4);
  lastViewedAt = sqlite3_column_int64(stmt, 5);
  path = std::string((const char *)sqlite3_column_text(stmt, 6));
  coverUrl = std::string((const char *)sqlite3_column_text(stmt, 7));
  customCoverUrl = std::string((const char *)sqlite3_column_text(stmt, 8));
  bannerUrl = std::string((const char *)sqlite3_column_text(stmt, 9));
  title = std::string((const char *)sqlite3_column_text(stmt, 10));
  description = std::string((const char *)sqlite3_column_text(stmt, 11));
  status = (MangaStatus)sqlite3_column_int(stmt, 12);
  readingStatus = (ReadingStatus)sqlite3_column_int(stmt, 13);
}

void Manga::save()
{
  static const char *sql {
    "INSERT INTO manga ("
    "source_id, updated_at, last_read_at, last_viewed_at, path, cover_url, "
    "custom_cover_url, banner_url, title, description, status, reading_status"
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 1, sourceId.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 2, updatedAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 3, lastReadAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 4, lastViewedAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 5, path.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 6, coverUrl.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 7, customCoverUrl.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 8, bannerUrl.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 9, title.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 10, description.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 11, (int)status);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 12, (int)readingStatus);
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
}

void Manga::update()
{
  static const char *sql {
    "UPDATE manga SET source_id = ?, updated_at = ?, last_read_at = ?, last_viewed_at = ?, path = ?, "
    "cover_url = ?, custom_cover_url = ?, banner_url = ?, title = ?, description = ?, status = ?, reading_status = ? "
    "WHERE id = ?",
  };
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 1, sourceId.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 2, updatedAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 3, lastReadAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 4, lastViewedAt);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 5, path.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 6, coverUrl.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 7, customCoverUrl.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 8, bannerUrl.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 9, title.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_text(stmt, 10, description.c_str(), -1, SQLITE_STATIC);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 11, (int)status);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 12, (int)readingStatus);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int(stmt, 13, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));

  saveArtists();
  saveAuthors();
  saveGenres();
}

void Manga::remove()
{
  remove(id);
}

void Manga::loadArtists()
{
  static const char *sql {
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
    artists.push_back(std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0))));
  sqlite3_finalize(stmt);
}

void Manga::saveArtists()
{
  static const char *deleteSql {"DELETE FROM manga_artists WHERE manga_id = ?"};
  static const char *insertSql {"INSERT INTO manga_artists (manga_id, author_id) VALUES (?, ?) ON CONFLICT DO NOTHING"};
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

  for (const std::string &name : artists) {
    Entity *artist = Entity::find("author", name);
    if (artist == nullptr) {
      artist = new Entity(name);
      artist->save("author");
    }

    int64_t artistId = artist->id;
    delete artist;

    int exit = sqlite3_prepare_v2(Database::instance, insertSql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 1, id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 2, artistId);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
  }
}

void Manga::loadAuthors()
{
  static const char *sql {
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
    authors.push_back(std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0))));
  sqlite3_finalize(stmt);
}

void Manga::saveAuthors()
{
  static const char *deleteSql {"DELETE FROM manga_authors WHERE manga_id = ?"};
  static const char *insertSql {"INSERT INTO manga_authors (manga_id, author_id) VALUES (?, ?) ON CONFLICT DO NOTHING"};
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

  for (const std::string &name : authors) {
    Entity *author = Entity::find("author", name);
    if (author == nullptr) {
      author = new Entity(name);
      author->save("author");
    }

    int64_t authorId = author->id;
    delete author;

    int exit = sqlite3_prepare_v2(Database::instance, insertSql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 1, id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 2, authorId);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
  }
}

void Manga::loadGenres()
{
  static const char *sql {
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
    genres.push_back(std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0))));
  sqlite3_finalize(stmt);
}

void Manga::saveGenres()
{
  static const char *deleteSql {"DELETE FROM manga_genres WHERE manga_id = ?"};
  static const char *insertSql {"INSERT INTO manga_genres (manga_id, genre_id) VALUES (?, ?) ON CONFLICT DO NOTHING"};
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

  for (const std::string &name : genres) {
    Entity *genre = Entity::find("genre", name);
    if (genre == nullptr) {
      genre = new Entity(name);
      genre->save("genre");
    }

    int64_t genreId = genre->id;
    delete genre;

    int exit = sqlite3_prepare_v2(Database::instance, insertSql, -1, &stmt, nullptr);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 1, id);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_bind_int64(stmt, 2, genreId);
    if (exit != SQLITE_OK)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
    exit = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (exit != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errmsg(Database::instance));
  }
}

bool Manga::exists(int64_t id)
{
  static const char *sql {"SELECT 1 FROM manga WHERE id = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  if (exit != SQLITE_ROW && exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  return exit == SQLITE_ROW;
}

bool Manga::exists(const std::string &sourceId, const std::string &path)
{
  static const char *sql {"SELECT 1 FROM manga WHERE source_id = ? AND path = ?"};
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
  sqlite3_finalize(stmt);

  if (exit != SQLITE_ROW && exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  return exit == SQLITE_ROW;
}

Manga *Manga::find(int64_t id)
{
  static const char *sql {"SELECT * FROM manga WHERE id = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);

  Manga *manga = nullptr;
  if (exit == SQLITE_ROW)
    manga = new Manga(stmt);
  sqlite3_finalize(stmt);
  if (manga != nullptr) {
    manga->loadArtists();
    manga->loadAuthors();
    manga->loadGenres();
  }
  return manga;
}

Manga *Manga::find(const std::string &sourceId, const std::string &path)
{
  static const char *sql {"SELECT * FROM manga WHERE source_id = ? AND path = ?"};
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

  Manga *manga = nullptr;
  if (exit == SQLITE_ROW)
    manga = new Manga(stmt);
  sqlite3_finalize(stmt);
  if (manga != nullptr) {
    manga->loadArtists();
    manga->loadAuthors();
    manga->loadGenres();
  }
  return manga;
}

void Manga::remove(int64_t id)
{
  static const char *sql {"DELETE FROM manga WHERE id = ?"};
  sqlite3_stmt *stmt = nullptr;

  int exit = sqlite3_prepare_v2(Database::instance, sql, -1, &stmt, nullptr);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_bind_int64(stmt, 1, id);
  if (exit != SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
  exit = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
}

void Manga::remove(const std::string &sourceId, const std::string &path)
{
  static const char *sql {"DELETE FROM manga WHERE source_id = ? AND path = ?"};
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
  sqlite3_finalize(stmt);
  if (exit != SQLITE_DONE)
    throw std::runtime_error(sqlite3_errmsg(Database::instance));
}