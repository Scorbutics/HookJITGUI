#ifndef PROCESSUTILS_H
#define PROCESSUTILS_H

int Is64Process(DWORD pid);
BOOL Is64Os();
DWORD GetFirstProcessIdFromProcessName(const char* processName);

#endif // PROCESSUTILS_H
