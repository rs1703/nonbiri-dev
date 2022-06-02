#include <fstream>
#include <string>

#include <json/json.h>
#include <nonbiri/manager.h>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  const std::string dir {"windows"};
#else
  const std::string dir {"linux"};
#endif

  Manager manager(dir);
  Json::Value root {};

  auto extensions = manager.getExtensions();
  for (const auto &[_, extension] : extensions) {
    auto k = extension->id;
    root[k]["id"] = k;
    root[k]["baseUrl"] = extension->baseUrl;
    root[k]["name"] = extension->name;
    root[k]["language"] = extension->language;
    root[k]["version"] = extension->version;
    root[k]["isNsfw"] = extension->isNsfw;
  }

  std::ofstream out {dir + ".json"};
  Json::FastWriter writer;

  out << writer.write(root);
  out.close();
}