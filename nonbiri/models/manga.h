#ifndef NONBIRI_MODELS_MANGA_H_
#define NONBIRI_MODELS_MANGA_H_

#include <memory>
#include <string>
#include <vector>

#include <core/models.h>
#include <json/json.h>
#include <sqlite3.h>

enum class ReadingStatus
{
  None,
  Reading,
  Planned,
  OnHold,
  Dropped
};

class Chapter;

class Manga : public Manga_t
{
public:
  int64_t id {};
  std::string sourceId {};
  int64_t addedAt {};
  int64_t updatedAt {};
  int64_t lastReadAt {};
  int64_t lastViewedAt {};
  std::string customCoverUrl {};
  std::string bannerUrl {};
  ReadingStatus readingStatus {-1};

public:
  Manga() = default;
  Manga(const std::string &sourceId, const Manga_t &manga);
  Manga(sqlite3_stmt *stmt);
  ~Manga();

  bool operator==(const Manga &other) const;
  Json::Value toJson();

  std::vector<std::shared_ptr<Chapter>> getChapters();
  ReadingStatus getReadState();
  void setReadState(ReadingStatus status);

  void save();
  void update();
  void remove();

  static std::shared_ptr<Manga> find(const std::string &sourceId, const std::string &path);
  static bool exists(const std::string &sourceId, const std::string &path);
  static ReadingStatus getReadState(const std::string &sourceId, const std::string &path);
  static int64_t setReadState(ReadingStatus status, const std::string &sourceId, const std::string &path);
  static void remove(const std::string &sourceId, const std::string &path);

private:
  void loadArtists();
  void loadAuthors();
  void loadGenres();
  void saveArtists();
  void saveAuthors();
  void saveGenres();
  void deserialize(sqlite3_stmt *stmt);
};

#endif  // NONBIRI_MODELS_MANGA_H_