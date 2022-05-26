#include <thread>

#include <nonbiri/app.h>
#include <nonbiri/controllers/api.h>
#include <nonbiri/controllers/web.h>
#include <nonbiri/database.h>
#include <nonbiri/manager.h>
#include <nonbiri/models/manga.h>
#include <nonbiri/server.h>

bool App::daemonize = false;
int App::port = 42081;

void App::initialize(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--daemonize") == 0 || strcmp(argv[i], "-d") == 0) {
      daemonize = true;
    } else if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
      port = atoi(argv[i + 1]);
      i++;
    }
  }

  Database::initialize();
  Manga::initialize();

  manager = new Manager();
  server = new Server(port);

  new Api();
  new Web();
}

void App::start()
{
  std::thread([]() { manager->updateExtensionIndexes(); }).detach();
  server->start();
}
