#ifndef NONBIRI_CONTROLLERS_LIBRARY_H_
#define NONBIRI_CONTROLLERS_LIBRARY_H_

#include <drogon/HttpController.h>

namespace api
{
namespace v1
{

class LibraryCtrl : public drogon::HttpController<LibraryCtrl>
{
public:
  METHOD_LIST_BEGIN

  // path: /api/{version}/library
  // method: GET
  METHOD_ADD(LibraryCtrl::getLibrary, "/library", Get);

  // path: /api/{version}/library/search
  // method: GET
  METHOD_ADD(LibraryCtrl::searchLibrary, "/library/search", Get);

  // path: /api/{version}/library
  // method: POST
  METHOD_ADD(LibraryCtrl::updateLibrary, "/library", Post);

  METHOD_LIST_END

  void getLibrary(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;

  void searchLibrary(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;

  void updateLibrary(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;

public:
  LibraryCtrl() = default;
};

}  // namespace v1
}  // namespace api

#endif  // NONBIRI_CONTROLLERS_LIBRARY_H_