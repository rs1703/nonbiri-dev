#ifndef NONBIRI_LIBRARY_H_
#define NONBIRI_LIBRARY_H_

#include <memory>
#include <string>

#include <nonbiri/models/manga.h>

struct Library
{
  static std::shared_ptr<Manga> getManga(int64_t id);
  static std::shared_ptr<Manga> getManga(const std::string &sourceId, const std::string &path);
  static bool hasManga(Manga &const manga);
  static bool hasManga(const std::string &sourceId, const std::string &path);

  static void addManga(Manga &const manga);
  static void updateManga(Manga &const manga);
  static void removeManga(int64_t id);
};

#endif  // NONBIRI_LIBRARY_H_