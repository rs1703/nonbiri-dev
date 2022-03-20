#ifndef NONBIRI_MODELS_CHAPTER_H_
#define NONBIRI_MODELS_CHAPTER_H_

#include <memory>

#include <core/models/models.h>
#include <nonbiri/models/extension.h>

class CManga;

class CChapter : public Chapter
{
public:
  std::shared_ptr<CManga> manga;

public:
  CChapter(const Chapter &chapter);
  CChapter(const Chapter &chapter, CManga &manga);
  ~CChapter();
};

#endif  // NONBIRI_MODELS_CHAPTER_H_