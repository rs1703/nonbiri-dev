#ifndef NONBIRI_MODELS_CHAPTER_H_
#define NONBIRI_MODELS_CHAPTER_H_

#include <memory>
#include <string>
#include <vector>

#include <core/models.h>
#include <json/json.h>
#include <sqlite3.h>

class Chapter : public Chapter_t
{
public:
  int64_t id {};
  int64_t mangaId {};
  std::string domain {};
  int64_t addedAt {};
  int64_t updatedAt {};
  int64_t downloadedAt {};
  int64_t lastReadAt {};
  int16_t lastReadPage {};
  int64_t readCount {};
  std::vector<std::string> pages {};
  int16_t pageCount {};
  bool isDownloaded {};

public:
  Chapter() = default;
  Chapter(const std::string &domain, const Chapter_t &chapter);
  Chapter(int64_t mangaId, const std::string &domain, const Chapter_t &chapter);
  Chapter(sqlite3_stmt *stmt);
  ~Chapter();

  bool operator==(const Chapter &other) const;
  Json::Value toJson();
  void save(int64_t mangaId = 0);

  static std::shared_ptr<Chapter> find(std::string domain, std::string path);
  static std::vector<std::shared_ptr<Chapter>> findAll(int64_t mangaId);
  static void saveAll(const std::vector<std::shared_ptr<Chapter>> &chapters, int64_t mangaId = 0);

private:
  void deserialize(sqlite3_stmt *stmt);
};

#endif  // NONBIRI_MODELS_CHAPTER_H_