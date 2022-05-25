#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <core/utils.h>
#include <json/json.h>
#include <nonbiri/controllers/api.h>
#include <nonbiri/controllers/macro.h>
#include <nonbiri/library.h>
#include <nonbiri/manager.h>
#include <nonbiri/server.h>

using httplib::Request;
using httplib::Response;

Api::Api()
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
  POST("/api/library/add", addManga);
  DELETE("/api/library/remove", removeManga);
}

void Api::getExtensions(const Request &req, Response &res)
{
  try {
    Json::Value root {};
    Json::FastWriter writer {};

    const bool isRefresh = !res.get_header_value("refresh").empty();
    if (isRefresh)
      res.headers.erase("refresh");

    const ExtensionMap &extensions = App::manager->getExtensions();
    if (req.matches[1].str() == "index" || isRefresh) {
      const ExtensionInfoMap &indexes = App::manager->getIndexes();
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
    App::manager->updateExtensionIndexes();
    res.set_header("refresh", "1");
    getExtensions(req, res);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

#define CHECK_EXTENSION_ID \
  const std::string sourceId = req.get_param_value("sourceId"); \
  if (sourceId.empty()) { \
    ABORT(STATUS_BAD_REQUEST, JSON_BAD_REQUEST, MIME_JSON); \
  }

void Api::getExtensionFilters(const httplib::Request &req, httplib::Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
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
    App::manager->downloadExtension(sourceId, false);
    getExtensions(req, res);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::uninstallExtension(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    App::manager->removeExtension(sourceId);
    getExtensions(req, res);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::updateExtension(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    App::manager->updateExtension(sourceId);
    getExtensions(req, res);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getLatests(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string sPage = req.get_param_value("page");
    const int page = std::max(1, sPage.empty() ? 1 : std::stoi(sPage));
    const auto &[entries, hasNext] = App::manager->getLatests(*ext, page);

    Json::Value root {};
    Json::FastWriter writer {};

    root["page"] = page;
    root["hasNext"] = hasNext;
    for (const auto &manga : entries)
      root["entries"].append(manga->toJson());

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::searchManga(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    int page {1};
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

    const auto &[entries, hasNext] = App::manager->searchManga(*ext, page, query, filters);

    Json::Value root {};
    Json::FastWriter writer {};

    root["page"] = page;
    root["hasNext"] = hasNext;
    for (const auto &manga : entries)
      root["entries"].append(manga->toJson());

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getManga(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string path = req.get_param_value("path");
    const auto manga = App::manager->getManga(*ext, path);

    Json::Value root {};
    Json::FastWriter writer {};
    if (manga != nullptr)
      root = manga->toJson();

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getChapters(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string path = req.get_param_value("path");
    const auto chapters = App::manager->getChapters(*ext, path);

    Json::Value root {};
    Json::FastWriter writer {};
    for (const auto &chapter : chapters)
      root["entries"].append(chapter->toJson());

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getPages(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string path = req.get_param_value("path");
    const std::vector<std::string> pages = App::manager->getPages(*ext, path);

    Json::Value root {};
    Json::FastWriter writer {};
    for (const std::string &page : pages)
      root["pages"].append(page);

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::addManga(const Request &req, Response &res)
{
  try {
    CHECK_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const std::string path = req.get_param_value("path");
    const auto manga = App::manager->getManga(*ext, path);
    if (manga == nullptr) {
      ABORT(STATUS_NOT_FOUND, JSON_MANGA_NOT_FOUND, MIME_JSON);
    }

    Library::addManga(*manga);

    Json::Value root {};
    Json::FastWriter writer {};
    if (manga != nullptr)
      root = manga->toJson();

    REPLY(STATUS_OK, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::removeManga(const Request &req, Response &res)
{
  try {
    const std::string id = req.get_param_value("id");
    if (id.empty()) {
      ABORT(STATUS_BAD_REQUEST, JSON_BAD_REQUEST, MIME_JSON);
    }
    Library::removeManga(std::stoll(id));
    REPLY(STATUS_OK, "", MIME_JSON);
  } catch (const std::exception &e) {
    REPLY(STATUS_INTERNAL_SERVER_ERROR, JSON_EXCEPTION, MIME_JSON);
  }
}