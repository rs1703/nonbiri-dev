#ifndef NONBIRI_CACHE_H_
#define NONBIRI_CACHE_H_

#include <memory>

#include <nonbiri/lru.h>
#include <nonbiri/models/manga.h>

namespace Cache
{

extern LRU<std::shared_ptr<Manga>> manga;
extern LRU<std::shared_ptr<Chapter>> chapter;
extern LRU<std::vector<std::shared_ptr<Chapter>>> chapters;

};  // namespace Cache

#endif  // NONBIRI_CACHE_H_