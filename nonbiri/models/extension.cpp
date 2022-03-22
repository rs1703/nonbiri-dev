#include <iostream>
#include <nonbiri/models/extension.h>
#include <nonbiri/utils/utils.h>

CExtension::CExtension(const Extension &extension) : Extension(extension) {}

CExtension::~CExtension()
{
  // std::cout << "CExtension::~CExtension()" << std::endl;
  if (handle != nullptr)
    utils::freeLibrary(handle);
}

bool CExtension::operator==(const CExtension &extension)
{
  return id == extension.id;
}

bool CExtension::operator!=(const CExtension &extension)
{
  return id != extension.id;
}

void *CExtension::getHandle() const
{
  return handle;
}

void CExtension::setHandle(void *handle)
{
  this->handle = handle;
}