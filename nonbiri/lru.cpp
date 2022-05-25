#include <mutex>

#include <nonbiri/lru.h>
#include <nonbiri/models/manga.h>

template class LRU<Manga>;

template<class T>
LRU<T>::LRU(unsigned int maxSize) : mMaxSize {maxSize}
{
}

template<class T>
LRU<T>::~LRU()
{
}

template<class T>
std::shared_ptr<T> LRU<T>::get(const std::string &key)
{
  std::shared_lock lock(mutex);
  const auto it = cache.find(key);
  if (it == cache.end())
    return nullptr;

  keys.remove(key);
  keys.push_front(key);

  return it->second;
}

template<class T>
void LRU<T>::set(const std::string &key, std::shared_ptr<T> value)
{
  std::lock_guard lock(mutex);
  const auto it = cache.find(key);
  if (it != cache.end())
    keys.remove(key);

  cache[key] = value;
  keys.push_front(key);

  while (cache.size() > mMaxSize) {
    const auto last = keys.back();
    keys.pop_back();
    cache.erase(last);
  }
}

template<class T>
bool LRU<T>::has(const std::string &key)
{
  std::shared_lock lock(mutex);
  return cache.find(key) != cache.end();
}

template<class T>
void LRU<T>::remove(const std::string &key)
{
  std::lock_guard lock(mutex);
  const auto it = cache.find(key);
  if (it != cache.end()) {
    keys.remove(key);
    cache.erase(it);
  }
}

template<class T>
void LRU<T>::clear()
{
  std::lock_guard lock(mutex);
  cache.clear();
  keys.clear();
}