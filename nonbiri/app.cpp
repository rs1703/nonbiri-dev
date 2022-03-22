#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <nonbiri/app.h>
#include <nonbiri/database/database.h>
#include <string.h>

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::string;

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
    std::thread([this]() { manager->checkExtensions(); }).detach();
    dummy();
  }
}

// Debugging purpose, we use this until we get a proper frontend.
void App::dummy()
{
  while (true) {
    cout << "==========================================================" << endl;
    cout << "Tasks:" << endl;
    cout << "1. Get installed extensions" << endl;
    cout << "2. List available extensions" << endl;
    cout << "3. Download extension" << endl;
    cout << "4. Update extension" << endl;
    cout << "5. Get latest manga from extension" << endl;
    cout << "6. Get manga from extension" << endl;
    cout << "0. Exit" << endl;
    cout << "==========================================================" << endl;

    int task;
    cout << "Task: ";
    cin >> task;
    cout << endl;

    switch (task) {
      case 0:
        return;
      case 1:
        for (auto &[id, ext] : manager->extensions) {
          cout << "- " << id << endl;
          cout << "ID: " << ext->id << endl;
          cout << "Name: " << ext->name << endl;
          cout << "Base URL: " << ext->baseUrl << endl;
          cout << "Language: " << ext->language << endl;
          cout << "Version: " << ext->version << endl;
          cout << "Need update: " << ext->isHasUpdate() << endl;
        }
        break;
      case 2:
        manager->updateExtensionIndexes();
        for (auto &[id, info] : manager->indexes) {
          cout << "- " << id << endl;
          cout << "ID: " << info.id << endl;
          cout << "Name: " << info.name << endl;
          cout << "Base URL: " << info.baseUrl << endl;
          cout << "Language: " << info.language << endl;
          cout << "Version: " << info.version << endl;

          auto isInstalled = manager->extensions.find(id) != manager->extensions.end();
          cout << "Installed: " << isInstalled << endl;
        }
        break;
      case 3:
      case 4:
      case 5:
      case 6:
        string id;
        cout << "ID: ";
        cin >> id;
        cout << endl;

        if (task == 3 || task == 4) {
          try {
            if (task == 3)
              manager->downloadExtension(id);
            else
              manager->updateExtension(id);
          } catch (const exception &e) {
            cout << "Error: " << e.what() << endl;
          }
        } else if (task == 5) {
          int page = 1;
          while (true) {
            cout << "Page: " << page << endl;
            auto [latests, hasNext] = manager->getLatests(id, page);
            for (auto &manga : latests) {
              cout << "- " << manga->title << endl;
              cout << "URL: " << manga->url << endl;
              cout << "CoverURL: " << manga->coverUrl << endl;
            }

            if (!hasNext)
              break;

            string next;
            cout << "Next page? (y/n): ";
            cin >> next;
            cout << endl;

            if (next != "y")
              break;

            page++;
          }
        } else if (task == 6) {
          string path;
          cout << "Path: ";
          cin >> path;
          cout << endl;

          auto manga = manager->getManga(id, path);
          cout << "- " << manga->title << endl;
          cout << "URL: " << manga->url << endl;
          cout << "CoverURL: " << manga->coverUrl << endl;
          cout << "Description: " << manga->description << endl;
          cout << "Status: " << manga->status << endl;

          cout << "Artists: " << endl;
          for (auto &artist : manga->artists)
            cout << "  - " << artist << endl;

          cout << "Authors: " << endl;
          for (auto &author : manga->authors)
            cout << "  - " << author << endl;

          cout << "Genres: " << endl;
          for (auto &genre : manga->genres)
            cout << "  - " << genre << endl;
        }

        break;
    }

    cout << endl;
  }
}