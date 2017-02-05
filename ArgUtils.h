#ifndef ARGUTILS_H
#define ARGUTILS_H

const char* FindArg(const char** argStr, unsigned int argc, const char* arg);
const char* FindNextArg(const char** argv, unsigned int argc, const char* arg);

#endif // ARGUTILS_H
