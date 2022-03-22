#ifndef NONBIRI_MODELS_CHAPTER_H_
#define NONBIRI_MODELS_CHAPTER_H_

#include <memory>

#include <core/models/models.h>

class CManga;

class CChapter : public Chapter
{
  std::shared_ptr<CManga> manga;

public:
  CChapter(const Chapter &chapter);
  CChapter(const CChapter &chapter);
  CChapter(const CManga &manga, const Chapter &chapter);
  ~CChapter();

  std::shared_ptr<CManga> getManga() const;
  void setManga(const CManga &manga);
};

#endif  // NONBIRI_MODELS_CHAPTER_H_