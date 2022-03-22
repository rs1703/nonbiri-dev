#ifndef NONBIRI_MANAGER_MANAGER_H
#define NONBIRI_MANAGER_MANAGER_H

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <nonbiri/models/chapter.h>
#include <nonbiri/models/extension.h>
#include <nonbiri/models/manga.h>

class Manager
{
  const std::string extensionsDir;
  std::atomic<time_t> indexLastUpdate;

public:
  std::map<std::string, std::shared_ptr<CExtension>> extensions;
  std::map<std::string, ExtensionInfo> indexes;

public:
  Manager(const std::string &dir = "extensions");
  ~Manager();
  void reset();

  //
  std::shared_ptr<CExtension> getExtension(const std::string &id);
  void loadExtension(const std::string &name);
  void unloadExtension(const std::string &id);
  void downloadExtension(const std::string &id, bool update = false);
  void updateExtension(const std::string &id);
  void updateExtensionIndexes();
  void checkExtensions();

  //
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> getLatests(const std::string &id, int page = 1);
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> searchManga(const std::string &id,
                                                                     int page = 1,
                                                                     const std::string &query = "");
  std::shared_ptr<CManga> getManga(const std::string &id, const std::string &path);
  std::vector<std::shared_ptr<CChapter>> getChapters(const std::string &id, CManga &manga);
  std::vector<std::string> getPages(const std::string &id, const CChapter &chapter);

private:
  std::vector<std::string> getLocalExtensionPaths();
};

#endif  // NONBIRI_MANAGER_MANAGER_H