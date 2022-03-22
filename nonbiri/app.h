#ifndef NONBIRI_APP_H_
#define NONBIRI_APP_H_

#include <nonbiri/manager/manager.h>

class App
{
  Manager *manager = nullptr;
  bool daemonize = false;
  int port = 42081;

public:
  App(int argc, char *argv[]);
  ~App();

  void Start();

private:
  void parseOptions(int argc, char *argv[]);
  void dummy();
};

#endif
