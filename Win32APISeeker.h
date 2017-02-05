#ifndef WIN32APISEEKER_H
#define WIN32APISEEKER_H
#include <QStringList>
#include <QFile>
#include <string>

class Win32APISeeker
{
private:
    Win32APISeeker();
    QStringList searchOccurencesInFile(const QString& fileName, QStringList &functions);
    QStringList searchOccurencesInFile(QFile& file, QStringList &functions);
    QString getRegexFromStrings(QStringList& list);
    QStringList getFunctionsOf(QStringList& prototypes);

    QString m_dir;

public:
    void setDirectory(std::string path);
    QStringList getPrototypesOf(QStringList& functions, bool* dbIntern);
    static Win32APISeeker& getInstance();
    QStringList writeValidatedPrototypes(QStringList& prototypes);
};

#endif // WIN32APISEEKER_H
