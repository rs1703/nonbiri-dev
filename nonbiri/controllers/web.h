#ifndef NONBIRI_CONTROLLERS_WEB_H_
#define NONBIRI_CONTROLLERS_WEB_H_

#include <httplib.h>

class Web
{
public:
  Web();

  void render(const httplib::Request &, httplib::Response &);
  void icon(const httplib::Request &, httplib::Response &);
};

#endif  // NONBIRI_CONTROLLERS_WEB_H_