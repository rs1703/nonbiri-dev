#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <core/utils/utils.h>
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
using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::vector;

typedef Extension *(*createPtr)(void);

shared_ptr<CExtension> createExtension(void *handle)
{
  auto create = (createPtr)utils::getSymbol(handle, "create");
  if (create == nullptr)
    throw runtime_error("Unable to get create function from extension");

  auto extension = create();
  if (extension == nullptr)
    throw runtime_error("Unable to create extension");

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

Manager::Manager() {}
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

shared_ptr<CExtension> Manager::loadExtension(const string &path)
{
  auto handle = utils::loadLibrary(path.c_str());
  if (handle == nullptr)
    throw runtime_error("Unable to load extension");

  auto ext = createExtension(handle);
  if (extensions.find(ext->name) != extensions.end())
    throw runtime_error("Extension already loaded");

  extensions.insert(make_pair(ext->name, ext));
  return ext;
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
  if (currentExtension && ext && currentExtension->name == ext->name)
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

  CHtml html(res);
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

  CHtml html(res);
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

  CHtml html(res);
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

  CHtml html(res);
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

  CHtml html(res);
  return ext->parsePages(chapter, html);
}

const vector<string> Manager::getExtensions(const string &path)
{
  vector<string> result;
  for (const auto &dirEntry : std::filesystem::recursive_directory_iterator(path))
    if (dirEntry.is_regular_file())
      result.push_back(dirEntry.path().string());
  return result;
}