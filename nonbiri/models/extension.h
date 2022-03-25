#ifndef NONBIRI_MODELS_EXTENSION_H_
#define NONBIRI_MODELS_EXTENSION_H_

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <vector>

#include <core/extension/extension.h>
#include <nonbiri/lru.h>
#include <nonbiri/models/chapter.h>
#include <nonbiri/models/manga.h>

class CExtension : public Extension
{
  static LRU<CManga> mLRU;

public:
  std::atomic_bool hasUpdate = false;

public:
  CExtension(const Extension &extension);
  ~CExtension();

  bool operator==(const CExtension &extension);
  bool operator!=(const CExtension &extension);

  std::tuple<std::vector<MangaPtr>, bool> getLatests(int page);
  std::tuple<std::vector<MangaPtr>, bool> searchManga(int page, const std::string &query);
  MangaPtr getManga(const std::string &path, bool update = false);
  std::vector<ChapterPtr> getChapters(CManga &manga);
  std::vector<ChapterPtr> getChapters(const std::string &path);
  std::vector<std::string> getPages(const CChapter &chapter);
  std::vector<std::string> getPages(const std::string &path);

private:
  std::tuple<std::vector<MangaPtr>, bool> normalizeMangaEntries(const std::tuple<std::vector<Manga *>, bool> &result);
  std::vector<ChapterPtr> normalizeChapterEntries(const std::vector<Chapter *> &result);
};

using ExtensionPtr = std::shared_ptr<CExtension>;

#endif  // NONBIRI_MODELS_EXTENSION_H_