#include <fstream>
#include <string>

#include <json/json.h>
#include <nonbiri/manager/manager.h>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  std::string dir {"windows"};
#else
  std::string dir {"linux"};
#endif
  auto manager = new Manager(dir);

  Json::Value extensions;
  for (auto const &[_, extension] : manager->extensions) {
    auto k = extension->id;
    extensions[k]["id"] = k;
    extensions[k]["baseUrl"] = extension->baseUrl;
    extensions[k]["name"] = extension->name;
    extensions[k]["language"] = extension->language;
    extensions[k]["version"] = extension->version;
  }

  std::ofstream f {dir + ".json"};
  Json::StyledWriter writer;

  f << writer.write(extensions);
  f.close();
}