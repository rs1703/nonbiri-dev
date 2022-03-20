#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif
#include <curl/curl.h>
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

int http::download(const char *url, const char *path)
{
  CURL *curl = curl_easy_init();
  if (curl == nullptr)
    return -1;

  FILE *fp = fopen(path, "wb");
  if (fp == nullptr) {
    curl_easy_cleanup(curl);
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

  auto code = curl_easy_perform(curl);
  long httpCode = -1;

  if (code == CURLE_OK)
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  curl_easy_cleanup(curl);
  fclose(fp);

  return httpCode;
}