#include <iostream>

#include <nonbiri/models/manga.h>

CManga::CManga(const Manga &manga) : Manga(manga) {}
CManga::CManga(const Manga &manga, CExtension &extension)
    : Manga(manga), extension(std::make_shared<CExtension>(extension))
{
}

CManga::~CManga()
{
  // std::cout << "CManga::~CManga()" << std::endl;
}