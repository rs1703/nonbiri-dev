#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

#include <nonbiri/utils/utils.h>

void *utils::loadLibrary(const char *path)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  return LoadLibrary(path);
#else
  return dlopen(path, RTLD_LAZY);
#endif
}

void *utils::getSymbol(void *handle, const char *symbol)
{
  if (handle == nullptr)
    return nullptr;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  return GetProcAddress((HMODULE)handle, symbol);
#else
  return dlsym(handle, symbol);
#endif
}

void utils::freeLibrary(void *handle)
{
  if (handle == nullptr)
    return;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  FreeLibrary((HMODULE)handle);
#else
  dlclose(handle);
#endif
}