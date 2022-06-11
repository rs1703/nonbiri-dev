#ifndef NONBIRI_CONTROLLERS_MACRO_H_
#define NONBIRI_CONTROLLERS_MACRO_H_

#define MIME_JSON "application/json"

#define JSON_ERROR(error)         "{\"error\": \"" + std::string(error) + "\"}"
#define JSON_BAD_REQUEST          JSON_ERROR("Bad Request")
#define JSON_EXCEPTION            JSON_ERROR(e.what())
#define JSON_EXTENSION_NOT_FOUND  JSON_ERROR("Extension not found")
#define JSON_MANGA_NOT_FOUND      JSON_ERROR("Manga not found")
#define JSON_MISSING_PARAM(param) JSON_ERROR("Missing parameter: " + std::string(param))

#define REPLY(code, body, mime) \
  res.status = code; \
  res.set_content(body, mime);

#define ABORT(code, body, mime) \
  res.status = code; \
  return res.set_content(body, mime);

#define HTTP_GET(path, callback)    App::server->Get(path, [&](const Request &req, Response &res) { callback(req, res); })
#define HTTP_POST(path, callback)   App::server->Post(path, [&](const Request &req, Response &res) { callback(req, res); })
#define HTTP_DELETE(path, callback) App::server->Delete(path, [&](const Request &req, Response &res) { callback(req, res); })

#endif  // NONBIRI_CONTROLLERS_MACRO_H_