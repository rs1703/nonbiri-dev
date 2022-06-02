#include <mutex>
#include <stdexcept>

#include <curl/curl.h>
#include <nonbiri/http.h>
#include <nonbiri/utility.h>

void Http::initialize()
{
  static std::mutex mtx {};
  static bool initialized {};

  std::lock_guard lock(mtx);
  if (initialized)
    return;
  initialized = true;

  init = &curl_easy_init;
  cleanup = &curl_easy_cleanup;
  setOpt = &curl_easy_setopt;
  perform = &curl_easy_perform;
  getInfo = &curl_easy_getinfo;
}

void Http::attach(Http::initialize_t initializeFnPtr)
{
  initializeFnPtr(init, setOpt, perform, cleanup, getInfo);
}