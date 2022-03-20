#include <iostream>

#include <nonbiri/models/chapter.h>
#include <nonbiri/models/manga.h>

CChapter::CChapter(const Chapter &chapter) : Chapter(chapter) {}
CChapter::CChapter(const Chapter &chapter, CManga &manga) : Chapter(chapter), manga(std::make_shared<CManga>(manga)) {}

CChapter::~CChapter()
{
  // std::cout << "CChapter::~CChapter()" << std::endl;
}