#ifndef NONBIRI_APP_H_
#define NONBIRI_APP_H_

#include <nonbiri/manager.h>
#include <nonbiri/server.h>

class App
{
  Manager *manager = nullptr;
  Server *server = nullptr;

  bool daemonize = false;
  int port = 42081;

public:
  App(int argc, char *argv[]);
  ~App();

  void start();

private:
  void parseOptions(int argc, char *argv[]);
};

#endif  // NONBIRI_APP_H_