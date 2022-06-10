#include <nonbiri/cache.h>

LRU<std::shared_ptr<Manga>> Cache::manga(256);
LRU<std::shared_ptr<Chapter>> Cache::chapter(128);
LRU<std::vector<std::shared_ptr<Chapter>>> Cache::chapters(8);
