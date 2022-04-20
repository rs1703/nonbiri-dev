#include <fstream>

#include <nonbiri/controllers/macro.h>
#include <nonbiri/controllers/web.h>

using httplib::Request;
using httplib::Response;

Web::Web(Server &s)
{
  GET(R"(/?(history|updates|browse)?/?.*)", render);
  s.set_mount_point("/assets", "./assets");
}

void Web::render(const Request &req, Response &res)
{
  static const std::string path {"index.html"};
  httplib::detail::read_file(path, res.body);

  res.status = 200;
  res.set_header("Content-Type", "text/html");
}