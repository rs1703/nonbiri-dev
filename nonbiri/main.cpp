#include <iostream>
#include <stdexcept>

#include <nonbiri/app.h>

int main(int argc, char *argv[])
{
  try {
    App::initialize(argc, argv);
    App::start();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}
