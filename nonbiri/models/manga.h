#ifndef NONBIRI_MODELS_MANGA_H_
#define NONBIRI_MODELS_MANGA_H_

#include <memory>
#include <string>
#include <vector>

#include <core/models.h>
#include <json/json.h>
#include <nonbiri/models/entity.h>
#include <sqlite3.h>

enum class ReadingStatus
{
  Unknown,
  Reading,
  Planned,
  OnHold,
  Dropped
};

struct Library;
class Chapter;

class Manga : public Manga_t
{
  friend class Library;

  int64_t id {};
  std::string sourceId {};
  int64_t addedAt {};
  int64_t updatedAt {};
  int64_t lastReadAt {};
  int16_t lastViewedAt {};
  std::string customCoverUrl {};
  std::string bannerUrl {};
  ReadingStatus readingStatus {ReadingStatus::Unknown};
  std::vector<std::shared_ptr<Chapter>> chapters;

public:
  Manga() = default;
  Manga(const std::string &sourceId, const Manga_t &manga);
  Manga(sqlite3_stmt *stmt);

  Json::Value toJson();

protected:
  void deserialize(sqlite3_stmt *stmt);

private:
  void save();
  void update();
  void remove();

  void loadArtists();
  void saveArtists();

  void loadAuthors();
  void saveAuthors();

  void loadGenres();
  void saveGenres();

  static bool exists(int64_t id);
  static bool exists(const std::string &sourceId, const std::string &path);

  static Manga *find(int64_t id);
  static Manga *find(const std::string &sourceId, const std::string &path);

  static void remove(int64_t id);
  static void remove(const std::string &sourceId, const std::string &path);
};

#endif  // NONBIRI_MODELS_MANGA_H_