CREATE TABLE IF NOT EXISTS author (
  id    INTEGER PRIMARY KEY,
  name  TEXT NOT NULL
);
CREATE UNIQUE INDEX IF NOT EXISTS author_name_unique_idx ON author (name);

CREATE TABLE IF NOT EXISTS genre (
  id    INTEGER PRIMARY KEY,
  name  TEXT NOT NULL
);
CREATE UNIQUE INDEX IF NOT EXISTS genre_name_unique_idx ON genre (name);

CREATE TABLE IF NOT EXISTS scanlation_group (
  id    INTEGER PRIMARY KEY,
  name  TEXT NOT NULL
);
CREATE UNIQUE INDEX IF NOT EXISTS scanlation_group_name_unique_idx ON scanlation_group (name);

CREATE TABLE IF NOT EXISTS manga (
  id                INTEGER PRIMARY KEY AUTOINCREMENT,
  domain            TEXT NOT NULL,

  added_at          INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
  updated_at        INTEGER,

  last_read_at      INTEGER,
  last_viewed_at    INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),

  path              TEXT NOT NULL,
  cover_url         TEXT NOT NULL,
  custom_cover_url  TEXT DEFAULT "",
  banner_url        TEXT DEFAULT "",
  
  title             TEXT NOT NULL,
  description       TEXT DEFAULT "",
  status            INTEGER DEFAULT 0,
  reading_status    INTEGER DEFAULT 1
);

CREATE UNIQUE INDEX IF NOT EXISTS manga_uidx ON manga(domain, path);
CREATE INDEX IF NOT EXISTS manga_domain_idx ON manga(domain);
CREATE INDEX IF NOT EXISTS manga_added_at_idx ON manga(added_at);
CREATE INDEX IF NOT EXISTS manga_updated_at_idx ON manga(updated_at);
CREATE INDEX IF NOT EXISTS manga_last_read_at_idx ON manga(last_read_at);
CREATE INDEX IF NOT EXISTS manga_last_viewed_at_idx ON manga(last_viewed_at);
CREATE INDEX IF NOT EXISTS manga_path_idx ON manga(path);
CREATE INDEX IF NOT EXISTS manga_title_idx ON manga(title);
CREATE INDEX IF NOT EXISTS manga_status_idx ON manga(status);

CREATE TABLE IF NOT EXISTS manga_artists ( 
  manga_id  INTEGER NOT NULL REFERENCES manga (id),
  author_id INTEGER NOT NULL REFERENCES author (id),
  PRIMARY KEY (manga_id, author_id)
);
CREATE UNIQUE INDEX IF NOT EXISTS manga_artists_uidx ON manga_artists (manga_id, author_id);

CREATE TABLE IF NOT EXISTS manga_authors (
  manga_id  INTEGER NOT NULL REFERENCES manga (id),
  author_id INTEGER NOT NULL REFERENCES author (id),
  PRIMARY KEY (manga_id, author_id)
);

CREATE UNIQUE INDEX IF NOT EXISTS manga_authors_uidx ON manga_authors (manga_id, author_id);

CREATE TABLE IF NOT EXISTS manga_genres (
  manga_id INTEGER NOT NULL REFERENCES manga (id),
  genre_id INTEGER NOT NULL REFERENCES genre (id),
  PRIMARY KEY (manga_id, genre_id)
);

CREATE UNIQUE INDEX IF NOT EXISTS manga_genres_uidx ON manga_genres (manga_id, genre_id);

CREATE TABLE IF NOT EXISTS chapter (
  id              INTEGER PRIMARY KEY AUTOINCREMENT,
  manga_id        INTEGER NOT NULL REFERENCES manga (id),
  domain          TEXT NOT NULL,

  added_at        INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
  updated_at      INTEGER,
  published_at    INTEGER NOT NULL,
  downloaded_at   INTEGER,

  last_read_at    INTEGER,
  last_read_page  INTEGER DEFAULT 0,
  read_count      INTEGER DEFAULT 0,

  path            TEXT NOT NULL,
  name            TEXT NOT NULL,
  pages           BLOB DEFAULT '[]',
  page_count      INTEGER DEFAULT 0,
  downloaded      INTEGER DEFAULT 0
);

CREATE UNIQUE INDEX IF NOT EXISTS chapter_uidx ON chapter(manga_id, domain, path);
CREATE INDEX IF NOT EXISTS chapter_added_at_idx ON chapter(added_at);
CREATE INDEX IF NOT EXISTS chapter_updated_at_idx ON chapter(updated_at);
CREATE INDEX IF NOT EXISTS chapter_published_at_idx ON chapter(published_at);
CREATE INDEX IF NOT EXISTS chapter_downloaded_at_idx ON chapter(downloaded_at);
CREATE INDEX IF NOT EXISTS chapter_last_read_at_idx ON chapter(last_read_at);
CREATE INDEX IF NOT EXISTS chapter_last_read_page_idx ON chapter(last_read_page);
CREATE INDEX IF NOT EXISTS chapter_path_idx ON chapter(path);
CREATE INDEX IF NOT EXISTS chapter_name_idx ON chapter(name);

CREATE TABLE IF NOT EXISTS chapter_scanlation_groups (
  chapter_id          INTEGER NOT NULL REFERENCES chapter (id),
  scanlation_group_id INTEGER NOT NULL REFERENCES scanlation_group (id),
  PRIMARY KEY (chapter_id, scanlation_group_id)
);

CREATE UNIQUE INDEX IF NOT EXISTS chapter_scanlation_groups_uidx ON chapter_scanlation_groups (chapter_id, scanlation_group_id);
