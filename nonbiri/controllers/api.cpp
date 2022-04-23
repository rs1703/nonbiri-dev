#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <core/utils.h>
#include <json/json.h>
#include <nonbiri/controllers/api.h>
#include <nonbiri/controllers/macro.h>
#include <nonbiri/manager.h>
#include <nonbiri/server.h>

using httplib::Request;
using httplib::Response;

Api::Api(Server &s, Manager &m) : manager(&m)
{
  GET("/api/extensions/filters/?", getExtensionFilters);
  GET(R"(/api/extensions/?(\w+)?/?)", getExtensions);
  POST("/api/extensions/?", refreshExtensions);
  POST("/api/extensions/install/?", installExtension);
  POST("/api/extensions/uninstall/?", uninstallExtension);
  POST("/api/extensions/update/?", updateExtension);

  GET("/api/manga/?", getLatests);
  GET("/api/search/?", searchManga);
  GET("/api/metadata/?", getManga);
  GET("/api/chapters/?", getChapters);
  GET("/api/pages/?", getPages);
}

void Api::getExtensions(const Request &req, Response &res)
{
  try {
    Json::Value root {};
    Json::FastWriter writer {};

    const bool isRefresh = !res.get_header_value("refresh").empty();
    if (isRefresh)
      res.headers.erase("refresh");

    const ExtensionMap &extensions = manager->getExtensions();
    if (req.matches[1].str() == "index" || isRefresh) {
      const ExtensionInfoMap &indexes = manager->getIndexes();
      for (const auto &[_, info] : indexes) {
        Json::Value json {};
        json["id"] = info.id;
        json["name"] = info.name;
        json["baseUrl"] = info.baseUrl;
        json["language"] = info.language;
        json["version"] = info.version;
        json["isNsfw"] = info.isNsfw;
        json["isInstalled"] = extensions.find(info.id) != extensions.end();

        root.append(json);
      }
    } else {
      for (const auto &[_, ext] : extensions) {
        Json::Value json {};
        json["id"] = ext->id;
        json["name"] = ext->name;
        json["baseUrl"] = ext->baseUrl;
        json["language"] = ext->language;
        json["version"] = ext->version;
        json["isNsfw"] = ext->isNsfw;
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

void Api::getExtensionFilters(const httplib::Request &req, httplib::Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = manager->getExtension(id);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const auto &filters = ext->getFilters();
    Json::Value root {};
    Json::FastWriter writer {};

    for (const auto &filter : filters)
      root.append(filter.toJson());

    const std::string json = root.empty() ? "[]" : writer.write(root);
    REPLY(STATUS_OK, json, MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
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
    Extension *ext = manager->getExtension(id);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string pageStr = req.get_param_value("page");
    const int page = std::max(1, pageStr.empty() ? 1 : std::stoi(pageStr));

    auto startTime = std::chrono::high_resolution_clock::now();
    const auto &[entries, hasNext] = ext->getLatests(page);

    auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Json::Value root {};
    Json::FastWriter writer {};

    root["id"] = ext->id;
    root["page"] = page;
    root["hasNext"] = hasNext;

    for (const MangaPtr entry : entries) {
      Json::Value json;
      json["path"] = entry->path;
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

void Api::searchManga(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = manager->getExtension(id);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    int page = 1;
    std::string query {};
    std::vector<FilterKV> filters {};

    const auto &filtersMap = ext->getFiltersMap();
    for (const auto &[key, value] : req.params) {
      if (key == "page") {
        page = std::max(1, std::stoi(value));
      } else if (key == "q") {
        query = value;
      } else if (filtersMap.find(key) != filtersMap.end()) {
        filters.push_back({key, value});
      }
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    const auto &[entries, hasNext] = ext->searchManga(page, query, filters);

    auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Json::Value root {};
    Json::FastWriter writer {};

    root["id"] = ext->id;
    root["page"] = page;
    root["hasNext"] = hasNext;

    for (const MangaPtr &entry : entries) {
      Json::Value json {};
      json["path"] = entry->path;
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

void Api::getManga(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = manager->getExtension(id);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string path = req.get_param_value("path");
    const std::string updateStr = req.get_param_value("update");

    auto startTime = std::chrono::high_resolution_clock::now();
    const MangaPtr manga = ext->getManga(path);

    auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Json::Value root;
    Json::FastWriter writer;

    root["id"] = ext->id;
    if (manga != nullptr) {
      root["path"] = manga->path;

      if (!manga->coverUrl.empty())
        root["coverUrl"] = manga->coverUrl;

      root["title"] = manga->title;

      if (!manga->description.empty())
        root["description"] = manga->description;

      root["status"] = manga->status;

      for (const auto &artist : manga->artists)
        root["artists"].append(artist);

      for (const auto &author : manga->authors)
        root["authors"].append(author);

      for (const auto &genre : manga->genres)
        root["genres"].append(genre);
    }

    root["execDuration"] = duration;

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getChapters(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = manager->getExtension(id);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string path = req.get_param_value("path");

    auto startTime = std::chrono::high_resolution_clock::now();
    const std::vector<ChapterPtr> &chapters = ext->getChapters(path);

    auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Json::Value root {};
    Json::FastWriter writer {};

    root["id"] = ext->id;
    for (const ChapterPtr &chapter : chapters) {
      Json::Value json {};
      json["path"] = chapter->path;
      json["name"] = chapter->name;
      json["publishedAt"] = chapter->publishedAt;

      for (const auto &group : chapter->groups)
        json["groups"].append(group);

      root["entries"].append(json);
    }

    root["execDuration"] = duration;

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getPages(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = manager->getExtension(id);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string path = req.get_param_value("path");

    auto startTime = std::chrono::high_resolution_clock::now();
    const std::vector<std::string> &pages = ext->getPages(path);

    auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    Json::Value root {};
    Json::FastWriter writer {};

    root["id"] = ext->id;
    for (const std::string &page : pages)
      root["pages"].append(page);

    root["execDuration"] = duration;

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}