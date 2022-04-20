#include <iostream>
#include <mutex>
#include <stdexcept>

#include <core/utils/utils.h>
#include <nonbiri/models/extension.h>
#include <nonbiri/utils.h>

using ElementPtr = std::shared_ptr<CElement>;

LRU<CManga> CExtension::mLRU {2048};

CExtension::CExtension(const Extension &extension) : Extension {extension} {}

CExtension::~CExtension()
{
  std::cout << "CExtension::~CExtension()" << std::endl;
}

bool CExtension::operator==(const CExtension &extension)
{
  return id == extension.id;
}

bool CExtension::operator!=(const CExtension &extension)
{
  return id != extension.id;
}

std::tuple<std::vector<MangaPtr>, bool> CExtension::getLatests(int page)
{
  const std::string res = latestsRequest(page);
  if (res.empty())
    throw std::runtime_error("No results");

  if (useApi) {
    const std::tuple<std::vector<Manga *>, bool> result = parseLatestEntries(res);
    return normalizeMangaEntries(result);
  }

  CHtml html {res};
  try {
    const std::tuple<std::vector<Manga *>, bool> result = parseLatestEntries(html);
    return normalizeMangaEntries(result);
  } catch (...) {
    // ignore
  }

  const std::string selector = latestsSelector();
  const std::vector<ElementPtr> entries = html.select(selector);

  std::vector<MangaPtr> result;
  for (const ElementPtr &entry : entries) {
    const Manga *manga = parseLatestEntry(*entry);
    if (manga != nullptr)
      result.push_back(std::make_shared<CManga>(*manga));
  }

  const std::string nextSelector = latestsNextSelector();
  bool hasNext = false;

  if (!nextSelector.empty()) {
    const ElementPtr next = html.selectFirst(nextSelector);
    if (next)
      hasNext = next->isValid();
  }
  return std::make_tuple(result, hasNext);
}

std::tuple<std::vector<MangaPtr>, bool> CExtension::searchManga(int page, const std::string &query)
{
  const std::string res = searchMangaRequest(page, query);
  if (res.empty())
    throw std::runtime_error("No results");

  if (useApi) {
    const std::tuple<std::vector<Manga *>, bool> result = parseSearchEntries(res);
    return normalizeMangaEntries(result);
  }

  CHtml html {res};
  try {
    const std::tuple<std::vector<Manga *>, bool> result = parseSearchEntries(html);
    return normalizeMangaEntries(result);
  } catch (...) {
    // ignore
  }

  const std::string selector = searchMangaSelector();
  const std::vector<ElementPtr> entries = html.select(selector);
  std::vector<MangaPtr> result;

  for (const ElementPtr &entry : entries) {
    const Manga *manga = parseSearchEntry(*entry);
    if (manga != nullptr)
      result.push_back(std::make_shared<CManga>(*manga));
  }

  const std::string nextSelector = searchMangaNextSelector();
  bool hasNext = false;

  if (!nextSelector.empty()) {
    const ElementPtr next = html.selectFirst(nextSelector);
    if (next)
      hasNext = next->isValid();
  }
  return std::make_tuple(result, hasNext);
}

MangaPtr CExtension::getManga(const std::string &path, bool update)
{
  std::string strippedPath {stripDomain(path)};
  std::string cacheKey {id + strippedPath};

  if (!update) {
    const MangaPtr cache = mLRU.get(cacheKey);
    if (cache != nullptr) {
      std::cout << "Cache hit for " << strippedPath << std::endl;
      return cache;
    }
  }

  const std::string uri {baseUrl + strippedPath};
  const std::string res = http::get(uri);
  if (res.empty())
    throw std::runtime_error("No results");

  Manga *result;
  if (useApi) {
    result = parseManga(res);
  } else {
    CHtml html {res};
    result = parseManga(html);
  }

  if (result == nullptr)
    throw std::runtime_error("No results");

  const MangaPtr manga = std::make_shared<CManga>(*result);
  mLRU.set(cacheKey, manga);

  return manga;
}

std::vector<ChapterPtr> CExtension::getChapters(CManga &manga)
{
  const std::string res = chaptersRequest(manga);
  if (res.empty())
    throw std::runtime_error("No results");

  if (useApi) {
    const std::vector<Chapter *> result = parseChapterEntries(manga, res);
    return normalizeChapterEntries(result);
  }

  CHtml html {res};
  try {
    const std::vector<Chapter *> result = parseChapterEntries(manga, html);
    return normalizeChapterEntries(result);
  } catch (...) {
    // ignore
  }

  const std::string selector = chaptersSelector();
  const std::vector<ElementPtr> elements = html.select(selector);
  std::vector<ChapterPtr> result;

  for (const ElementPtr &element : elements) {
    const Chapter *entry = parseChapterEntry(manga, *element);
    if (entry != nullptr)
      result.push_back(std::make_shared<CChapter>(*entry));
  }
  return result;
}

std::vector<ChapterPtr> CExtension::getChapters(const std::string &path)
{
  const MangaPtr manga = getManga(path);
  return getChapters(*manga);
}

std::vector<std::string> CExtension::getPages(const CChapter &chapter)
{
  const std::string res = pagesRequest(chapter);
  if (res.empty())
    throw std::runtime_error("No results");

  if (useApi)
    return parsePages(chapter, res);

  CHtml html {res};
  return parsePages(chapter, html);
}

std::tuple<std::vector<MangaPtr>, bool> CExtension::normalizeMangaEntries(
  const std::tuple<std::vector<Manga *>, bool> &result)
{
  std::vector<MangaPtr> entries;
  for (const Manga *entry : std::get<0>(result))
    entries.push_back(std::make_shared<CManga>(*entry));
  return std::make_tuple(entries, std::get<1>(result));
}

std::vector<ChapterPtr> CExtension::normalizeChapterEntries(const std::vector<Chapter *> &result)
{
  std::vector<ChapterPtr> entries;
  for (const Chapter *entry : result)
    entries.push_back(std::make_shared<CChapter>(*entry));
  return entries;
}
