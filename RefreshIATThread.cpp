#include <stdio.h>
#include <windows.h>
#include "RefreshIATThread.h"
#include "MainWindow.h"

RefreshIATThread::RefreshIATThread() {
    aborted = true;
}

void RefreshIATThread::abort() {
    aborted = true;
}

RefreshIATThread& RefreshIATThread::getInstance() {
    static RefreshIATThread instance;
    return instance;
}

void RefreshIATThread::start(MainWindow* listener, std::string messName, std::string responseName, ISenderIPC* ipcSender, IReceiverIPC* ipcReceiver) {
    if(aborted) {
        std::cout << "Starting Thread to query IAT " << std::endl;

        message = messName;
        messageSent = responseName;
        aborted = false;
        this->listener = std::unique_ptr<RefreshIATCallback>(new RefreshIATCallback(listener));
        this->ipcSender = ipcSender;
        this->ipcReceiver = ipcReceiver;
        ipcReceiver->addListener(messageSent, this->listener.get());
        /* tempo pour attendre que le processus parent ait bien set son listener */
        //Sleep(1000);
        sendRefreshIAT(true);
        QThread::start();
    }
}

void RefreshIATThread::run() {
    ipcReceiver->listenMessages(&aborted, 1);
    std::cout << "Thread finished" << std::endl;
}

void RefreshIATThread::continueReading() {
    sendRefreshIAT(false);
}

void RefreshIATThread::sendRefreshIAT(bool reset) {
    ipcSender->send(message, (WPARAM)reset, (LPARAM)"NULL");
}
