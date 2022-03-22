#include <filesystem>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <core/utils/utils.h>
#include <json/json.h>
#include <nonbiri/manager/manager.h>
#include <nonbiri/utils/utils.h>

using std::cout;
using std::endl;
using std::exception;
using std::lock_guard;
using std::make_pair;
using std::make_shared;
using std::make_tuple;
using std::map;
using std::mutex;
using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::time;
using std::tuple;
using std::vector;

namespace fs = std::filesystem;
typedef CExtension *(*createPtr)(void);

static const string dataBaseUrl {"https://raw.githubusercontent.com/rs1703/nonbiri-extensions-dev/releases"};

shared_ptr<CExtension> createExtension(void *handle)
{
  auto create = (createPtr)utils::getSymbol(handle, "create");
  if (create == nullptr)
    throw runtime_error("Unable to get fn symbol 'create' from extension");

  auto extension = create();
  if (extension == nullptr)
    throw runtime_error("Unable to instantiate extension");

  auto ext = shared_ptr<CExtension>(extension);
  ext->setHandle(handle);
  return ext;
}

Manager::Manager(const string &dir) : extensionsDir(dir)
{
  cout << "Initializing manager..." << endl;
  auto paths = getLocalExtensionPaths();
  for (auto path : paths) {
    try {
      loadExtension(path);
    } catch (const exception &e) {
      cout << e.what() << endl;
    }
  }
}

Manager::~Manager()
{
  cout << "Manager::~Manager()" << endl;
}

shared_ptr<CExtension> Manager::getExtension(const string &id)
{
  auto it = extensions.find(id);
  if (it == extensions.end()) {
    if (indexes.find(id) == indexes.end())
      throw runtime_error("Extension not found");
    else
      throw runtime_error("Extension not loaded");
  }
  return it->second;
}

void Manager::loadExtension(const string &name)
{
  cout << "Loading " << name << "..." << endl;
  auto path = fs::path(extensionsDir) / name;
  if (!fs::exists(path))
    throw runtime_error("Extension not found");

  auto handle = utils::loadLibrary(path.string());
  if (handle == nullptr)
    throw runtime_error("Unable to load extension");

  auto ext = createExtension(handle);
  auto id = ext->id;

  if (extensions.find(id) != extensions.end())
    throw runtime_error("Extension already loaded");

  extensions.insert(make_pair(id, ext));
  cout << "Loaded " << id << endl;
}

void Manager::unloadExtension(const string &id)
{
  cout << "Unloading " << id << "..." << endl;
  auto it = extensions.find(id);
  if (it == extensions.end())
    throw runtime_error("Extension not loaded");

  utils::freeLibrary(it->second->getHandle());
  extensions.erase(it);

  cout << "Unloaded " << id << endl;
}

void Manager::downloadExtension(const string &id, bool update)
{
  static mutex mtx;
  lock_guard<mutex> lck(mtx);

  if (indexes.find(id) == indexes.end())
    throw runtime_error("Extension not found");

  if (!update && extensions.find(id) != extensions.end())
    throw runtime_error("Extension already loaded");

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  string name {id + ".dll"};
  string url {dataBaseUrl + "/windows/" + name};
#else
  string name {"lib" + id + ".so"};
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
      unloadExtension(id);
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

void Manager::updateExtension(const string &id)
{
  downloadExtension(id, true);
}

void Manager::updateExtensionIndexes()
{
  static mutex mtx;
  lock_guard<mutex> lck(mtx);

  std::time_t now {std::time(nullptr)};
  double minutes {std::difftime(now, indexLastUpdate) / 60.0};

  if (minutes > 0 && minutes <= 10.0)
    return;
  indexLastUpdate = now;

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

void Manager::checkExtensions()
{
  static bool isChecking {false};
  if (isChecking)
    return;

  isChecking = true;
  try {
    updateExtensionIndexes();
  } catch (...) {
  }

  for (auto &[id, info] : indexes) {
    auto it = extensions.find(id);
    if (it == extensions.end())
      continue;

    auto ext = it->second;
    if (ext->version != ext->version)
      ext->setHasUpdate(true);
  }
  isChecking = false;
}

tuple<vector<shared_ptr<CManga>>, bool> Manager::getLatests(const string &id, int page)
{
  return getExtension(id)->getLatests(page);
}

tuple<vector<shared_ptr<CManga>>, bool> Manager::searchManga(const string &id, int page, const string &query)
{
  return getExtension(id)->searchManga(page, query);
}

shared_ptr<CManga> Manager::getManga(const string &id, const string &path)
{
  return getExtension(id)->getManga(path);
}

vector<shared_ptr<CChapter>> Manager::getChapters(const string &id, CManga &manga)
{
  return getExtension(id)->getChapters(manga);
}

vector<string> Manager::getPages(const string &id, const CChapter &chapter)
{
  return getExtension(id)->getPages(chapter);
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
