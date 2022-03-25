#ifndef NONBIRI_LRU_H_
#define NONBIRI_LRU_H_

#include <list>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

template<class T>
class LRU
{
  const unsigned int mMaxSize;
  std::shared_mutex mutex;

  std::map<std::string, std::shared_ptr<T>> cache;
  std::list<std::string> keys;

public:
  LRU(unsigned int maxSize);
  ~LRU();

  std::shared_ptr<T> get(const std::string &key);
  void set(const std::string &key, std::shared_ptr<T> value);
  bool has(const std::string &key);

  void remove(const std::string &key);
  void clear();
};

#endif  // NONBIRI_LRU_H_