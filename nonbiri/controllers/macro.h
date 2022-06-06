#ifndef NONBIRI_CONTROLLERS_MACRO_H_
#define NONBIRI_CONTROLLERS_MACRO_H_

#define MIME_JSON "application/json"

#define JSON_BAD_REQUEST          "{\"error\": \"Bad request\"}"
#define JSON_EXCEPTION            "{\"error\": \"" + std::string(e.what()) + "\"}"
#define JSON_EXTENSION_NOT_FOUND  "{\"error\": \"Extension not found\"}"
#define JSON_MANGA_NOT_FOUND      "{\"error\": \"Manga not found\"}"
#define JSON_MISSING_PARAM(param) "{\"error\": \"Missing parameter: " + std::string(param) + "\"}"

#define REPLY(code, body, mime) \
  res.status = code; \
  res.set_content(body, mime);

#define ABORT(code, body, mime) \
  res.status = code; \
  return res.set_content(body, mime);

#define GET(path, callback)    App::server->Get(path, [&](const Request &req, Response &res) { callback(req, res); })
#define POST(path, callback)   App::server->Post(path, [&](const Request &req, Response &res) { callback(req, res); })
#define DELETE(path, callback) App::server->Delete(path, [&](const Request &req, Response &res) { callback(req, res); })

#endif  // NONBIRI_CONTROLLERS_MACRO_H_