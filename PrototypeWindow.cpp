#include <iostream>

#include "PrototypeWindow.h"
#include "QTreeWidgetItem"
#include "Win32APISeeker.h"

PrototypeWindow::PrototypeWindow(QStringList& potentialPrototypes, MainWindow *parent) : QMainWindow(parent) {
    resize(800, 600);
    QWidget* popupCentralWidget = new QWidget(this);
    setCentralWidget(popupCentralWidget);
    popupCentralWidget->setObjectName(QStringLiteral("popupCentralWidget"));
    m_treeWidget = new QListWidget(popupCentralWidget);
    m_treeWidget->setGeometry(QRect(10, 10, 541, 531));
    m_treeWidget->setObjectName(QStringLiteral("popupTreeWidget"));
    m_treeWidget->setFrameShadow(QFrame::Plain);
    m_treeWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    for(QStringList::iterator it = potentialPrototypes.begin(); it != potentialPrototypes.end(); it++) {
        std::cout << it->toStdString().c_str() << std::endl;
        QListWidgetItem* entry = new QListWidgetItem(m_treeWidget) ;
        entry->setText(*it);
        m_treeWidget->addItem(entry);
    }

    m_pushButton = new QPushButton(popupCentralWidget);
    m_pushButton->setText(QStringLiteral("Validate"));
    m_pushButton->setGeometry(QRect(561, 10, 50, 20));

    QObject::connect(m_pushButton, SIGNAL(pressed()), this, SLOT(on_m_pushButton_pressed()));
    QObject::connect(this, SIGNAL(generateHooks(QStringList&,QStringList&)), parent, SLOT(onGenerateHooks(QStringList&,QStringList&)));

}

void PrototypeWindow::on_m_pushButton_pressed() {
    QList<QListWidgetItem*> selectedItems = m_treeWidget->selectedItems();
    QStringList prototypesSelected;
    for(QListWidgetItem* wi : selectedItems) {
        prototypesSelected << wi->text();
    }

    QStringList functions = Win32APISeeker::getInstance().writeValidatedPrototypes(prototypesSelected);
    hide();
    emit generateHooks(prototypesSelected, functions);
}

PrototypeWindow::~PrototypeWindow()
{

}

