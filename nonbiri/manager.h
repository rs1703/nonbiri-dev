#ifndef NONBIRI_MANAGER_H_
#define NONBIRI_MANAGER_H_

#include <map>
#include <memory>

#include <nonbiri/models/extension.h>
#include <nonbiri/models/manga.h>

class Manager
{
public:
  std::map<std::string, std::shared_ptr<CExtension>> extensions;

  std::shared_ptr<CExtension> currentExtension = NULL;
  const char *currentQuery = NULL;
  int currentPage = 1;
  bool hasNext = true;

public:
  static const std::vector<std::string> getExtensions(const std::string &path);

public:
  Manager();
  ~Manager();
  void reset();

public:
  std::shared_ptr<CExtension> getExtension(const std::string &name) const;
  std::map<std::string, std::shared_ptr<CExtension>> getExtensions();
  std::shared_ptr<CExtension> loadExtension(const std::string &path);

public:
  void setCurrentExtension(std::shared_ptr<CExtension> extension);
  void setCurrentQuery(const char *query);

public:
  std::vector<std::shared_ptr<CManga>> getLatests();
  std::vector<std::shared_ptr<CManga>> searchManga(const char *query);
  std::shared_ptr<CManga> getManga(const std::string &url);
  std::vector<std::shared_ptr<CChapter>> getChapters(CManga &manga);
  std::vector<std::string> getPages(Chapter &chapter);

private:
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> getLatests(const std::string &name, int page);
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> getLatests(std::shared_ptr<CExtension> ext, int page);

  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> searchManga(const std::string &name,
                                                                     int page,
                                                                     const char *query);
  std::tuple<std::vector<std::shared_ptr<CManga>>, bool> searchManga(std::shared_ptr<CExtension> ext,
                                                                     int page,
                                                                     const char *query);

private:
  std::shared_ptr<CManga> getManga(const std::string &name, const std::string &url);
  std::shared_ptr<CManga> getManga(std::shared_ptr<CExtension> ext, const std::string &url);

private:
  std::vector<std::shared_ptr<CChapter>> getChapters(const std::string &name, CManga &manga);
  std::vector<std::shared_ptr<CChapter>> getChapters(std::shared_ptr<CExtension> ext, CManga &manga);

private:
  std::vector<std::string> getPages(const std::string &name, Chapter &chapter);
  std::vector<std::string> getPages(std::shared_ptr<CExtension> ext, Chapter &chapter);
};

#endif  // NONBIRI_MANAGER_H_