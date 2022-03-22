#include <iostream>
#include <stdexcept>

#include <core/utils/utils.h>
#include <nonbiri/models/chapter.h>
#include <nonbiri/models/extension.h>
#include <nonbiri/models/manga.h>
#include <nonbiri/utils/utils.h>

using std::cout;
using std::endl;
using std::exception;
using std::make_shared;
using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::vector;

LRU<CManga> CExtension::mlru {2048};

CExtension::CExtension(const Extension &extension) : Extension(extension) {}

CExtension::~CExtension()
{
  // std::cout << "CExtension::~CExtension()" << std::endl;
  if (handle != nullptr)
    utils::freeLibrary(handle);
}

bool CExtension::operator==(const CExtension &extension)
{
  return id == extension.id;
}

bool CExtension::operator!=(const CExtension &extension)
{
  return id != extension.id;
}

void *CExtension::getHandle() const
{
  return handle;
}

void CExtension::setHandle(void *handle)
{
  this->handle = handle;
}

bool CExtension::isHasUpdate() const
{
  return hasUpdate;
}

void CExtension::setHasUpdate(bool hasUpdate)
{
  this->hasUpdate = hasUpdate;
}

tuple<vector<shared_ptr<CManga>>, bool> CExtension::getLatests(int page)
{
  auto res = latestsRequest(page);
  if (res.empty())
    throw runtime_error("No results");

  if (useApi) {
    auto result = parseLatestEntries(res);
    return normalizeMangaEntries(result);
  }

  CHtml html {res};
  try {
    auto result = parseLatestEntries(html);
    return normalizeMangaEntries(result);
  } catch (...) {
    // ignore
  }

  auto selector = latestsSelector();
  auto entries = html.select(selector);

  vector<shared_ptr<CManga>> result;
  for (auto &entry : entries) {
    auto manga = parseLatestEntry(*entry);
    if (manga != nullptr)
      result.push_back(make_shared<CManga>(*this, *manga));
  }

  auto nextSelector = latestsNextSelector();
  bool hasNext = false;

  if (!nextSelector.empty()) {
    auto next = html.selectFirst(nextSelector);
    if (next)
      hasNext = next->isValid();
  }
  return make_tuple(result, hasNext);
}

tuple<vector<shared_ptr<CManga>>, bool> CExtension::searchManga(int page, const string &query)
{
  auto res = searchMangaRequest(page, query);
  if (res.empty())
    throw runtime_error("No results");

  if (useApi) {
    auto result = parseSearchEntries(res);
    return normalizeMangaEntries(result);
  }

  CHtml html {res};
  try {
    auto result = parseSearchEntries(html);
    return normalizeMangaEntries(result);
  } catch (...) {
    // ignore
  }

  auto selector = searchMangaSelector();
  auto entries = html.select(selector);
  vector<shared_ptr<CManga>> result;

  for (auto &entry : entries) {
    auto manga = parseSearchEntry(*entry);
    if (manga != nullptr)
      result.push_back(make_shared<CManga>(*this, *manga));
  }

  auto nextSelector = searchMangaNextSelector();
  bool hasNext = false;

  if (!nextSelector.empty()) {
    auto next = html.selectFirst(nextSelector);
    if (next)
      hasNext = next->isValid();
  }
  return make_tuple(result, hasNext);
}

shared_ptr<CManga> CExtension::getManga(const string &path, bool update)
{
  string strippedPath {stripDomain(path)};
  string cacheKey {id + strippedPath};

  if (!update) {
    auto cache = mlru.get(cacheKey);
    if (cache != nullptr) {
      cout << "Cache hit for " << strippedPath << endl;
      return cache;
    }
  }

  string uri {baseUrl + strippedPath};
  auto res = http::get(uri);
  if (res.empty())
    throw runtime_error("No results");

  Manga *result;
  if (useApi) {
    result = parseManga(res);
  } else {
    CHtml html {res};
    result = parseManga(html);
  }

  if (result == nullptr)
    throw runtime_error("No results");

  auto manga = make_shared<CManga>(*this, *result);
  mlru.set(cacheKey, manga);

  return manga;
}

vector<shared_ptr<CChapter>> CExtension::getChapters(CManga &manga)
{
  auto res = chaptersRequest(manga);
  if (res.empty())
    throw runtime_error("No results");

  if (useApi) {
    auto result = parseChapterEntries(manga, res);
    return normalizeChapterEntries(manga, result);
  }

  CHtml html {res};
  try {
    auto result = parseChapterEntries(manga, html);
    return normalizeChapterEntries(manga, result);
  } catch (...) {
    // ignore
  }

  auto selector = chaptersSelector();
  auto elements = html.select(selector);
  vector<shared_ptr<CChapter>> result;

  for (auto &element : elements) {
    auto entry = parseChapterEntry(manga, *element);
    if (entry != nullptr) {
      auto chapter = make_shared<CChapter>(manga, *entry);
      manga.addChapter(chapter->url, *chapter);
      result.push_back(chapter);
    }
  }
  return result;
}

vector<shared_ptr<CChapter>> CExtension::getChapters(const string &path)
{
  auto manga = getManga(path);
  return getChapters(*manga);
}

vector<string> CExtension::getPages(const CChapter &chapter)
{
  auto res = pagesRequest(chapter);
  if (res.empty())
    throw runtime_error("No results");

  if (useApi)
    return parsePages(chapter, res);

  CHtml html {res};
  return parsePages(chapter, html);
}

tuple<vector<shared_ptr<CManga>>, bool> CExtension::normalizeMangaEntries(const tuple<vector<Manga *>, bool> &result)
{
  vector<shared_ptr<CManga>> entries;
  for (auto entry : std::get<0>(result))
    entries.push_back(make_shared<CManga>(*this, *entry));
  return make_tuple(entries, std::get<1>(result));
}

vector<shared_ptr<CChapter>> CExtension::normalizeChapterEntries(CManga &manga, const vector<Chapter *> &result)
{
  vector<shared_ptr<CChapter>> entries;
  for (auto entry : result) {
    auto chapter = make_shared<CChapter>(manga, *entry);
    manga.addChapter(chapter->url, *chapter);
    entries.push_back(chapter);
  }
  return entries;
}
