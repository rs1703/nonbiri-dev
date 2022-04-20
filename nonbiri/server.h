#ifndef NONBIRI_SERVER_H_
#define NONBIRI_SERVER_H_

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#endif

#include <httplib.h>

class Server : private httplib::Server
{
  const int mPort;

public:
  Server(int port);

  using httplib::Server::Delete;
  using httplib::Server::Get;
  using httplib::Server::Patch;
  using httplib::Server::Post;
  using httplib::Server::Put;
  using httplib::Server::set_mount_point;

  void start();
};

#endif  // NONBIRI_SERVER_H_
