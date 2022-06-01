#include <cstdio>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <core/utils.h>
#include <json/json.h>
#include <nonbiri/cache.h>
#include <nonbiri/manager.h>
#include <nonbiri/utility.h>

namespace fs = std::filesystem;
typedef Extension *(*createExtFn)(void);

static const std::string dataBaseUrl {"https://raw.githubusercontent.com/rs1703/nonbiri-extensions-dev/releases"};
Manager *App::manager = nullptr;

Extension *createExtension(void *handle)
{
  auto createExt = (createExtFn)utils::getSymbol(handle, "create");
  if (createExt == nullptr)
    throw std::runtime_error("Unable to get fn symbol 'create' from extension");
  Extension *extension = createExt();
  if (extension == nullptr)
    throw std::runtime_error("Unable to create extension");
  return extension;
}

Manager::Manager(const std::string &dir) : extensionsDir {dir}
{
  std::cout << "Initializing manager..." << std::endl;
  const std::vector<std::string> paths = getLocalExtensionPaths();
  for (const std::string &path : paths) {
    try {
      loadExtension(path);
    } catch (const std::exception &e) {
      std::cerr << "Error loading extension from " << path << ": " << e.what() << std::endl;
    }
  }
}

Manager::~Manager()
{
  std::cout << "Manager::~Manager()" << std::endl;
}

Extension *Manager::getExtension(const std::string &id)
{
  std::shared_lock lock {mExtensionsMutex};
  auto it = mExtensions.find(id);
  if (it == mExtensions.end())
    return nullptr;
  return it->second;
}

const ExtensionMap &Manager::getExtensions()
{
  std::shared_lock lock(mExtensionsMutex);
  return mExtensions;
}

const std::shared_ptr<ExtensionInfo> Manager::getExtensionInfo(const std::string &id)
{
  std::shared_lock lock(mIndexesMutex);
  auto it = mIndexes.find(id);
  if (it == mIndexes.end())
    return nullptr;
  return std::make_shared<ExtensionInfo>(it->second);
}

const ExtensionInfoMap &Manager::getIndexes()
{
  std::shared_lock lock(mIndexesMutex);
  return mIndexes;
}

#define LOAD_EXTENSION \
  void *handle = utils::loadLibrary(path.string()); \
  if (handle == nullptr) \
    throw std::runtime_error("Unable to load extension"); \
\
  Extension *ext = createExtension(handle); \
  if (mExtensions.find(ext->id) != mExtensions.end()) { \
    utils::freeLibrary(handle); \
    throw std::runtime_error("Extension already loaded"); \
  } \
\
  const auto info = getExtensionInfo(ext->id); \
  ext->hasUpdate = info != nullptr && ext->version != info->version; \
\
  mExtensions.insert(std::make_pair(ext->id, ext)); \
  std::cout << "Loaded " << ext->id << std::endl;

void Manager::loadExtension(const std::string &name)
{
  std::lock_guard lock(mExtensionsMutex);
  std::cout << "Loading extension: " << name << "..." << std::endl;
  const fs::path path {fs::path(extensionsDir) / name};
  if (!fs::exists(path))
    throw std::runtime_error("Extension not found");

  LOAD_EXTENSION;
}

void Manager::unloadExtension(const std::string &id)
{
  std::lock_guard lock(mExtensionsMutex);
  std::cout << "Unloading " << id << "..." << std::endl;

  auto it = mExtensions.find(id);
  if (it == mExtensions.end())
    throw std::runtime_error("Extension not loaded");

  delete it->second;
  mExtensions.erase(it);
  std::cout << "Unloaded " << id << std::endl;
}

void Manager::downloadExtension(const std::string &id, bool update)
{
  std::lock_guard lock(mExtensionsMutex);
  if (getExtensionInfo(id) == nullptr)
    throw std::runtime_error("Extension not found");

  auto it = mExtensions.find(id);
  if (!update && it != mExtensions.end())
    throw std::runtime_error("Extension already installed");

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  const std::string name {id + ".dll"};
  const std::string url {dataBaseUrl + "/windows/" + name};
#else
  const std::string name {"lib" + id + ".so"};
  const std::string url {dataBaseUrl + "/linux/" + name};
#endif
  int code;

  if (!fs::exists(extensionsDir))
    fs::create_directory(extensionsDir);

  const fs::path path = fs::path(extensionsDir) / name;
  if (fs::exists(path)) {
    if (!update)
      return;
    if (it != mExtensions.end())
      mExtensions.erase(it);
    fs::remove(path);
  }

  std::cout << "Downloading " << name << "..." << std::endl;
  code = http::download(url, path.string());
  if (code != 200)
    throw std::runtime_error("Unable to download extension");

  std::cout << "Loading " << name << "..." << std::endl;
  LOAD_EXTENSION;
}

void Manager::removeExtension(const std::string &id, fs::path path)
{
  std::lock_guard lock(mExtensionsMutex);

  if (getExtensionInfo(id) == nullptr)
    throw std::runtime_error("Extension not found");

  auto it = mExtensions.find(id);
  if (it != mExtensions.end())
    mExtensions.erase(it);

  if (path.empty()) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    const std::string name {id + ".dll"};
#else
    const std::string name {"lib" + id + ".so"};
#endif
    path = fs::absolute(fs::path(extensionsDir) / name);
  }

  if (fs::exists(path))
    fs::remove(path);
}

void Manager::updateExtension(const std::string &id)
{
  downloadExtension(id, true);
}

void Manager::updateExtensionIndexes()
{
  std::lock_guard lock(mIndexesMutex);

  const time_t now {time(nullptr)};
  const double minutes {difftime(now, indexLastUpdated) / 60.0};

  if (minutes > 0 && minutes <= 10.0)
    return;
  indexLastUpdated = now;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  static const std::string url {dataBaseUrl + "/windows.json"};
#else
  static const std::string url {dataBaseUrl + "/linux.json"};
#endif

  const std::string res = http::get(url);
  if (res.empty())
    throw std::runtime_error("No results");

  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(res, root))
    throw std::runtime_error("Unable to parse extensions index: " + reader.getFormattedErrorMessages());

  for (const std::string &key : root.getMemberNames()) {
    const Json::Value json = root[key];
    ExtensionInfo info {
      json["id"].asString(),
      json["baseUrl"].asString(),
      json["name"].asString(),
      json["language"].asString(),
      json["version"].asString(),
    };
    mIndexes.insert(std::make_pair(info.id, info));

    Extension *ext = getExtension(info.id);
    if (ext != nullptr)
      ext->hasUpdate = ext->version != info.version;
  }
}

std::tuple<std::vector<std::shared_ptr<Manga>>, bool> Manager::getLatests(Extension &ext, int page)
{
  utils::ExecTime execTime("Manager::getLatests(ext, page)");
  const auto &[entries, hasNext] = ext.getLatests(page);

  std::vector<std::shared_ptr<Manga>> manga;
  for (const Manga_t *e : entries) {
    std::shared_ptr<Manga> entry = std::make_shared<Manga>(ext.id, *e);
    entry->getReadState();
    manga.push_back(entry);
    delete e;
  }
  return {manga, hasNext};
}

std::tuple<std::vector<std::shared_ptr<Manga>>, bool> Manager::searchManga(Extension &ext,
                                                                           int page,
                                                                           const std::string &query,
                                                                           const std::vector<FilterKV> &filters)
{
  utils::ExecTime execTime("Manager::searchManga(ext, page, query, filters)");
  const auto &[entries, hasNext] = ext.searchManga(page, query, filters);

  std::vector<std::shared_ptr<Manga>> manga;
  for (const Manga_t *e : entries) {
    std::shared_ptr<Manga> entry = std::make_shared<Manga>(ext.id, *e);
    entry->getReadState();
    manga.push_back(entry);
    delete e;
  }
  return {manga, hasNext};
}

std::shared_ptr<Manga> Manager::getManga(Extension &ext, const std::string &path)
{
  utils::ExecTime execTime("Manager::getManga(ext, path)");
  const std::string cacheKey {ext.id + path};
  if (Cache::manga.has(cacheKey)) {
    std::shared_ptr<Manga> manga = Cache::manga.get(cacheKey);
    if (manga->id > 0)
      Cache::manga.remove(cacheKey);
    else
      return manga;
  }

  try {
    const std::shared_ptr<Manga> manga = Manga::find(ext.id, path);
    if (manga != nullptr)
      return manga;
  } catch (const std::exception &e) {
    std::cerr << "Unable to get manga: " << e.what() << std::endl;
  }

  std::shared_ptr<Manga> manga = nullptr;
  try {
    const Manga_t *m = ext.getManga(path);
    if (m != nullptr) {
      manga = std::make_shared<Manga>(ext.id, *m);
      delete m;
    }
  } catch (const std::exception &e) {
    std::cerr << "Unable to fetch manga: " << e.what() << std::endl;
  }

  if (manga != nullptr)
    Cache::manga.set(cacheKey, manga);
  return manga;
}

std::vector<std::shared_ptr<Chapter>> Manager::getChapters(Extension &ext, const std::string &path)
{
  utils::ExecTime execTime("Manager::getChapters(ext, path)");
  std::shared_ptr<Manga> manga = getManga(ext, path);
  if (manga == nullptr)
    throw std::runtime_error("Unable to get manga");
  return getChapters(ext, *manga);
}

std::vector<std::shared_ptr<Chapter>> Manager::getChapters(Extension &ext, Manga &manga)
{
  utils::ExecTime execTime("Manager::getChapters(ext, manga)");
  try {
    std::vector<std::shared_ptr<Chapter>> chapters = manga.getChapters();
    if (!chapters.empty())
      return chapters;
  } catch (const std::exception &e) {
    std::cerr << "Unable to get chapters: " << e.what() << std::endl;
  }

  const std::string cacheKey {ext.id + manga.path};
  if (Cache::chapters.has(cacheKey)) {
    std::vector<std::shared_ptr<Chapter>> chapters = Cache::chapters.get(cacheKey);
    if (manga.id > 0) {
      Chapter::saveAll(chapters, manga.id);
      Cache::chapters.remove(cacheKey);
      return getChapters(ext, manga);
    }
    return chapters;
  }

  std::vector<std::shared_ptr<Chapter>> chapters {};
  try {
    const std::vector<Chapter_t *> entries = ext.getChapters(manga.path);
    for (const Chapter_t *e : entries) {
      std::shared_ptr<Chapter> entry = std::make_shared<Chapter>(manga.id, ext.id, *e);
      chapters.push_back(entry);
      delete e;
    }
  } catch (const std::exception &e) {
    std::cerr << "Unable to fetch chapters: " << e.what() << std::endl;
  }

  if (manga.id > 0)
    Chapter::saveAll(chapters);
  else if (!chapters.empty())
    Cache::chapters.set(cacheKey, chapters);
  return chapters;
}

std::vector<std::string> Manager::getPages(Extension &ext, const std::string &path)
{
  utils::ExecTime execTime("Manager::getPages(ext, path)");
  return ext.getPages(path);
}

std::vector<std::string> Manager::getLocalExtensionPaths()
{
  if (!fs::exists(extensionsDir))
    return {};
  std::vector<std::string> result;
  for (const fs::directory_entry &dirEntry : fs::recursive_directory_iterator(extensionsDir))
    if (dirEntry.is_regular_file())
      result.push_back(dirEntry.path().filename().string());
  return result;
}
