#include <cstring>
#include <iostream>
#include <string>
#include <tuple>

#include <core/utils/utils.h>
#include <drogon/drogon.h>
#include <nonbiri/database/database.h>
#include <nonbiri/main.h>
#include <nonbiri/manager.h>

using std::cout;
using std::endl;

Manager *gManager;

struct Options
{
  bool daemonize;
  int port;
};

Options parseOptions(int argc, char *argv[])
{
  Options options {false, 42081};
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--daemonize") == 0 || strcmp(argv[i], "-d") == 0) {
      options.daemonize = true;
    } else if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
      options.port = atoi(argv[i + 1]);
      i++;
    }
  }
  return options;
}

int main(int argc, char *argv[])
{
  cout << "Initializing database..." << endl;
  auto err = db::init();
  if (err != nullptr) {
    cout << "Failed to initialize database: " << err << endl;
    return -1;
  }

  auto options = parseOptions(argc, argv);
  gManager = new Manager();

  drogon::app()
      .setLogPath("./")
      .setLogLevel(trantor::Logger::kWarn)
      .addListener("0.0.0.0", options.port)
      .setThreadNum(0);

  if (options.daemonize)
    drogon::app().enableRunAsDaemon();

  cout << "Running on port " << options.port << endl;
  drogon::app().run();
}
