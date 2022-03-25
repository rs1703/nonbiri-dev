#include <iostream>

#include <core/utils/utils.h>
#include <nonbiri/models/chapter.h>
#include <nonbiri/models/extension.h>
#include <nonbiri/models/manga.h>

CManga::CManga(const Manga &manga) : Manga {manga} {}

CManga::~CManga()
{
  std::cout << "CManga::~CManga()" << std::endl;
}