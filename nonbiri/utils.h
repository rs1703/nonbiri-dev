#ifndef NONBIRI_UTILS_H_
#define NONBIRI_UTILS_H_

#include <string>

namespace utils
{
void *loadLibrary(const std::string &path);
void *getSymbol(void *handle, const std::string &symbol);
void freeLibrary(void *handle);
}  // namespace utils

namespace http
{
int download(const std::string &url, const std::string &path);
}  // namespace http

#endif  // NONBIRI_UTILS_H_