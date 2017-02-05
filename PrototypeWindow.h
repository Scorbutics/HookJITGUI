#ifndef PROTOTYPEWINDOW_H
#define PROTOTYPEWINDOW_H

#include <QMainWindow>
#include "MainWindow.h"
#include "QListWidget"
#include "QPushButton"

class PrototypeWindow : public QMainWindow
{
    Q_OBJECT
public:
    PrototypeWindow(QStringList &potentialPrototypes, MainWindow *parent = 0);
    ~PrototypeWindow();

private slots:
    void on_m_pushButton_pressed();

signals:
    void generateHooks(QStringList& prototypeList, QStringList& functionList);

private:
    QPushButton* m_pushButton;
    QListWidget* m_treeWidget;
};

#endif // PROTOTYPEWINDOW_H
