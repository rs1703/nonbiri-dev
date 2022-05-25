#include <fstream>

#include <nonbiri/controllers/macro.h>
#include <nonbiri/controllers/web.h>
#include <nonbiri/server.h>

using httplib::Request;
using httplib::Response;

Web::Web()
{
  GET(R"(/?(history|updates|browse)?/?.*)", render);
  App::server->set_mount_point("/assets", "./assets");
}

void Web::render(const Request &req, Response &res)
{
  static const std::string path {"index.html"};
  httplib::detail::read_file(path, res.body);

  res.status = 200;
  res.set_header("Content-Type", "text/html");
}