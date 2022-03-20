#ifndef NONBIRI_UTILS_UTILS_H
#define NONBIRI_UTILS_UTILS_H

namespace utils
{
void *loadLibrary(const char *path);
void *getSymbol(void *handle, const char *symbol);
void freeLibrary(void *handle);
}  // namespace utils

namespace http
{
int download(const char *url, const char *path);
}  // namespace http

#endif  // NONBIRI_UTILS_UTILS_H