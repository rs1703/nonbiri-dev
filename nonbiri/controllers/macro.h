#ifndef NONBIRI_CONTROLLERS_MACRO_H_
#define NONBIRI_CONTROLLERS_MACRO_H_

#define MIME_JSON "application/json"

#define STATUS_OK 200
#define STATUS_BAD_REQUEST 400
#define STATUS_NOT_FOUND 404
#define STATUS_INTERNAL_SERVER_ERROR 500

#define JSON_BAD_REQUEST "{\"error\": \"Bad request\"}"
#define JSON_EXCEPTION "{\"error\": \"" + std::string(e.what()) + "\"}"
#define JSON_EXTENSION_NOT_FOUND "{\"error\": \"Extension not found\"}"
#define JSON_MESSAGE(msg) "{\"message\": \"" + std::string(msg) + "\"}"

#define REPLY(code, body, mime) \
  res.status = code; \
  res.set_content(body, mime);

#define ABORT(code, body, mime) \
  res.status = code; \
  return res.set_content(body, mime);

#endif  // NONBIRI_CONTROLLERS_MACRO_H_