#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <json/json.h>
#include <nonbiri/manager.h>

int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <extensions directory>" << std::endl;
    return 1;
  }

  auto dir = argv[1];
  auto manager = new Manager(dir);
  Json::Value extensions;

  for (auto const &[_, extension] : manager->extensions) {
    auto k = extension->libName;

    extensions[k]["baseUrl"] = extension->baseUrl;
    extensions[k]["name"] = extension->name;
    extensions[k]["language"] = extension->language;
    extensions[k]["version"] = extension->version;
  }

  std::ofstream f(std::string(dir) + ".json");
  Json::StyledWriter writer;

  f << writer.write(extensions);
  f.close();
}