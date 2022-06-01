#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif
#include <iostream>

#include <curl/curl.h>
#include <nonbiri/utility.h>

utils::ExecTime::ExecTime(const std::string &name) : name(name), start(std::chrono::high_resolution_clock::now()) {}

utils::ExecTime::~ExecTime()
{
  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << name << " " << duration.count() << "ms" << std::endl;
}

void *utils::loadLibrary(const std::string &path)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  return LoadLibrary(path.c_str());
#else
  return dlopen(path.c_str(), RTLD_LAZY);
#endif
}

void *utils::getSymbol(void *handle, const std::string &symbol)
{
  if (handle == nullptr)
    return nullptr;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  return GetProcAddress((HMODULE)handle, symbol.c_str());
#else
  return dlsym(handle, symbol.c_str());
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

int http::download(const std::string &url, const std::string &path)
{
  CURL *curl = curl_easy_init();
  if (curl == nullptr)
    return -1;

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  FILE *fp = fopen(path.c_str(), "wb");
  if (fp == nullptr) {
    curl_easy_cleanup(curl);
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

  const CURLcode code = curl_easy_perform(curl);
  long httpCode = -1;

  if (code == CURLE_OK)
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  curl_easy_cleanup(curl);
  fclose(fp);

  return httpCode;
}
