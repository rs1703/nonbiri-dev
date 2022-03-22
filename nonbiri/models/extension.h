#ifndef NONBIRI_MODELS_EXTENSION_H_
#define NONBIRI_MODELS_EXTENSION_H_

#include <string>

#include <core/extension/extension.h>

class CExtension : public Extension
{
  void *handle = NULL;

public:
  CExtension(const Extension &extension);
  ~CExtension();

  bool operator==(const CExtension &extension);
  bool operator!=(const CExtension &extension);

public:
  void *getHandle() const;
  void setHandle(void *handle);
};

#endif  // NONBIRI_MODELS_EXTENSION_H_