#ifndef NONBIRI_DATABASE_SCHEMA_H_
#define NONBIRI_DATABASE_SCHEMA_H_

#include <string>

std::string schema =
    "CREATE TABLE IF NOT EXISTS author ("
    "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  name TEXT NOT NULL"
    ");"
    ""
    "CREATE INDEX IF NOT EXISTS author_name_index ON author (name);"
    ""
    "CREATE TABLE IF NOT EXISTS scanlation_group ("
    "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  name TEXT NOT NULL"
    ");"
    ""
    "CREATE INDEX IF NOT EXISTS scanlation_group_name_index ON scanlation_group (name);"
    ""
    "CREATE TABLE IF NOT EXISTS genre ("
    "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  name TEXT NOT NULL"
    ");"
    ""
    "CREATE INDEX IF NOT EXISTS genre_name_index ON genre (name);"
    ""
    "CREATE TABLE IF NOT EXISTS manga ("
    "  id           INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  source       TEXT NOT NULL,"
    ""
    "  added_at     INTEGER NOT NULL,"
    "  updated_at   INTEGER NOT NULL,"
    ""
    "  url          TEXT NOT NULL,"
    "  cover_url    TEXT NOT NULL,"
    ""
    "  title        TEXT NOT NULL,"
    "  description  TEXT NOT NULL,"
    "  status       INTEGER NOT NULL DEFAULT 0"
    ");"
    ""
    "CREATE INDEX IF NOT EXISTS manga_source_index ON manga (source);"
    "CREATE INDEX IF NOT EXISTS manga_added_at_index ON manga (added_at);"
    "CREATE INDEX IF NOT EXISTS manga_updated_at_index ON manga (updated_at);"
    "CREATE INDEX IF NOT EXISTS manga_title_index ON manga (title);"
    "CREATE INDEX IF NOT EXISTS manga_status_index ON manga (status);"
    ""
    "CREATE TABLE IF NOT EXISTS manga_author ("
    "  manga_id   INTEGER NOT NULL REFERENCES manga(id),"
    "  author_id  INTEGER NOT NULL REFERENCES author(id),"
    "  PRIMARY KEY (manga_id, author_id)"
    ");"
    ""
    "CREATE INDEX IF NOT EXISTS manga_author_manga_id_index ON manga_author (manga_id);"
    "CREATE INDEX IF NOT EXISTS manga_author_author_id_index ON manga_author (author_id);"
    ""
    "CREATE TABLE IF NOT EXISTS manga_genre ("
    "  manga_id   INTEGER NOT NULL REFERENCES manga(id),"
    "  genre_id   INTEGER NOT NULL REFERENCES genre(id),"
    "  PRIMARY KEY (manga_id, genre_id)"
    ");"
    ""
    "CREATE INDEX IF NOT EXISTS manga_genre_manga_id_index ON manga_genre (manga_id);"
    "CREATE INDEX IF NOT EXISTS manga_genre_genre_id_index ON manga_genre (genre_id);"
    ""
    "CREATE TABLE IF NOT EXISTS chapter ("
    "  id           INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  manga_id     INTEGER NOT NULL REFERENCES manga(id),"
    ""
    "  added_at     INTEGER,"
    "  uploaded_at  INTEGER,"
    ""
    "  last_read_at INTEGER,"
    "  last_page_at INTEGER,"
    ""
    "  url          TEXT NOT NULL,"
    "  name         TEXT NOT NULL,"
    "  pages        BLOB NOT NULL DEFAULT '[]',"
    "  page_count   INTEGER NOT NULL DEFAULT 0,"
    "  downloaded   INTEGER NOT NULL DEFAULT 0"
    ");"
    ""
    "CREATE INDEX IF NOT EXISTS chapter_manga_id_index ON chapter (manga_id);"
    "CREATE INDEX IF NOT EXISTS chapter_added_at_index ON chapter (added_at);"
    "CREATE INDEX IF NOT EXISTS chapter_uploaded_at_index ON chapter (uploaded_at);"
    "CREATE INDEX IF NOT EXISTS chapter_last_read_at_index ON chapter (last_read_at);"
    "CREATE INDEX IF NOT EXISTS chapter_last_page_at_index ON chapter (last_page_at);"
    "CREATE INDEX IF NOT EXISTS Chapter_page_count_index ON chapter (page_count);"
    "CREATE INDEX IF NOT EXISTS chapter_downloaded_index ON chapter (downloaded);"
    ""
    "CREATE TABLE IF NOT EXISTS chapter_scanlation_group ("
    "  chapter_id          INTEGER NOT NULL REFERENCES chapter(id),"
    "  scanlation_group_id INTEGER NOT NULL REFERENCES scanlation_group(id),"
    "  PRIMARY KEY (chapter_id, scanlation_group_id)"
    ");"
    ""
    "CREATE INDEX IF NOT EXISTS chapter_scanlation_group_chapter_id_index ON chapter_scanlation_group (chapter_id);"
    "CREATE INDEX IF NOT EXISTS chapter_scanlation_group_scanlation_group_id_index ON chapter_scanlation_group "
    "(scanlation_group_id);";

#endif  // NONBIRI_DATABASE_SCHEMA_H_