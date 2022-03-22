#include <nonbiri/cache/lru.h>
#include <nonbiri/models/chapter.h>
#include <nonbiri/models/extension.h>
#include <nonbiri/models/manga.h>

using std::lock_guard;
using std::mutex;
using std::shared_ptr;
using std::string;

template class LRU<CManga>;

template<class T>
LRU<T>::LRU(unsigned int maxSize) : maxSize(maxSize)
{
}

template<class T>
LRU<T>::~LRU()
{
}

template<class T>
shared_ptr<T> LRU<T>::get(const string &key)
{
  lock_guard<mutex> lock(mtx);

  auto it = cache.find(key);
  if (it == cache.end())
    return nullptr;

  keys.remove(key);
  keys.push_front(key);

  return it->second;
}

template<class T>
void LRU<T>::set(const string &key, shared_ptr<T> value)
{
  lock_guard<mutex> lock(mtx);

  auto it = cache.find(key);
  if (it != cache.end())
    keys.remove(key);

  cache[key] = value;
  keys.push_front(key);

  while (cache.size() > maxSize) {
    auto last = keys.back();
    keys.pop_back();
    cache.erase(last);
  }
}

template<class T>
bool LRU<T>::has(const string &key)
{
  lock_guard<mutex> lock(mtx);
  return cache.find(key) != cache.end();
}

template<class T>
void LRU<T>::remove(const string &key)
{
  lock_guard<mutex> lock(mtx);
  auto it = cache.find(key);
  if (it != cache.end()) {
    keys.remove(key);
    cache.erase(it);
  }
}

template<class T>
void LRU<T>::clear()
{
  lock_guard<mutex> lock(mtx);
  cache.clear();
  keys.clear();
}