#ifndef NONBIRI_CONTROLLERS_WEB_H_
#define NONBIRI_CONTROLLERS_WEB_H_

#include <httplib.h>
#include <nonbiri/server.h>

class Web
{
public:
  Web(Server &);

  void render(const httplib::Request &, httplib::Response &);
};

#endif  // NONBIRI_CONTROLLERS_WEB_H_