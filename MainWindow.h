#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>

#include "HWNDSenderIPC.h"
#include "IReceiverIPC.h"
#include "IResultCallback.h"


class RefreshIATThread;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, IResultCallback
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent);
    explicit MainWindow(QWidget *parent, ISenderIPC* ipcSender, IReceiverIPC* ipcReceiver);
    virtual void onFailure(std::string reason);
    QStringList::iterator processIATTreeLineAdd(QStringList& strList, QStringList::iterator& start, QString& patternToMatch, QString& unitPattern, QTreeWidgetItem* parent);
    void buildIATTree(QString& iatContent);
    void setDirectory(std::string& directory);
    QTreeWidgetItem* addIATEntry(QColor& color, QTreeWidgetItem* parent, QString& addressEntry, QString& nameEntry);
    virtual ~MainWindow();

private slots:
    void on_pushButton_pressed();
    virtual void onSuccess(WPARAM wParam, LPARAM lParam);
    void onGenerateHooks(QStringList& prototypeList, QStringList& functionList);

private:
    std::string m_directory;
    Ui::MainWindow *ui;
    ISenderIPC* ipcSender;
    IReceiverIPC* ipcReceiver;
};

#endif // MAINWINDOW_H
