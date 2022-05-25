#ifndef NONBIRI_MODELS_CHAPTER_H_
#define NONBIRI_MODELS_CHAPTER_H_

#include <memory>
#include <string>
#include <vector>

#include <core/models.h>
#include <json/json.h>
#include <nonbiri/models/entity.h>
#include <sqlite3.h>

struct Library;

class Chapter : public Chapter_t
{
  friend class Library;

  int64_t id {};
  int64_t mangaId {};
  std::string sourceId {};
  int64_t addedAt {};
  int64_t updatedAt {};
  int64_t downloadedAt {};
  int64_t lastReadAt {};
  int16_t lastReadPage {};
  int64_t readCount {};
  std::vector<std::string> pages;
  int16_t pageCount {};
  bool isDownloaded {false};

public:
  Chapter() = default;
  Chapter(const std::string &sourceId, const Chapter_t &chapter);
  Chapter(sqlite3_stmt *stmt);

  Json::Value toJson();

protected:
  void deserialize(sqlite3_stmt *stmt);

private:
  void save();
  void remove();

  static Chapter *find(int64_t id);
  static Chapter *find(std::string sourceId, std::string path);
};

#endif  // NONBIRI_MODELS_CHAPTER_H_