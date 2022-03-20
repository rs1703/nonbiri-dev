#include <iostream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

#include <nonbiri/models/extension.h>

CExtension::CExtension(const Extension &extension) : Extension(extension) {}

CExtension::~CExtension()
{
  // std::cout << "CExtension::~CExtension()" << std::endl;
  if (handle != nullptr) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
  }
}
