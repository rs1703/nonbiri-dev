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
#include <nonbiri/models/chapter.h>
#include <nonbiri/models/manga.h>

class Manager
{
  const std::string extensionsDir;
  std::atomic<time_t> indexLastUpdated;

  std::map<std::string, Extension *> extensions;
  std::shared_mutex extensionsMutex;

  std::map<std::string, void *> handles;
  std::shared_mutex handlesMutex;

  std::map<std::string, ExtensionInfo> indexes;
  std::shared_mutex indexesMutex;

public:
  Manager(const std::string &dir = "extensions");
  ~Manager();
  void reset();

  //
  Extension *getExtension(const std::string &id);
  const std::map<std::string, Extension *> &getExtensions();
  const ExtensionInfo *getExtensionInfo(const std::string &id);
  const std::map<std::string, ExtensionInfo> &getIndexes();

  void loadExtension(const std::string &path);
  void unloadExtension(const std::string &id);

  void downloadExtension(const std::string &id, bool update = false);
  void removeExtension(const std::string &id, std::filesystem::path path = "");

  void updateExtension(const std::string &id);
  void updateExtensionIndexes();

  //
  std::tuple<std::vector<std::shared_ptr<Manga>>, bool> getLatests(Extension &ext, int page);
  std::tuple<std::vector<std::shared_ptr<Manga>>, bool> searchManga(
    Extension &ext, int page, const std::string &query, const std::vector<std::pair<std::string, std::string>> &filters);

  std::shared_ptr<Manga> getManga(Extension &ext, const std::string &path);
  std::vector<std::shared_ptr<Chapter>> getChapters(Extension &ext, const std::string &path);
  std::vector<std::shared_ptr<Chapter>> getChapters(Extension &ext, Manga &manga);
  std::vector<std::string> getPages(Extension &ext, const std::string &path);

private:
  std::vector<std::string> getLocalExtensionPaths();
};

namespace App
{
extern Manager *manager;
}  // namespace App

#endif  // NONBIRI_MANAGER_H_