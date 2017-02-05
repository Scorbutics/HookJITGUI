#ifndef REFRESHIATTHREAD_H
#define REFRESHIATTHREAD_H

#include <QThread>
#include <windows.h>
#include <memory>

#include "IResultCallback.h"
#include "HWNDSenderIPC.h"
#include "HWNDReceiverIPC.h"
#include "MainWindow.h"

class RefreshIATCallback;

class RefreshIATThread : public QThread
{
public:
    static RefreshIATThread& getInstance();
    void start(MainWindow* listener, std::string messName, std::string responseName, ISenderIPC* ipcSender, IReceiverIPC* ipcReceiver);
    void abort();

    void continueReading();
private:
    RefreshIATThread();

protected:
    void run();

private:
    std::string message;
    std::string messageSent;
    ISenderIPC* ipcSender;
    IReceiverIPC* ipcReceiver;
    void sendRefreshIAT(bool reset);
    std::unique_ptr<RefreshIATCallback> listener;
    bool aborted;
};

class RefreshIATCallback : public QObject, public IResultCallback {
Q_OBJECT
private:
    MainWindow* wrapped;
    std::string finalContent;

signals:
    void success(WPARAM wParam, LPARAM lParam);

public :
    RefreshIATCallback(MainWindow* wrapped) {
        this->wrapped = wrapped;
         qRegisterMetaType<WPARAM>("WPARAM") ;
         qRegisterMetaType<LPARAM>("LPARAM") ;
        connect(this, SIGNAL(success(WPARAM,LPARAM)), this->wrapped, SLOT(onSuccess(WPARAM,LPARAM)));
    }

    virtual void onSuccess(WPARAM wParam, LPARAM lParam) {
        int bytesRead = (int)wParam;
        std::cout << "Bytes read : " << bytesRead << std::endl;
        if(bytesRead == 0) {
            RefreshIATThread::getInstance().abort();
            emit success((WPARAM) finalContent.size(), (LPARAM)finalContent.c_str());
        } else {
            const char* read = (const char*) lParam;
            finalContent.append(read, bytesRead);
            RefreshIATThread::getInstance().continueReading();
        }
    }

    virtual void onFailure(std::string reason) {
        //TODO log exceptions
        wrapped->onFailure(reason);
    }

    virtual ~RefreshIATCallback() {
    }
};

#endif // REFRESHIATTHREAD_H
