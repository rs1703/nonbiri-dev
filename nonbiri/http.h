#ifndef NONBIRI_HTTP_H_
#define NONBIRI_HTTP_H_

#include <core/http.h>

namespace Http
{
void initialize();
void attach(Http::initialize_t initializeFnPtr);
};  // namespace Http

#endif  // NONBIRI_HTTP_H_