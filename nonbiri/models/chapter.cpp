#include <iostream>

#include <nonbiri/models/chapter.h>
#include <nonbiri/models/manga.h>

CChapter::CChapter(const Chapter &chapter) : Chapter {chapter} {}

CChapter::~CChapter()
{
  std::cout << "CChapter::~CChapter()" << std::endl;
}
