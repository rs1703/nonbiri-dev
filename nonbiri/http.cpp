#include <mutex>
#include <stdexcept>

#include <nonbiri/http.h>
#include <nonbiri/utility.h>

void *handle {nullptr};

void Http::initialize()
{
  static std::mutex mtx {};
  std::lock_guard lock(mtx);
  if (handle != nullptr)
    return;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  const std::string libcurl {"libcurl.dll"};
#else
  const std::string libcurl {"libcurl.so"};
#endif

  handle = Utils::loadLibrary(libcurl);
  if (handle == nullptr)
    throw std::runtime_error("Unable to load libcurl");

  init = (init_t)Utils::getSymbol(handle, "curl_easy_init");
  setOpt = (setOpt_t)Utils::getSymbol(handle, "curl_easy_setopt");
  perform = (perform_t)Utils::getSymbol(handle, "curl_easy_perform");
  cleanup = (cleanup_t)Utils::getSymbol(handle, "curl_easy_cleanup");
  getInfo = (getInfo_t)Utils::getSymbol(handle, "curl_easy_getinfo");

  if (init == nullptr || setOpt == nullptr || perform == nullptr || cleanup == nullptr)
    throw std::runtime_error("Unable to load libcurl symbols");
}

void Http::attach(Http::initialize_t initializeFnPtr)
{
  initializeFnPtr(init, setOpt, perform, cleanup, getInfo);
}