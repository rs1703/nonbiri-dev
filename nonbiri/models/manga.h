#ifndef NONBIRI_MODELS_MANGA_H_
#define NONBIRI_MODELS_MANGA_H_

#include <map>
#include <memory>
#include <string>

#include <core/models/models.h>

class CChapter;
class CExtension;

class CManga : public Manga
{
  std::shared_ptr<CExtension> extension;
  std::map<std::string, std::shared_ptr<CChapter>> chapters;

public:
  CManga(const Manga &manga);
  CManga(const CManga &manga);
  CManga(const CExtension &extension, const Manga &manga);
  ~CManga();

  std::shared_ptr<CExtension> getExtension() const;
  void setExtension(const CExtension &extension);

  std::shared_ptr<CChapter> getChapter(const std::string &path);
  std::map<std::string, std::shared_ptr<CChapter>> getChapters() const;

  void addChapter(const std::string &path, const CChapter &chapter);
  void removeChapter(const std::string &path);
  void clearChapters();
};

#endif  // NONBIRI_MODELS_MANGA_H_