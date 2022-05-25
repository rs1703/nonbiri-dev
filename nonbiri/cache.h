#ifndef NONBIRI_CACHE_H_
#define NONBIRI_CACHE_H_

#include <nonbiri/lru.h>
#include <nonbiri/models/manga.h>

namespace Cache
{

extern LRU<Manga> manga;

};  // namespace Cache

#endif  // NONBIRI_CACHE_H_