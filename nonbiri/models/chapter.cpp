#include <iostream>

#include <nonbiri/models/chapter.h>
#include <nonbiri/models/manga.h>

using std::make_shared;
using std::shared_ptr;

CChapter::CChapter(const Chapter &chapter) : Chapter(chapter) {}
CChapter::CChapter(const CChapter &chapter) : Chapter(chapter) {}

CChapter::CChapter(const CManga &manga, const Chapter &chapter) : Chapter(chapter), manga(make_shared<CManga>(manga)) {}

CChapter::~CChapter()
{
  std::cout << "CChapter::~CChapter()" << std::endl;
}

shared_ptr<CManga> CChapter::getManga() const
{
  return manga;
}

void CChapter::setManga(const CManga &manga)
{
  this->manga = make_shared<CManga>(manga);
}