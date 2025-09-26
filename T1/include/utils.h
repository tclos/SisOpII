#ifndef UTILS_H
#define UTILS_H

#include <string>

std::string getCurrentFormattedTime();
bool validatePortArg(int argc, char* argv[]);
bool validatePort(int port);
int getValidatedPort(int argc, char* argv[]);

#endif // UTILS_H
