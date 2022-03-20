#ifndef NONBIRI_MODELS_EXTENSION_H_
#define NONBIRI_MODELS_EXTENSION_H_

#include <string>

#include <core/extension/extension.h>

class CExtension : public Extension
{
  void *handle = NULL;

public:
  std::string id;
  bool isInstalled;
  bool isEnabled;

public:
  CExtension(const Extension &extension);
  ~CExtension();

  void setHandle(void *handle)
  {
    this->handle = handle;
  }
};

#endif  // NONBIRI_MODELS_EXTENSION_H_