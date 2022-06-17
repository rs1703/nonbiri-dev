#include <filesystem>
#include <fstream>

#include <nonbiri/controllers/macro.h>
#include <nonbiri/controllers/web.h>
#include <nonbiri/manager.h>
#include <nonbiri/server.h>

using httplib::Request;
using httplib::Response;

namespace fs = std::filesystem;

Web::Web()
{
  HTTP_GET(R"(/icons/(\S+)/(\S+)?)", icon);
  HTTP_GET("/?(history|updates|browse)?/?.*", render);
  App::server->set_mount_point("/assets", "./assets");
}

void Web::render(const Request &, Response &res)
{
  httplib::detail::read_file("index.html", res.body);
  res.set_header("Content-Type", "text/html");
  res.status = 200;
}

void Web::icon(const Request &req, Response &res)
{
  const std::string domain = req.matches[1].str();
  const std::string version = req.matches[2].str();

  const std::string fileName = domain + "-" + version + ".png";
  const std::string path = (fs::path("icons") / fileName).string();

  if (!fs::exists(path) && App::manager->getExtensionInfo(domain) == nullptr) {
    res.status = 404;
    return;
  }

  if (!fs::exists(path)) {
    const int code = App::manager->downloadIcon(fileName, path);
    if (code != 200) {
      res.status = code;
      return;
    }
  }

  httplib::detail::read_file(path, res.body);
  res.set_header("Content-Type", "image/png");
  res.status = 200;
}