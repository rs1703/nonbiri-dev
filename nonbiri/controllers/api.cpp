#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>

#include <core/utils/utils.h>
#include <json/json.h>
#include <nonbiri/controllers/api.h>
#include <nonbiri/controllers/macro.h>
#include <nonbiri/manager.h>
#include <nonbiri/server.h>

using httplib::Request;
using httplib::Response;

Api::Api(Server &s, Manager &m) : manager(&m)
{
#define Get(path, callback) s.Get(path, [&](const Request &req, Response &res) { callback(req, res); })
#define Post(path, callback) s.Post(path, [&](const Request &req, Response &res) { callback(req, res); })

  Get(R"(/api/extensions/?(\w+)?/?)", getExtensions);
  Post("/api/extensions/?", refreshExtensions);
  Post("/api/extensions/install/?", installExtension);
  Post("/api/extensions/uninstall/?", uninstallExtension);
  Post("/api/extensions/update/?", updateExtension);

  Get("/api/manga/?", getLatests);
  Get("/api/search/?", searchManga);
  Get("/api/metadata/?", getManga);
  Get("/api/chapters/?", getChapters);
  Get("/api/pages/?", getPages);
}

void Api::getExtensions(const Request &req, Response &res)
{
  try {
    Json::Value root;
    Json::FastWriter writer;

    const bool isRefresh = !res.get_header_value("refresh").empty();
    if (isRefresh)
      res.headers.erase("refresh");

    ExtensionMap extensions = manager->getExtensions();
    if (req.matches[1].str() == "index" || isRefresh) {
      ExtensionInfoMap indexes = manager->getIndexes();
      for (const auto &[_, info] : indexes) {
        Json::Value json;
        json["id"] = info.id;
        json["name"] = info.name;
        json["baseUrl"] = info.baseUrl;
        json["language"] = info.language;
        json["version"] = info.version;
        json["isInstalled"] = extensions.find(info.id) != extensions.end();

        root.append(json);
      }
    } else {
      for (const auto &[_, ext] : extensions) {
        Json::Value json;
        json["id"] = ext->id;
        json["name"] = ext->name;
        json["baseUrl"] = ext->baseUrl;
        json["language"] = ext->language;
        json["version"] = ext->version;
        json["hasUpdate"] = (bool)ext->hasUpdate;

        root.append(json);
      }
    }

    const std::string json = root.empty() ? "[]" : writer.write(root);
    REPLY(STATUS_OK, json, MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::refreshExtensions(const Request &req, Response &res)
{
  try {
    manager->updateExtensionIndexes();
    res.set_header("refresh", "1");
    getExtensions(req, res);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

#define CHECK_EXTENSION_ID \
  const std::string id = req.get_param_value("id"); \
  if (id.empty()) { \
    ABORT(STATUS_BAD_REQUEST, JSON_BAD_REQUEST, MIME_JSON); \
  }

void Api::installExtension(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    manager->downloadExtension(id, false);
    getExtensions(req, res);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::uninstallExtension(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    manager->removeExtension(id);
    getExtensions(req, res);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::updateExtension(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    manager->updateExtension(id);
    getExtensions(req, res);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getLatests(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    ExtensionPtr ext = manager->getExtension(id);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string pageStr = req.get_param_value("page");
    const int page = std::max(1, std::stoi(pageStr.empty() ? "1" : pageStr));

    auto startTime = std::chrono::high_resolution_clock::now();
    const auto &[entries, hasNext] = ext->getLatests(page);

    auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Json::Value root;
    Json::FastWriter writer;

    root["id"] = ext->id;
    root["page"] = page;
    root["hasNext"] = hasNext;

    for (const MangaPtr &entry : entries) {
      Json::Value json;
      json["path"] = stripDomain(entry->url);
      json["coverUrl"] = entry->coverUrl;
      json["title"] = entry->title;

      root["entries"].append(json);
    }

    root["execDuration"] = duration;

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::searchManga(const Request &req, Response &res) {}

void Api::getManga(const Request &req, Response &res) {}

void Api::getChapters(const Request &req, Response &res) {}

void Api::getPages(const Request &req, Response &res) {}