#include <windows.h>
#include <stdio.h>
#include <Psapi.h>

#include "ProcessUtils.h"

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

int IsWow64ProcessCaller(HANDLE hProcess)
{
    BOOL bIsWow64 = FALSE;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(fnIsWow64Process != NULL)
    {
        if (!fnIsWow64Process(hProcess,&bIsWow64))
        {
            printf("Erreur lors du test de profondeur de bits du processus\n");
            //Erreur : par défaut on choisi 32 bits
            return -1;
        }
    }
    else
    {
        //IsWow64Process introuvable : OS 32 bits
        return 2;
    }

    //Si bisWow64 == 1, OS 64 bits sinon impossible de savoir
    return bIsWow64 ? 1 : 0;
}

int Is64Process(DWORD pid)
{
    DWORD dwVersion, dwMajorVersion, dwMinorVersion;
    dwVersion = GetVersion();

    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

//    printf("OS Version : %u.%u\n", dwMajorVersion, dwMinorVersion);

    if((dwMajorVersion < 5 || (dwMajorVersion == 5 && dwMinorVersion == 1)))
    {
        return 2;
    }
    else
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        int wow64 = IsWow64ProcessCaller(hProcess);
//        printf("pid = %u (wow64 : %i)\n", pid, wow64);
        CloseHandle(hProcess);
        return !wow64;
    }
}


BOOL Is64Os()
{
//    printf("int* = %u\n", sizeof(int*));
    return (sizeof(int*) == 8) || (Is64Process(GetCurrentProcessId()) == 0);
}

DWORD GetFirstProcessIdFromProcessName(const char* processName)
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
        printf("Erreur lors de l'enumeration des processus\n");
        return -1;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    for ( i = 0; i < cProcesses; i++ )
    {
        if( aProcesses[i] != 0 )
        {
            HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                               PROCESS_VM_READ,
                                               FALSE, aProcesses[i] );
            if(hProcess != NULL)
            {
                char name[2048] = {'\0'};
                GetModuleBaseNameA(hProcess, NULL, name, sizeof(name) - 1);
                printf("name = %s\npid = %u\n", name, (unsigned int) aProcesses[i]);
                if(strstr(name, processName) != NULL)
                {
                    //printf("name = %s\npid = %u\n", name, aProcesses[i]);
                    return aProcesses[i];
                }
            }
            CloseHandle(hProcess);
        }
    }
    printf("Erreur : impossible de trouver le processus correspondant a \"%s\"\n", processName);
    return -1;
}
