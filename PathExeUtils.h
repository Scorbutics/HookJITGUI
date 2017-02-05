#ifndef PATHEXEUTILS_H
#define PATHEXEUTILS_H

void StringWithoutExtension(const char* pathToExe, char* output);
void GetExecutablePathName(char* output, unsigned int maxLength);
void GetCurrentDirectoryName(char* output, unsigned int maxLength);

#endif // PATHEXEUTILS_H
