#include <mutex>
#include <thread>

#include <core/core.h>
#include <nonbiri/app.h>
#include <nonbiri/controllers/api.h>
#include <nonbiri/controllers/web.h>
#include <nonbiri/database.h>
#include <nonbiri/manager.h>
#include <nonbiri/server.h>

bool App::daemonize {};
int App::port {42081};

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

  Http::init = &curl_easy_init;
  Http::cleanup = &curl_easy_cleanup;
  Http::setOpt = &curl_easy_setopt;
  Http::perform = &curl_easy_perform;
  Http::getInfo = &curl_easy_getinfo;
  Http::slist_append = &curl_slist_append;
  Http::slist_freeAll = &curl_slist_free_all;
  Http::getError = &curl_easy_strerror;

  Database::initialize();
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
