#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <core/utils/utils.h>
#include <json/json.h>
#include <nonbiri/main.h>
#include <nonbiri/manager.h>
#include <nonbiri/models/chapter.h>
#include <nonbiri/utils/utils.h>
#include <string.h>

using std::cout;
using std::endl;
using std::exception;
using std::make_pair;
using std::make_shared;
using std::make_tuple;
using std::map;
using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::vector;

namespace fs = std::filesystem;
typedef Extension *(*createPtr)(void);

static const string dataBaseUrl {"https://raw.githubusercontent.com/rs1703/nonbiri-extensions-dev/releases"};

shared_ptr<CExtension> createExtension(void *handle)
{
  auto create = (createPtr)utils::getSymbol(handle, "create");
  if (create == nullptr)
    throw runtime_error("Unable to get fn symbol 'create' from extension");

  auto extension = create();
  if (extension == nullptr)
    throw runtime_error("Unable to instantiate extension");

  auto ext = make_shared<CExtension>(*extension);
  ext->setHandle(handle);
  return ext;
}

tuple<vector<shared_ptr<CManga>>, bool> normalizeMangaEntries(shared_ptr<CExtension> ext,
                                                              tuple<vector<Manga *>, bool> &tuple)
{
  vector<shared_ptr<CManga>> entries;
  for (auto entry : std::get<0>(tuple))
    entries.push_back(make_shared<CManga>(*entry, *ext));
  return make_tuple(entries, std::get<1>(tuple));
}

vector<shared_ptr<CChapter>> normalizeChapterEntries(shared_ptr<CExtension> ext,
                                                     CManga &manga,
                                                     vector<Chapter *> &chapters)
{
  vector<shared_ptr<CChapter>> entries;
  for (auto entry : chapters)
    entries.push_back(make_shared<CChapter>(*entry, manga));
  return entries;
}

Manager::Manager(const string &path)
{
  auto paths = getExtensions(path);
  for (auto path : paths) {
    try {
      loadExtension(path);
    } catch (std::exception &e) {
      cout << e.what() << endl;
    }
  }
}

Manager::~Manager()
{
  cout << "Manager::~Manager()" << endl;
}

void Manager::reset()
{
  currentExtension = NULL;
  currentQuery = NULL;
}

shared_ptr<CExtension> Manager::getExtension(const string &name) const
{
  auto it = extensions.find(name);
  if (it == extensions.end())
    return nullptr;
  return it->second;
}

static const string extensionsDir {"extensions"};

shared_ptr<CExtension> Manager::loadExtension(const string &name)
{
  string path {fs::path(extensionsDir) / fs::path(name)};
  if (!fs::exists(path))
    throw runtime_error("Extension not found");

  if (extensions.find(name) != extensions.end())
    throw runtime_error("Extension already loaded");

  cout << "Loading " << name << "..." << endl;
  auto handle = utils::loadLibrary(path.c_str());
  if (handle == nullptr)
    throw runtime_error("Unable to load extension");

  auto ext = createExtension(handle);
  ext->libName = name;
  extensions.insert(make_pair(name, ext));
  cout << "Loaded " << name << endl;
  return ext;
}

void Manager::unloadExtension(const string &name)
{
  auto it = extensions.find(name);
  if (it == extensions.end())
    throw runtime_error("Extension not loaded");

  cout << "Unloading " << name << "..." << endl;
  utils::freeLibrary(it->second->getHandle());
  extensions.erase(it);
  cout << "Unloaded " << name << endl;
}

shared_ptr<CExtension> Manager::downloadExtension(const string &name, bool rewrite)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  string url {dataBaseUrl + "/windows/" + name};
#else
  string url {dataBaseUrl + "/linux/" + name};
#endif

  if (!fs::exists(extensionsDir))
    fs::create_directory(extensionsDir);

  string outPath {fs::path(extensionsDir) / fs::path(name)};
  if (fs::exists(outPath)) {
    if (!rewrite) {
      auto ext = getExtension(name);
      if (ext != nullptr)
        return ext;
      return loadExtension(name);
    }
    fs::remove(outPath);
  }

  cout << "Downloading " << name << "..." << endl;
  auto code = http::download(url.c_str(), outPath.c_str());
  if (code != 200)
    throw runtime_error("Unable to download extension");
  return loadExtension(name);
}

tuple<vector<shared_ptr<CManga>>, bool> Manager::getLatests(const string &name, int page)
{
  auto ext = getExtension(name);
  if (ext == nullptr)
    throw runtime_error("Extension not loaded");
  return getLatests(ext, page);
}

void Manager::setCurrentExtension(shared_ptr<CExtension> ext)
{
  if (currentExtension != nullptr && ext != nullptr && currentExtension->libName == ext->libName)
    return;

  reset();
  currentExtension = ext;
}

void Manager::setCurrentQuery(const char *query)
{
  currentQuery = query;
  currentPage = 1;
  hasNext = true;
}

vector<shared_ptr<CManga>> Manager::getLatests()
{
  if (currentExtension == nullptr)
    throw runtime_error("Extension not set");

  if (!hasNext)
    return {};

  auto [entries, hasNext] = getLatests(currentExtension, currentPage);
  this->hasNext = hasNext;
  currentPage++;

  return entries;
}

vector<shared_ptr<CManga>> Manager::searchManga(const char *query)
{
  if (currentExtension == nullptr)
    throw runtime_error("Extension not set");

  if (strcmp(query, currentQuery) != 0)
    setCurrentQuery(query);

  if (!hasNext)
    return {};

  auto [entries, hasNext] = searchManga(currentExtension, currentPage, query);
  this->hasNext = hasNext;
  currentPage++;

  return entries;
}

shared_ptr<CManga> Manager::getManga(const string &url)
{
  if (currentExtension == nullptr)
    throw runtime_error("Extension not set");
  return getManga(currentExtension, url);
}

vector<shared_ptr<CChapter>> Manager::getChapters(CManga &manga)
{
  if (currentExtension == nullptr)
    throw runtime_error("Extension not set");
  return getChapters(currentExtension, manga);
}

vector<string> Manager::getPages(Chapter &chapter)
{
  if (currentExtension == nullptr)
    throw runtime_error("Extension not set");
  return getPages(currentExtension, chapter);
}

tuple<vector<shared_ptr<CManga>>, bool> Manager::getLatests(shared_ptr<CExtension> ext, int page)
{
  auto res = ext->latestsRequest(page);
  if (res.empty())
    throw runtime_error("No results");

  if (ext->useApi) {
    auto result = ext->parseLatestEntries(res);
    return normalizeMangaEntries(ext, result);
  }

  CHtml html {res};
  try {
    auto result = ext->parseLatestEntries(html);
    return normalizeMangaEntries(ext, result);
  } catch (...) {
    // ignore
  }

  auto selector = ext->latestsSelector();
  auto entries = html.select(selector);

  vector<shared_ptr<CManga>> result;
  for (auto &entry : entries) {
    auto manga = ext->parseLatestEntry(*entry);
    if (manga != nullptr)
      result.push_back(make_shared<CManga>(*manga, *ext));
  }

  auto nextSelector = ext->latestsNextSelector();
  bool hasNext = false;

  if (!nextSelector.empty()) {
    auto next = html.selectFirst(nextSelector);
    if (next)
      hasNext = next->isValid();
  }
  return make_tuple(result, hasNext);
}

tuple<vector<shared_ptr<CManga>>, bool> Manager::searchManga(const string &name, int page, const char *query)
{
  auto ext = getExtension(name);
  if (ext == nullptr)
    throw runtime_error("Extension not loaded");
  return searchManga(ext, page, query);
}

tuple<vector<shared_ptr<CManga>>, bool> Manager::searchManga(shared_ptr<CExtension> ext, int page, const char *query)
{
  auto res = ext->searchMangaRequest(page, query);
  if (res.empty())
    throw runtime_error("No results");

  if (ext->useApi) {
    auto result = ext->parseSearchEntries(res);
    return normalizeMangaEntries(ext, result);
  }

  CHtml html {res};
  try {
    auto result = ext->parseSearchEntries(html);
    return normalizeMangaEntries(ext, result);
  } catch (...) {
    // ignore
  }

  auto selector = ext->searchMangaSelector();
  auto entries = html.select(selector);
  vector<shared_ptr<CManga>> result;

  for (auto &entry : entries) {
    auto manga = ext->parseSearchEntry(*entry);
    if (manga != nullptr)
      result.push_back(make_shared<CManga>(*manga, *ext));
  }

  auto nextSelector = ext->searchMangaNextSelector();
  bool hasNext = false;

  if (!nextSelector.empty()) {
    auto next = html.selectFirst(nextSelector);
    if (next)
      hasNext = next->isValid();
  }
  return make_tuple(result, hasNext);
}

shared_ptr<CManga> Manager::getManga(const string &name, const string &url)
{
  auto ext = getExtension(name);
  if (ext == nullptr)
    throw runtime_error("Extension not loaded");
  return getManga(ext, url);
}

shared_ptr<CManga> Manager::getManga(shared_ptr<CExtension> ext, const string &url)
{
  auto res = http::get(url);
  if (res.empty())
    throw runtime_error("No results");

  if (ext->useApi) {
    auto manga = ext->parseManga(res);
    if (manga == nullptr)
      throw runtime_error("No results");
    return make_shared<CManga>(*manga, *ext);
  }

  CHtml html {res};
  auto manga = ext->parseManga(html);
  if (manga == nullptr)
    throw runtime_error("No results");
  return make_shared<CManga>(*manga, *ext);
}

vector<shared_ptr<CChapter>> Manager::getChapters(const string &name, CManga &manga)
{
  auto ext = getExtension(name);
  if (ext == nullptr)
    throw runtime_error("Extension not loaded");
  return getChapters(ext, manga);
}

vector<shared_ptr<CChapter>> Manager::getChapters(shared_ptr<CExtension> ext, CManga &manga)
{
  auto res = ext->chaptersRequest(manga);
  if (res.empty())
    throw runtime_error("No results");

  if (ext->useApi) {
    auto result = ext->parseChapterEntries(manga, res);
    return normalizeChapterEntries(ext, manga, result);
  }

  CHtml html {res};
  try {
    auto result = ext->parseChapterEntries(manga, html);
    return normalizeChapterEntries(ext, manga, result);
  } catch (...) {
    // ignore
  }

  auto selector = ext->chaptersSelector();
  auto entries = html.select(selector);
  vector<shared_ptr<CChapter>> result;

  for (auto &entry : entries) {
    auto chapter = ext->parseChapterEntry(manga, *entry);
    if (chapter != nullptr)
      result.push_back(make_shared<CChapter>(*chapter, manga));
  }
  return result;
}

vector<string> Manager::getPages(const string &name, Chapter &chapter)
{
  auto ext = getExtension(name);
  if (ext == nullptr)
    throw runtime_error("Extension not loaded");
  return getPages(ext, chapter);
}

vector<string> Manager::getPages(shared_ptr<CExtension> ext, Chapter &chapter)
{
  auto res = ext->pagesRequest(chapter);
  if (res.empty())
    throw runtime_error("No results");

  if (ext->useApi)
    return ext->parsePages(chapter, res);

  CHtml html {res};
  return ext->parsePages(chapter, html);
}

const vector<string> Manager::getExtensions(const string &path)
{
  if (!fs::exists(path))
    return {};

  vector<string> result;
  for (const auto &dirEntry : fs::recursive_directory_iterator(path))
    if (dirEntry.is_regular_file())
      result.push_back(dirEntry.path().filename().string());
  return result;
}

const map<string, ExtensionInfo> Manager::fetchExtensions()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  static string url {dataBaseUrl + "/windows.json"};
#else
  static string url {dataBaseUrl + "/linux.json"};
#endif

  auto res = http::get(url);
  if (res.empty())
    throw runtime_error("No results");

  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(res, root))
    throw runtime_error("Unable to parse extensions index: " + reader.getFormattedErrorMessages());

  map<string, ExtensionInfo> result;
  for (auto &k : root.getMemberNames()) {
    auto v = root[k];
    result[k] = ExtensionInfo {
        .baseUrl = v["baseUrl"].asString(),
        .name = v["name"].asString(),
        .language = v["language"].asString(),
        .version = v["version"].asString(),
    };
  }

  return result;
}
