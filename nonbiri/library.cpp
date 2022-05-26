#include <stdexcept>

#include <nonbiri/cache.h>
#include <nonbiri/library.h>

std::shared_ptr<Manga> Library::getManga(int64_t id)
{
  auto manga = Manga::find(id);
  if (manga == nullptr)
    return nullptr;
  return std::make_shared<Manga>(*manga);
}

std::shared_ptr<Manga> Library::getManga(const std::string &sourceId, const std::string &path)
{
  auto manga = Manga::find(sourceId, path);
  if (manga == nullptr)
    return nullptr;
  return std::make_shared<Manga>(*manga);
}

bool Library::hasManga(Manga &const manga)
{
  if (manga.id > 0)
    return Manga::exists(manga.id);
  return Manga::exists(manga.sourceId, manga.path);
}

bool Library::hasManga(const std::string &sourceId, const std::string &path)
{
  return Manga::exists(sourceId, path);
}

void Library::addManga(Manga &const manga)
{
  if (manga.id || Manga::exists(manga.sourceId, manga.path))
    manga.update();
  else
    manga.save();
  Cache::manga.set(manga.sourceId + manga.path, std::make_shared<Manga>(manga));
}

void Library::updateManga(Manga &const manga)
{
  addManga(manga);
}

void Library::removeManga(int64_t id)
{
  auto manga = getManga(id);
  if (manga == nullptr)
    return;
  manga->remove();
  Cache::manga.remove(manga->sourceId + manga->path);
}
