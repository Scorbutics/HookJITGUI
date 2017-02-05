#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Win32APISeeker.h"
#include "PrototypeWindow.h"
#include "RefreshIATThread.h"
#include "HooksGenerator.h"

#include <QListWidget>

#define MESSAGE_HOOK_JIT_QUERY_IAT "HOOK_JIT_QUERY_IAT"
#define MESSAGE_HOOK_JIT_PROVIDE_IAT "HOOK_JIT_PROVIDE_IAT"

using namespace std;

MainWindow::MainWindow(QWidget *parent, ISenderIPC* ipcSender, IReceiverIPC* ipcReceiver) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->ipcSender = ipcSender;
    this->ipcReceiver = ipcReceiver;

    ui->setupUi(this);
    ui->treeWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    RefreshIATThread::getInstance().start(this, MESSAGE_HOOK_JIT_QUERY_IAT, MESSAGE_HOOK_JIT_PROVIDE_IAT, ipcSender, ipcReceiver);

}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->treeWidget->setSelectionMode(QAbstractItemView::MultiSelection);
}

void MainWindow::onSuccess(WPARAM wParam, LPARAM lParam) {
    if(lParam != NULL) {
        const char * buffer = (const char*) lParam;

        char* bufferLocal = (char*) calloc((unsigned int) wParam, sizeof(char));
        memcpy(bufferLocal, buffer, (unsigned int) wParam);
        bufferLocal[(unsigned int) wParam - 1] = '\0';

        QString finalContent(bufferLocal);
        buildIATTree(finalContent);
        free(bufferLocal);
    }
}

QStringList::iterator MainWindow::processIATTreeLineAdd(QStringList& strList, QStringList::iterator& start, QString& patternToMatch, QString& unitPattern, QTreeWidgetItem* parent) {
    static QRegularExpression regexp(unitPattern);
    QString nextPattern(patternToMatch + unitPattern);

    for (QStringList::iterator it = start; it != strList.end(); ++it) {
        if(it->isEmpty()) {
            continue;
        }

        if(it->startsWith(patternToMatch) && !it->startsWith(nextPattern)) {
            QStringList lineContent = it->mid(patternToMatch.length()).split(regexp);


            QStringList::iterator nextIt = it;
            QTreeWidgetItem* nextParent;
            QColor color(0, 0, 0);
            if(lineContent.length() == 2) {
                 nextParent = addIATEntry(color, parent, lineContent[0], lineContent[1]);

            } else {
                QString empty;
                nextParent = addIATEntry(color, parent, empty, *it);
            }
            it = processIATTreeLineAdd(strList, ++nextIt, nextPattern, unitPattern, nextParent);
            if(it == strList.end()) {
                return strList.end();
            }
        } else {
            QStringList::iterator lastIt = it;
            return --lastIt;
        }

    }
    return strList.end();
}

void MainWindow::buildIATTree(QString& iatContent) {
    QRegularExpression regexp("\n");
    QStringList strList = iatContent.split(regexp);

    QTreeWidgetItem* root = new QTreeWidgetItem(ui->treeWidget);
    root->setText(0,  "Modules");
    ui->treeWidget->addTopLevelItem(root);
    QTreeWidgetItem* lastParent = root;

    QString pattern("\t");
    QString empty;
    QStringList::iterator begin = strList.begin();
    processIATTreeLineAdd(strList, begin, empty, pattern, lastParent);

    ui->treeWidget->sortItems(0, Qt::AscendingOrder);
    root->setExpanded(true);
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(1);

    std::cout << "Tree built" << std::endl;
}

QTreeWidgetItem* MainWindow::addIATEntry(QColor& color, QTreeWidgetItem* parent, QString& addressEntry, QString& nameEntry) {
    QTreeWidgetItem* entry = new QTreeWidgetItem(parent) ;
    entry->setText(1, addressEntry);
    entry->setText(0, nameEntry);
    entry->setTextColor(0, color);

    parent->addChild(entry);
    return entry;
}

void MainWindow::onFailure(std::string reason) {
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onGenerateHooks(QStringList& prototypeList, QStringList& functionList) {
    HooksGenerator hooksGenerator = HooksGenerator::getInstance();
    hooksGenerator.initContext(prototypeList, functionList);
    std::string dllmain(m_directory + "\\pattern_dllmain_cpp.txt");
    std::string hooks_cpp(m_directory + "\\pattern_hooks_cpp.txt");
    std::string hooks_h(m_directory + "\\pattern_hooks_h.txt");
    std::string dllmainOutput(m_directory + "\\pattern_dllmain_cpp_output.txt");
    std::string hooks_cppOutput(m_directory + "\\pattern_hooks_cpp_output.txt");
    std::string hooks_hOutput(m_directory + "\\pattern_hooks_h_output.txt");
    hooksGenerator.generateFile(dllmain, dllmainOutput);
    hooksGenerator.generateFile(hooks_cpp, hooks_cppOutput);
    hooksGenerator.generateFile(hooks_h, hooks_hOutput);
}

void MainWindow::setDirectory(string &directory) {
    m_directory = directory;
}

void MainWindow::on_pushButton_pressed()
{
    QList<QTreeWidgetItem*> selectedItems = ui->treeWidget->selectedItems();
    QStringList iatEntriesSelected;
    for(QTreeWidgetItem* wi : selectedItems) {
        iatEntriesSelected << wi->text(0);
        std::cout << wi->text(0).toStdString().c_str() << std::endl;
    }

    bool dbIntern;
    QStringList potentialPrototypes = Win32APISeeker::getInstance().getPrototypesOf(iatEntriesSelected, &dbIntern);
    if(!dbIntern) {
        PrototypeWindow* popupWidget = new PrototypeWindow(potentialPrototypes, this);
        popupWidget->show();
    } else {
        onGenerateHooks(potentialPrototypes, iatEntriesSelected);
    }

}
