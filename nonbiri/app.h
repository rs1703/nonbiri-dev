#ifndef NONBIRI_APP_H_
#define NONBIRI_APP_H_

namespace App
{
extern bool daemonize;
extern int port;

void initialize(int argc, char *argv[]);
void start();
};  // namespace App

#endif  // NONBIRI_APP_H_