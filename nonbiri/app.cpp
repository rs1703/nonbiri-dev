#include <iostream>
#include <thread>

#include <nonbiri/app.h>
#include <nonbiri/database/database.h>
#include <string.h>

using std::cout;
using std::endl;

void App::parseOptions(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--daemonize") == 0 || strcmp(argv[i], "-d") == 0) {
      daemonize = true;
    } else if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
      port = atoi(argv[i + 1]);
      i++;
    }
  }
}

App::App(int argc, char *argv[])
{
  parseOptions(argc, argv);

  Database::initialize();
  manager = new Manager();
}

App::~App()
{
  delete manager;
}

void App::Start()
{
  while (true) {
    // do whatever
  }
}