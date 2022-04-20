#include <thread>

#include <nonbiri/app.h>
#include <nonbiri/controllers/api.h>
#include <nonbiri/controllers/web.h>
#include <nonbiri/database.h>
#include <nonbiri/server.h>

App::App(int argc, char *argv[])
{
  parseOptions(argc, argv);

  Database::initialize();
  manager = new Manager();
  server = new Server(port);

  new Api(*server, *manager);
  new Web(*server);
}

App::~App()
{
  delete manager;
}

void App::start()
{
  std::thread([this]() { manager->updateExtensionIndexes(); }).detach();
  server->start();
}

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
