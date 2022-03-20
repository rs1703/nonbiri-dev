#ifndef NONBIRI_MODELS_MANGA_H_
#define NONBIRI_MODELS_MANGA_H_

#include <memory>

#include <core/models/models.h>
#include <nonbiri/models/extension.h>

class CChapter;

class CManga : public Manga
{
public:
  std::shared_ptr<CExtension> extension;
  std::vector<std::shared_ptr<CChapter>> chapters;

public:
  CManga(const Manga &manga);
  CManga(const Manga &manga, CExtension &extension);
  ~CManga();
};

#endif  // NONBIRI_MODELS_MANGA_H_