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

#include <nonbiri/models/chapter.h>
#include <nonbiri/models/extension.h>
#include <nonbiri/models/manga.h>

using ExtensionMap = std::map<std::string, ExtensionPtr>;
using ExtensionInfoMap = std::map<std::string, ExtensionInfo>;

class Manager
{
  const std::string extensionsDir;
  std::atomic<time_t> indexLastUpdated;

  ExtensionMap mExtensions;
  std::map<std::string, void *> mExtensionHandles;
  std::shared_mutex mExtensionsMutex;

  ExtensionInfoMap mIndexes;
  std::shared_mutex mIndexesMutex;

public:
  Manager(const std::string &dir = "extensions");
  ~Manager();
  void reset();

  //
  ExtensionPtr getExtension(const std::string &id);
  ExtensionMap &getExtensions();
  ExtensionInfo *getExtensionInfo(const std::string &id);
  ExtensionInfoMap &getIndexes();

  void loadExtension(const std::string &name);
  void unloadExtension(const std::string &id);

  void downloadExtension(const std::string &id, bool update = false);
  void removeExtension(const std::string &id, std::filesystem::path path = "");

  void updateExtension(const std::string &id);
  void updateExtensionIndexes();

  //
  std::tuple<std::vector<MangaPtr>, bool> getLatests(const std::string &id, int page = 1);
  std::tuple<std::vector<MangaPtr>, bool> searchManga(const std::string &id,
                                                      int page = 1,
                                                      const std::string &query = "");
  MangaPtr getManga(const std::string &id, const std::string &path);
  std::vector<ChapterPtr> getChapters(const std::string &id, CManga &manga);
  std::vector<std::string> getPages(const std::string &id, const std::string &path);

private:
  std::vector<std::string> getLocalExtensionPaths();
};

#endif  // NONBIRI_MANAGER_H_