#ifndef NONBIRI_MODELS_EXTENSION_H_
#define NONBIRI_MODELS_EXTENSION_H_

#include <string>

#include <core/extension/extension.h>

class CExtension : public Extension
{
  void *handle = NULL;

public:
  int id;
  bool isInstalled;
  bool isEnabled;

  std::string libName;

public:
  void *getHandle()
  {
    return handle;
  }

  void setHandle(void *handle)
  {
    this->handle = handle;
  }

public:
  CExtension(const Extension &extension);
  ~CExtension();
};

struct ExtensionInfo
{
  std::string baseUrl;
  std::string name;
  std::string language;
  std::string version;
};

#endif  // NONBIRI_MODELS_EXTENSION_H_