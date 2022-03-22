#ifndef NONBIRI_MODELS_EXTENSION_H_
#define NONBIRI_MODELS_EXTENSION_H_

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <core/extension/extension.h>
#include <nonbiri/cache/lru.h>

class CManga;
class CChapter;

class CExtension : public Extension
{
  static LRU<CManga> mlru;

  void *handle = NULL;
  bool hasUpdate = false;

public:
  CExtension(const Extension &extension);
  ~CExtension();

  bool operator==(const CExtension &extension);
  bool operator!=(const CExtension &extension);

public:
  void *getHandle() const;
  void setHandle(void *handle);

  bool isHasUpdate() const;
  void setHasUpdate(bool hasUpdate);

public:
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> getLatests(int page);
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> searchManga(int page, const std::string &query);
  std::shared_ptr<CManga> getManga(const std::string &path, bool update = false);
  std::vector<std::shared_ptr<CChapter>> getChapters(CManga &manga);
  std::vector<std::shared_ptr<CChapter>> getChapters(const std::string &path);
  std::vector<std::string> getPages(const CChapter &chapter);
  std::vector<std::string> getPages(const std::string &path);

private:
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> normalizeMangaEntries(
      const std::tuple<std::vector<Manga *>, bool> &result);
  std::vector<std::shared_ptr<CChapter>> normalizeChapterEntries(CManga &manga, const std::vector<Chapter *> &result);
};

#endif  // NONBIRI_MODELS_EXTENSION_H_