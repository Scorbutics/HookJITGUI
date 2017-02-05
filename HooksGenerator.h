#ifndef HOOKSGENERATOR_H
#define HOOKSGENERATOR_H

#include <QStringList>
#include <QString>
#include <vector>
#include <unordered_map>

class HooksGenerator
{
private:
    HooksGenerator();
    std::vector<std::unordered_map<std::string, QString>> m_data;

public:
    static HooksGenerator& getInstance();
    void initContext(QStringList& prototypes, QStringList& functions);
    void generateFile(std::string &filename, std::string &output);
    ~HooksGenerator();

};

#endif // HOOKSGENERATOR_H
