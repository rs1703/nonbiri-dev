#ifndef NONBIRI_MODELS_MANGA_H_
#define NONBIRI_MODELS_MANGA_H_

#include <map>
#include <memory>
#include <string>

#include <core/models/models.h>

class CManga : public Manga
{
public:
  CManga(const Manga &manga);
  ~CManga();
};

using MangaPtr = std::shared_ptr<CManga>;

#endif  // NONBIRI_MODELS_MANGA_H_