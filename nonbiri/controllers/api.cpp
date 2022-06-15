#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <core/pref.h>
#include <json/json.h>
#include <nonbiri/controllers/api.h>
#include <nonbiri/controllers/macro.h>
#include <nonbiri/manager.h>
#include <nonbiri/server.h>
#include <nonbiri/utility.h>

#define REQUIRE_PARAM(varName, name) \
  const std::string varName = req.get_param_value(name); \
  if (varName.empty()) { \
    ABORT(400, JSON_MISSING_PARAM(name), MIME_JSON); \
  }

#define REQUIRE_EXTENSION_ID REQUIRE_PARAM(sourceId, "sourceId")

using httplib::Request;
using httplib::Response;

Api::Api()
{
  HTTP_GET("/api/extensions/filters/?", getExtensionFilters);
  HTTP_GET("/api/extensions/prefs/?", getExtensionPrefs);
  HTTP_POST("/api/extensions/prefs/?", setExtensionPrefs);

  HTTP_GET(R"(/api/extensions/?(\w+)?/?)", getExtensions);
  HTTP_POST("/api/extensions/?", refreshExtensions);
  HTTP_POST("/api/extensions/install/?", installExtension);
  HTTP_POST("/api/extensions/uninstall/?", uninstallExtension);
  HTTP_POST("/api/extensions/update/?", updateExtension);

  HTTP_GET("/api/manga/?", getLatests);
  HTTP_GET("/api/search/?", searchManga);
  HTTP_GET("/api/metadata/?", getManga);
  HTTP_GET("/api/chapters/?", getChapters);
  HTTP_GET("/api/pages/?", getPages);
  HTTP_POST("/api/library/manga/readState", setMangaReadState);
}

void Api::getExtensions(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::getExtensions");
  try {
    const bool isRefresh = !res.get_header_value("refresh").empty();
    if (isRefresh)
      res.headers.erase("refresh");

    Json::Value root {};
    const auto &extensions = App::manager->getExtensions();

    if (req.matches[1].str() == "index" || isRefresh) {
      const auto &indexes = App::manager->getIndexes();
      for (const auto &info : indexes) {
        Json::Value json = info.second.toJson();
        json["isInstalled"] = extensions.find(info.second.id) != extensions.end();
        root.append(json);
      }
    } else {
      for (const auto &ext : extensions) {
        Json::Value json = ext.second->toJson();
        json["hasUpdate"] = ext.second->hasUpdate.load();
        json["hasPrefs"] = dynamic_cast<Pref::Prefs *>(ext.second) != nullptr;
        root.append(json);
      }
    }

    Json::FastWriter writer {};
    REPLY(200, root.empty() ? "[]" : writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::refreshExtensions(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::refreshExtensions");
  try {
    App::manager->updateExtensionIndexes();
    res.set_header("refresh", "1");
    getExtensions(req, res);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::installExtension(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::installExtension");
  try {
    REQUIRE_EXTENSION_ID;
    App::manager->downloadExtension(sourceId, false);
    getExtensions(req, res);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::uninstallExtension(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::uninstallExtension");
  try {
    REQUIRE_EXTENSION_ID;
    App::manager->removeExtension(sourceId);
    getExtensions(req, res);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::updateExtension(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::updateExtension");
  try {
    REQUIRE_EXTENSION_ID;
    App::manager->updateExtension(sourceId);
    getExtensions(req, res);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getExtensionFilters(const httplib::Request &req, httplib::Response &res)
{
  Utils::ExecTime execTime("Api::getExtensionFilters");
  try {
    REQUIRE_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(404, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const auto &filters = ext->getFilters();

    Json::Value root {};
    for (const auto &filter : filters)
      if (!filter->hidden)
        root.append(filter->toJson());

    Json::FastWriter writer {};
    REPLY(200, root.empty() ? "[]" : writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getExtensionPrefs(const httplib::Request &req, httplib::Response &res)
{
  Utils::ExecTime execTime("Api::getExtensionPrefs");
  try {
    REQUIRE_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(404, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    const auto *prefs = dynamic_cast<Pref::Prefs *>(ext);
    if (prefs == nullptr) {
      ABORT(404, JSON_ERROR("Extension does not have prefs"), MIME_JSON);
    }

    Json::FastWriter writer {};
    REPLY(200, writer.write(prefs->toJson(true)), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::setExtensionPrefs(const httplib::Request &req, httplib::Response &res)
{
  Utils::ExecTime execTime("Api::setExtensionPrefs");
  try {
    REQUIRE_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(404, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    auto *prefs = dynamic_cast<Pref::Prefs *>(ext);
    if (prefs == nullptr) {
      ABORT(404, JSON_ERROR("Extension does not have prefs"), MIME_JSON);
    }

    Json::Reader reader {};
    Json::Value payload {};
    if (!reader.parse(req.body, payload)) {
      ABORT(400, JSON_ERROR("Unable to parse payload"), MIME_JSON);
    }

    std::vector<std::string> invalidKeys {};
    for (const auto &obj : payload) {
      if (!obj.isObject() || !obj.isMember("key") || !obj.isMember("value")) {
        ABORT(400, JSON_ERROR("Payload must be an array of objects, each with a key and value"), MIME_JSON);
      }

      const std::string key = obj["key"].asString();
      if (prefs->set(key, obj["value"]) < 0)
        invalidKeys.push_back(key);
    }

    Json::Value root {};
    root["new"] = prefs->toJson();

    if (!invalidKeys.empty()) {
      root["error"] = "Payload contains invalid keys";
      for (const auto &key : invalidKeys)
        root["keys"].append(key);
    }

    Json::FastWriter writer {};
    REPLY(200, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getLatests(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::getLatests");
  try {
    REQUIRE_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(404, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    REQUIRE_PARAM(sPage, "page");
    const int page = std::max(1, sPage.empty() ? 1 : std::stoi(sPage));
    const auto &[entries, hasNext] = App::manager->getLatests(*ext, page);

    Json::Value root {};
    root["page"] = page;
    root["hasNext"] = hasNext;
    for (const auto &manga : entries)
      root["entries"].append(manga->toJson());

    Json::FastWriter writer {};
    REPLY(200, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::searchManga(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::searchManga");
  try {
    REQUIRE_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(404, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    int page {1};
    std::string query {};
    std::vector<std::pair<std::string, std::string>> pairs {};

    const auto &filtersIndex = ext->getFiltersIndex();
    for (const auto &[key, value] : req.params) {
      if (key == "page") {
        page = std::max(1, std::stoi(value));
      } else if (key == "q") {
        query = value;
      } else if (filtersIndex.find(key) != filtersIndex.end()) {
        pairs.push_back({key, value});
      }
    }

    const auto &[entries, hasNext] = App::manager->searchManga(*ext, page, query, pairs);

    Json::Value root {};
    root["page"] = page;
    root["hasNext"] = hasNext;
    for (const auto &manga : entries)
      root["entries"].append(manga->toJson());

    Json::FastWriter writer {};
    REPLY(200, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getManga(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::getManga");
  try {
    REQUIRE_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(404, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    REQUIRE_PARAM(path, "path");
    const auto manga = App::manager->getManga(*ext, path);

    Json::Value root {};
    if (manga != nullptr)
      root = manga->toJson();

    Json::FastWriter writer {};
    REPLY(200, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getChapters(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::getChapters");
  try {
    REQUIRE_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(404, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    REQUIRE_PARAM(path, "path");
    const auto chapters = App::manager->getChapters(*ext, path);

    Json::Value root {};
    Json::FastWriter writer {};
    for (const auto &chapter : chapters)
      root["entries"].append(chapter->toJson());

    REPLY(200, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::getPages(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::getPages");
  try {
    REQUIRE_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(404, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    REQUIRE_PARAM(path, "path");
    const auto pages = App::manager->getPages(*ext, path);

    Json::Value root {};
    Json::FastWriter writer {};
    for (const auto &page : pages)
      root["pages"].append(page);

    REPLY(200, writer.write(root), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}

void Api::setMangaReadState(const Request &req, Response &res)
{
  Utils::ExecTime execTime("Api::setMangaReadState");
  try {
    REQUIRE_PARAM(sState, "state");
    ReadingStatus state = static_cast<ReadingStatus>(std::stoi(sState));

    REQUIRE_EXTENSION_ID;
    Extension *ext = App::manager->getExtension(sourceId);
    if (ext == nullptr) {
      ABORT(404, JSON_EXTENSION_NOT_FOUND, MIME_JSON);
    }

    REQUIRE_PARAM(path, "path");
    auto manga = App::manager->getManga(*ext, path);
    if (manga == nullptr) {
      ABORT(404, JSON_MANGA_NOT_FOUND, MIME_JSON);
    }

    if (manga->readingStatus == state) {
      res.status = 304;
      return;
    }

    if (manga->id <= 0)
      manga->save();
    manga->setReadState(state);

    Json::FastWriter writer {};
    REPLY(200, writer.write(manga->toJson()), MIME_JSON);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    REPLY(500, JSON_EXCEPTION, MIME_JSON);
  }
}