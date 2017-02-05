#include <QFile>
#include <iostream>
#include <QRegularExpression>
#include "HooksGenerator.h"

using namespace std;

HooksGenerator::HooksGenerator()
{

}

void HooksGenerator::initContext(QStringList& prototypes, QStringList& functions) {
    unsigned int index = 0;
    for(QStringList::iterator it = prototypes.begin(); it != prototypes.end(); it++, index++) {
        m_data.push_back(std::unordered_map<std::string, QString>());
        m_data[index]["<prototype>"] = *it;
    }

    index = 0;
    for(QStringList::iterator it = functions.begin(); it != functions.end(); it++, index++) {
        m_data[index]["<function_name>"] = *it;
        m_data[index]["<function_hook_name>"] = QString("Hook") + *it;
        m_data[index]["<prototype_hook>"] = QString(m_data[index]["<prototype>"]).replace(m_data[index]["<function_name>"], m_data[index]["<function_hook_name>"]);
        m_data[index]["<prototype_typedef>"] = QString(m_data[index]["<prototype>"]).replace(m_data[index]["<function_name>"], QString("(_type") + m_data[index]["<function_name>"] + ")");
    }

}


void HooksGenerator::generateFile(std::string& filename, std::string& output) {
    QString qFileName(filename.c_str());
    QString qOutputFileName(output.c_str());
    QFile file(qFileName);
    QFile fileOutput(qOutputFileName);
    if(!file.open(QFile::ReadOnly) || !fileOutput.open(QFile::WriteOnly)) {
        return;
    }

    QString content = file.readAll();
    QStringList list = content.split("\n");
    file.close();

    std::cout << filename << std::endl;


    //std::cout << m_data[index]["<function_name>"].toStdString() << std::endl;
    bool insideBlock = false;
    std::vector<QString> finalValueList(m_data.size());
    for(QStringList::iterator it = list.begin(); it != list.end(); it++) {
        int index = 0;


        if(it->toStdString().find("<function_list>") != std::string::npos) {
            insideBlock = true;
            it->clear();
        } else if(it->toStdString().find("</function_list>") != std::string::npos) {
            insideBlock = false;
            it->clear();
            for(const QString& s : finalValueList) {
                *it += s;
            }
            finalValueList.clear();
        } else if(insideBlock) {
            QString original = *it;

            for(index = 0; index < m_data.size(); index++) {
                QString dataValueList = original;
                for(const auto &kv : m_data[index]) {
                    if(it->toStdString().find(kv.first) != std::string::npos) {
                        QString qStringFirst(kv.first.c_str());
                        QRegularExpression localQRegFirstVal(qStringFirst);
                        dataValueList.replace(localQRegFirstVal, kv.second);
                        std::cout << "FOUND !!!! " << " in " << it->toStdString() << std::endl << kv.second.toStdString() << std::endl << std::endl;
                    }
                }
                finalValueList[index] += dataValueList;
            }
            it->clear();
        }

    }


    for(QStringList::iterator it = list.begin(); it != list.end(); it++) {
        fileOutput.write((*it + "\n").toStdString().c_str(), it->size() + 1);
    }

    fileOutput.close();
}

HooksGenerator& HooksGenerator::getInstance() {
    static HooksGenerator instance;
    return instance;
}

HooksGenerator::~HooksGenerator()
{

}

