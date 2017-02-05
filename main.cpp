#include <stdio.h>
#include <windows.h>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include "MainWindow.h"
#include "ArgUtils.h"
#include "PathExeUtils.h"
#include "ProcessUtils.h"
#include "Win32APISeeker.h"
#include "FMSenderIPC.h"
#include "FMReceiverIPC.h"

void OnlyTestPurpose(MainWindow& w) {

    QString fileContent;
    QFile inputFile("D:\\Dev\\Projects\\build-HookJITGUI-Desktop_Qt_5_4_1_MSVC2013_64bit-Release\\release\\DumpIAT.txt");
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          fileContent += in.readLine() + "\n";

       }
       inputFile.close();
    } else {
        std::cout << "Impossible d'ouvrir le fichier" << std::endl;
    }
    //std::cout << fileContent.toStdString().c_str();
    w.buildIATTree(fileContent);
    w.show();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FMSenderIPC msgSender(argc, argv);
    FMReceiverIPC msgReceiver(argc, argv);

    /*if(msgReceiver.getHandle() == NULL) {
        std::cout << "Erreur : ce programme a pour vocation d'etre lance par HookJIT ou HookJIT.x86" << std::endl;
        return 0;
    }*/

    char directoryName[1024] = {0};
    GetCurrentDirectoryName(directoryName, sizeof(directoryName) - 1);
    std::string directory(directoryName);
    std::cout << "Current directory : " << directory << std::endl;

    Win32APISeeker::getInstance().setDirectory(directory + "\\Win32HeaderFiles");


    msgReceiver.deathResponsability(GetProcessId(msgReceiver.getHandle()));
    const char* hHookStarterPid = FindNextArg((const char**)argv, argc, "-HookStarterPID");
    const char* bitsPlateformStart = FindNextArg((const char**)argv, argc, "-bits");
    if(hHookStarterPid != NULL) {
        msgSender.deathResponsability(atoi(hHookStarterPid));
    }

    MainWindow w(NULL, &msgSender, &msgReceiver);
    if(bitsPlateformStart != NULL) {
        if(atoi(bitsPlateformStart) == 64) {
            w.setWindowTitle(w.windowTitle() + " (x64)");
        } else {
            w.setWindowTitle(w.windowTitle() + " (x86)");
        }
    }
    w.setDirectory(directory);
    w.show();

    /*MainWindow w(NULL);
    w.setDirectory(directory);
    OnlyTestPurpose(w);*/

    int result = a.exec();
    return  result;
}


