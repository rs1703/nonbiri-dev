#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <core/utils/utils.h>
#include <json/json.h>
#include <nonbiri/manager/manager.h>
#include <nonbiri/models/chapter.h>
#include <nonbiri/utils/utils.h>

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

Manager::Manager(const string &dir) : extensionsDir(dir)
{
  cout << "Initializing manager..." << endl;
  auto paths = getLocalExtensionPaths();
  for (auto path : paths) {
    try {
      loadExtension(path);
    } catch (exception &e) {
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
  currentQuery = "";
}

void Manager::loadExtension(const string &name)
{
  auto path = fs::path(extensionsDir) / name;
  if (!fs::exists(path))
    throw runtime_error("Extension not found");

  if (extensions.find(name) != extensions.end())
    throw runtime_error("Extension already loaded");

  cout << "Loading " << name << "..." << endl;

  auto handle = utils::loadLibrary(path.string());
  if (handle == nullptr)
    throw runtime_error("Unable to load extension");

  auto ext = createExtension(handle);
  extensions.insert(make_pair(name, ext));

  cout << "Loaded " << name << endl;
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

void Manager::downloadExtension(const string &name, bool update)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  string url {dataBaseUrl + "/windows/" + name};
#else
  string url {dataBaseUrl + "/linux/" + name};
#endif
  int code;

  if (!fs::exists(extensionsDir))
    fs::create_directory(extensionsDir);

  auto outPath = fs::path(extensionsDir) / name;
  if (fs::exists(outPath)) {
    if (!update)
      goto end;

    try {
      unloadExtension(name);
    } catch (...) {
    }

    fs::remove(outPath);
  }

  cout << "Downloading " << name << "..." << endl;
  code = http::download(url, outPath.string());
  if (code != 200)
    throw runtime_error("Unable to download extension");

end:
  return loadExtension(name);
}

void Manager::updateExtension(const string &name)
{
  downloadExtension(name, true);
}

void Manager::setCurrentExtension(shared_ptr<CExtension> ext)
{
  if (currentExtension == ext)
    return;

  reset();
  currentExtension = ext;
}

void Manager::setCurrentQuery(const string &query)
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

void Manager::updateExtensionIndexes()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  static const string url {dataBaseUrl + "/windows.json"};
#else
  static const string url {dataBaseUrl + "/linux.json"};
#endif

  auto res = http::get(url);
  if (res.empty())
    throw runtime_error("No results");

  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(res, root))
    throw runtime_error("Unable to parse extensions index: " + reader.getFormattedErrorMessages());

  for (auto &k : root.getMemberNames()) {
    auto v = root[k];
    indexes[k] = ExtensionInfo {
        v["id"].asString(),
        v["baseUrl"].asString(),
        v["name"].asString(),
        v["language"].asString(),
        v["version"].asString(),
    };
  }
}

vector<shared_ptr<CManga>> Manager::searchManga(const string &query)
{
  if (currentExtension == nullptr)
    throw runtime_error("Extension not set");

  if (query != currentQuery)
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

vector<string> Manager::getLocalExtensionPaths()
{
  if (!fs::exists(extensionsDir))
    return {};
  vector<string> result;
  for (const auto &dirEntry : fs::recursive_directory_iterator(extensionsDir))
    if (dirEntry.is_regular_file())
      result.push_back(dirEntry.path().filename().string());
  return result;
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

tuple<vector<shared_ptr<CManga>>, bool> Manager::searchManga(shared_ptr<CExtension> ext, int page, const string &query)
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
