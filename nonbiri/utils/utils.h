#ifndef NONBIRI_UTILS_UTILS_H
#define NONBIRI_UTILS_UTILS_H

#include <string>

namespace utils
{
void *loadLibrary(const char *path);
void *loadLibrary(const std::string &path);
void *getSymbol(void *handle, const char *symbol);
void *getSymbol(void *handle, const std::string &symbol);
void freeLibrary(void *handle);
}  // namespace utils

namespace http
{
int download(const char *url, const char *path);
int download(const std::string &url, const std::string &path);
}  // namespace http

#endif  // NONBIRI_UTILS_UTILS_H