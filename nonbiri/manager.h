#ifndef NONBIRI_MANAGER_H_
#define NONBIRI_MANAGER_H_

#include <atomic>
#include <filesystem>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <vector>

#include <core/extension.h>
#include <core/models.h>

using ExtensionMap = std::map<std::string, Extension *>;
using ExtensionInfoMap = std::map<std::string, ExtensionInfo>;

class Manager
{
  const std::string extensionsDir;
  std::atomic<time_t> indexLastUpdated;

  ExtensionMap mExtensions;
  std::shared_mutex mExtensionsMutex;

  ExtensionInfoMap mIndexes;
  std::shared_mutex mIndexesMutex;

public:
  Manager(const std::string &dir = "extensions");
  ~Manager();
  void reset();

  //
  Extension *getExtension(const std::string &id);
  const ExtensionMap &getExtensions();
  const std::shared_ptr<ExtensionInfo> getExtensionInfo(const std::string &id);
  const ExtensionInfoMap &getIndexes();

  void loadExtension(const std::string &name);
  void unloadExtension(const std::string &id);

  void downloadExtension(const std::string &id, bool update = false);
  void removeExtension(const std::string &id, std::filesystem::path path = "");

  void updateExtension(const std::string &id);
  void updateExtensionIndexes();

  //
  std::tuple<std::vector<MangaPtr_t>, bool> getLatests(const std::string &id, int page);
  std::tuple<std::vector<MangaPtr_t>, bool> searchManga(const std::string &id,
                                                        int page,
                                                        const std::string &query,
                                                        const std::vector<FilterKV> &filters);
  MangaPtr_t getManga(const std::string &id, const std::string &path);
  std::vector<ChapterPtr_t> getChapters(const std::string &id, Manga_t &manga);
  std::vector<std::string> getPages(const std::string &id, const std::string &path);

private:
  std::vector<std::string> getLocalExtensionPaths();
};

#endif  // NONBIRI_MANAGER_H_