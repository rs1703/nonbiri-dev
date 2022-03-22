#ifndef NONBIRI_MANAGER_MANAGER_H
#define NONBIRI_MANAGER_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <nonbiri/models/extension.h>
#include <nonbiri/models/manga.h>

class Manager
{
  const std::string extensionsDir;
  std::shared_ptr<CExtension> currentExtension = nullptr;
  std::string currentQuery;
  int currentPage = 1;
  bool hasNext = true;

public:
  std::map<std::string, std::shared_ptr<CExtension>> extensions;
  std::map<std::string, ExtensionInfo> indexes;

public:
  Manager(const std::string &dir = "extensions");
  ~Manager();
  void reset();

  //
  void loadExtension(const std::string &name);
  void unloadExtension(const std::string &name);
  void downloadExtension(const std::string &name, bool update = false);
  void updateExtension(const std::string &name);
  void updateExtensionIndexes();

  //
  void setCurrentExtension(std::shared_ptr<CExtension> ext);
  void setCurrentQuery(const std::string &query);

  //
  std::vector<std::shared_ptr<CManga>> getLatests();
  std::vector<std::shared_ptr<CManga>> searchManga(const std::string &query);
  std::shared_ptr<CManga> getManga(const std::string &url);
  std::vector<std::shared_ptr<CChapter>> getChapters(CManga &manga);
  std::vector<std::string> getPages(Chapter &chapter);

private:
  //
  std::vector<std::string> getLocalExtensionPaths();

  //
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> getLatests(std::shared_ptr<CExtension> ext, int page);
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> searchManga(std::shared_ptr<CExtension> ext,
                                                                     int page,
                                                                     const std::string &query);
  std::shared_ptr<CManga> getManga(std::shared_ptr<CExtension> ext, const std::string &url);
  std::vector<std::shared_ptr<CChapter>> getChapters(std::shared_ptr<CExtension> ext, CManga &manga);
  std::vector<std::string> getPages(std::shared_ptr<CExtension> ext, Chapter &chapter);
};

#endif  // NONBIRI_MANAGER_MANAGER_H