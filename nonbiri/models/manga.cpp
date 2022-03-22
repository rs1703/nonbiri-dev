#include <iostream>

#include <core/utils/utils.h>
#include <nonbiri/models/chapter.h>
#include <nonbiri/models/extension.h>
#include <nonbiri/models/manga.h>

using std::make_shared;
using std::map;
using std::shared_ptr;
using std::string;

CManga::CManga(const Manga &manga) : Manga(manga) {}
CManga::CManga(const CManga &manga) : Manga(manga) {}

CManga::CManga(const CExtension &extension, const Manga &manga)
    : Manga(manga), extension(make_shared<CExtension>(extension))
{
}

CManga::~CManga()
{
  std::cout << "CManga::~CManga()" << std::endl;
}

shared_ptr<CExtension> CManga::getExtension() const
{
  return extension;
}

void CManga::setExtension(const CExtension &extension)
{
  this->extension = make_shared<CExtension>(extension);
}

shared_ptr<CChapter> CManga::getChapter(const string &path)
{
  auto it = chapters.find(stripDomain(path));
  if (it == chapters.end())
    return nullptr;
  return it->second;
}

map<string, shared_ptr<CChapter>> CManga::getChapters() const
{
  return chapters;
}

void CManga::addChapter(const string &path, const CChapter &chapter)
{
  chapters[stripDomain(path)] = make_shared<CChapter>(chapter);
}

void CManga::removeChapter(const string &path)
{
  chapters.erase(stripDomain(path));
}

void CManga::clearChapters()
{
  chapters.clear();
}