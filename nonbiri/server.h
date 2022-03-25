#ifndef NONBIRI_SERVER_H_
#define NONBIRI_SERVER_H_

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#endif

#include <httplib.h>
#include <nonbiri/manager.h>

class Controller;

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

  void start();
};

#endif  // NONBIRI_SERVER_H_
