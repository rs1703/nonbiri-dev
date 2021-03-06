#include <cstdio>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <core/core.h>
#include <core/http/http.h>
#include <json/json.h>
#include <nonbiri/cache.h>
#include <nonbiri/manager.h>
#include <nonbiri/utility.h>

namespace fs = std::filesystem;
static const std::string dataBaseUrl {
  "https://raw.githubusercontent.com/rs1703/nonbiri-extensions-dev/releases",
};
Manager *App::manager {nullptr};

Extension *createExtension(void *handle)
{
  auto initialize = (Core::initialize_t)Utils::getSymbol(handle, "initialize");
  if (initialize == nullptr)
    throw std::runtime_error("Unable to get fn symbol 'initialize' from extension");

  initialize(
    Http::init, Http::setOpt, Http::perform, Http::cleanup, Http::getInfo, Http::slist_append, Http::slist_freeAll, Http::getError);

  auto create = (create_t)Utils::getSymbol(handle, "create");
  if (create == nullptr)
    throw std::runtime_error("Unable to get fn symbol 'create' from extension");

  auto extension = create();
  if (extension == nullptr)
    throw std::runtime_error("Unable to create extension");

  return extension;
}

Manager::Manager(const std::string &dir) : extensionsDir {dir}
{
  std::cout << "Initializing manager..." << std::endl;
  const auto paths = getLocalExtensionPaths();
  for (const auto &path : paths) {
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

Extension *Manager::getExtension(const std::string &domain)
{
  std::shared_lock lock(extensionsMutex);
  auto it = extensions.find(domain);
  if (it == extensions.end())
    return nullptr;
  return it->second;
}

const std::map<std::string, Extension *> &Manager::getExtensions()
{
  std::shared_lock lock(extensionsMutex);
  return extensions;
}

const ExtensionInfo *Manager::getExtensionInfo(const std::string &domain)
{
  std::shared_lock lock(indexesMutex);
  auto it = indexes.find(domain);
  if (it == indexes.end())
    return nullptr;
  return &it->second;
}

const std::map<std::string, ExtensionInfo> &Manager::getIndexes()
{
  std::shared_lock lock(indexesMutex);
  return indexes;
}

void Manager::loadExtension(const std::string &path)
{
  std::lock_guard lock(extensionsMutex);
  std::lock_guard lock2(handlesMutex);

  std::cout << "Loading " << path << std::endl;
  if (!fs::exists(path))
    throw std::runtime_error("Extension not found");

  auto handle = Utils::loadLibrary(path);
  if (handle == nullptr)
    throw std::runtime_error("Unable to load extension");

  auto ext = createExtension(handle);
  if (extensions.find(ext->domain) != extensions.end()) {
    Utils::freeLibrary(handle);
    throw std::runtime_error("Extension already loaded");
  }

  const auto info = getExtensionInfo(ext->domain);
  ext->hasUpdate.store(info != nullptr && ext->version != info->version);

  extensions.insert(std::make_pair(ext->domain, ext));
  handles.insert(std::make_pair(ext->domain, handle));
  std::cout << "Loaded " << ext->name << " v" << ext->version << std::endl;
}

void Manager::unloadExtension(const std::string &domain)
{
  std::lock_guard lock(extensionsMutex);
  std::lock_guard lock2(handlesMutex);

  auto ext = extensions.find(domain);
  if (ext == extensions.end())
    throw std::runtime_error("Extension not loaded");

  const auto name = ext->second->name;
  const auto version = ext->second->version;
  std::cout << "Unloading " << name << " v" << version << std::endl;

  auto handle = handles.find(domain);
  if (handle == handles.end())
    throw std::runtime_error("Extension not loaded");

  delete ext->second;
  extensions.erase(ext);

  Utils::freeLibrary(handle->second);
  handles.erase(handle);
  std::cout << "Unloaded " << name << " v" << version << std::endl;
}

void Manager::downloadExtension(const std::string &domain, bool update)
{
  static std::mutex mutex;
  std::lock_guard lock(mutex);

  if (!fs::exists(extensionsDir))
    fs::create_directory(extensionsDir);

  const auto info = getExtensionInfo(domain);
  if (info == nullptr)
    throw std::runtime_error("Extension not found");

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  const std::string fileName {domain + ".dll"};
#else
  const std::string fileName {domain + ".so"};
#endif

  const auto path = (fs::path(extensionsDir) / fileName).string();
  if (fs::exists(path)) {
    if (!update)
      throw std::runtime_error("Extension already installed");
    removeExtension(domain);
  }

  std::cout << "Downloading " << info->name << "..." << std::endl;
  int code = Http::download(dataBaseUrl + "/" + info->path, path);
  if (code != 200)
    throw std::runtime_error("Unable to download extension");

  loadExtension(path);
}

void Manager::removeExtension(const std::string &domain, fs::path path)
{
  if (getExtensionInfo(domain) == nullptr)
    throw std::runtime_error("Extension not found");

  try {
    unloadExtension(domain);
  } catch (const std::exception &e) {
    std::cerr << "Error unloading extension: " << e.what() << std::endl;
  }

  if (path.empty()) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    const std::string localName {domain + ".dll"};
#else
    const std::string localName {domain + ".so"};
#endif
    path = fs::absolute(fs::path(extensionsDir) / localName);
  }

  if (fs::exists(path))
    fs::remove(path);
}

int Manager::downloadIcon(const std::string &fileName, const std::string &outputPath)
{
  if (!fs::exists("icons"))
    fs::create_directory("icons");
  const std::string url = dataBaseUrl + "/icons/" + fileName;
  return Http::download(url, outputPath);
}

void Manager::updateExtension(const std::string &domain)
{
  downloadExtension(domain, true);
}

void Manager::updateExtensionIndexes()
{
  std::lock_guard lock(indexesMutex);

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

  const auto res = Http::get(url);
  if (res->body.empty())
    throw std::runtime_error("Unable to download extension index");

  Json::Value root {};
  Json::Reader reader {};

  if (!reader.parse(res->body, root))
    throw std::runtime_error("Unable to parse extensions index: " + reader.getFormattedErrorMessages());

  for (const auto &domain : root.getMemberNames()) {
    const auto &json = root[domain];
    ExtensionInfo info {};
    info.domain = domain;
    info.name = json["name"].asString();
    info.description = json["description"].asString();
    info.language = json["language"].asString();
    info.version = json["version"].asString();
    info.isNsfw = json["isNsfw"].asBool();
    info.path = json["path"].asString();
    indexes.emplace(domain, info);

    auto ext = getExtension(domain);
    if (ext != nullptr)
      ext->hasUpdate.store(ext->version != info.version);
  }
}

std::tuple<std::vector<std::shared_ptr<Manga>>, bool> Manager::getLatests(Extension &ext, int page)
{
  Utils::ExecTime execTime("Manager::getLatests(ext, page)");
  const auto &[entries, hasNext] = ext.getLatests(page);

  std::vector<std::shared_ptr<Manga>> manga {};
  for (const auto &e : entries) {
    const auto entry = std::make_shared<Manga>(ext.domain, *e);
    entry->getReadState();
    manga.push_back(entry);
  }
  return {manga, hasNext};
}

std::tuple<std::vector<std::shared_ptr<Manga>>, bool> Manager::searchManga(
  Extension &ext, int page, const std::string &query, const std::vector<std::pair<std::string, std::string>> &filters)
{
  Utils::ExecTime execTime("Manager::searchManga(ext, page, query, filters)");
  const auto &[entries, hasNext] = ext.searchManga(page, query, filters);

  std::vector<std::shared_ptr<Manga>> manga {};
  for (const auto &e : entries) {
    const auto entry = std::make_shared<Manga>(ext.domain, *e);
    entry->getReadState();
    manga.push_back(entry);
  }
  return {manga, hasNext};
}

std::shared_ptr<Manga> Manager::getManga(Extension &ext, const std::string &path)
{
  Utils::ExecTime execTime("Manager::getManga(ext, path)");
  const auto cacheKey {ext.domain + path};
  if (Cache::manga.has(cacheKey)) {
    const auto manga = Cache::manga.get(cacheKey);
    if (manga->id > 0)
      Cache::manga.remove(cacheKey);
    else
      return manga;
  }

  try {
    const auto manga = Manga::find(ext.domain, path);
    if (manga != nullptr)
      return manga;
  } catch (const std::exception &e) {
    std::cerr << "Unable to get manga: " << e.what() << std::endl;
  }

  std::shared_ptr<Manga> manga {nullptr};
  try {
    const auto m = ext.getManga(path);
    if (m != nullptr)
      manga = std::make_shared<Manga>(ext.domain, *m);
  } catch (const std::exception &e) {
    std::cerr << "Unable to fetch manga: " << e.what() << std::endl;
  }

  if (manga != nullptr)
    Cache::manga.set(cacheKey, manga);
  return manga;
}

std::vector<std::shared_ptr<Chapter>> Manager::getChapters(Extension &ext, const std::string &path)
{
  Utils::ExecTime execTime("Manager::getChapters(ext, path)");
  auto manga = getManga(ext, path);
  if (manga == nullptr)
    throw std::runtime_error("Unable to get manga");
  return getChapters(ext, *manga);
}

std::vector<std::shared_ptr<Chapter>> Manager::getChapters(Extension &ext, Manga &manga)
{
  Utils::ExecTime execTime("Manager::getChapters(ext, manga)");
  try {
    const auto chapters = manga.getChapters();
    if (!chapters.empty())
      return chapters;
  } catch (const std::exception &e) {
    std::cerr << "Unable to get chapters: " << e.what() << std::endl;
  }

  const auto cacheKey {ext.domain + manga.path};
  if (Cache::chapters.has(cacheKey)) {
    const auto chapters = Cache::chapters.get(cacheKey);
    if (manga.id > 0) {
      Chapter::saveAll(chapters, manga.id);
      Cache::chapters.remove(cacheKey);
      return getChapters(ext, manga);
    }
    return chapters;
  }

  std::vector<std::shared_ptr<Chapter>> chapters {};
  try {
    const auto entries = ext.getChapters(manga.path);
    for (const auto &e : entries) {
      const auto entry = std::make_shared<Chapter>(manga.id, ext.domain, *e);
      chapters.push_back(entry);
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
  Utils::ExecTime execTime("Manager::getPages(ext, path)");
  return ext.getPages(path);
}

std::vector<std::string> Manager::getLocalExtensionPaths()
{
  if (!fs::exists(extensionsDir))
    return {};
  std::vector<std::string> result {};
  for (const auto &dirEntry : fs::recursive_directory_iterator(extensionsDir))
    if (dirEntry.is_regular_file())
      result.push_back(dirEntry.path().string());
  return result;
}
