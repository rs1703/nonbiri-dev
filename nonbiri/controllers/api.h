#ifndef NONBIRI_CONTROLLERS_API_H_
#define NONBIRI_CONTROLLERS_API_H_

#include <httplib.h>

class Api
{
public:
  Api();

  void getExtensions(const httplib::Request &, httplib::Response &);
  void refreshExtensions(const httplib::Request &, httplib::Response &);
  void getExtensionFilters(const httplib::Request &, httplib::Response &);
  void getExtensionPrefs(const httplib::Request &, httplib::Response &);
  void installExtension(const httplib::Request &, httplib::Response &);
  void uninstallExtension(const httplib::Request &, httplib::Response &);
  void updateExtension(const httplib::Request &, httplib::Response &);

  void getLatests(const httplib::Request &, httplib::Response &);
  void searchManga(const httplib::Request &, httplib::Response &);
  void getManga(const httplib::Request &, httplib::Response &);
  void getChapters(const httplib::Request &, httplib::Response &);
  void getPages(const httplib::Request &, httplib::Response &);

  void setMangaReadState(const httplib::Request &, httplib::Response &);
};

#endif  // NONBIRI_CONTROLLERS_API_H_