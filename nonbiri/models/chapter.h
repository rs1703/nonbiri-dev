#ifndef NONBIRI_MODELS_CHAPTER_H_
#define NONBIRI_MODELS_CHAPTER_H_

#include <memory>

#include <core/models/models.h>

class CChapter : public Chapter
{
public:
  CChapter(const Chapter &chapter);
  ~CChapter();
};

using ChapterPtr = std::shared_ptr<CChapter>;

#endif  // NONBIRI_MODELS_CHAPTER_H_