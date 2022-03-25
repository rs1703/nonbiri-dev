#include <iostream>

#include <nonbiri/server.h>

Server::Server(int port) : httplib::Server(), mPort {port} {}

void Server::start()
{
  std::cout << "Server listening on " << mPort << std::endl;
  listen("localhost", mPort);
}