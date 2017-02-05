#include <QDirIterator>
#include <iostream>
#include <QRegularExpression>
#include <QTextStream>
#include "Win32APISeeker.h"

//Matches everything like ";" or " ;" or "  ;", etc, same with "#"
static QString endLineSeparator("\\s*(;|#)");
static QRegularExpression separatorPattern(endLineSeparator);

//File name of database of valid known prototypes
static QString validPrototypesDBFile("prototypes.db");

//Matches all "#" at line start
static QString preprocessorSeparator("^#");
static QRegularExpression preprocessorPattern(preprocessorSeparator);

Win32APISeeker::Win32APISeeker()
{
}

QStringList Win32APISeeker::getPrototypesOf(QStringList& functions, bool* dbIntern) {
    QDirIterator it(m_dir, QStringList() << "*.h", QDir::Files, QDirIterator::Subdirectories);
    QStringList occurences;
    QStringList result;
    QString prototypeMatcher (("\\s(" + getRegexFromStrings(functions) + ")\\s*\\("));
    QString virtualStr("\\)\\s*=\\s*0\\s*;");
    QRegularExpression patternVirtual(virtualStr);
    QRegularExpression patternPrototypeMatcher(prototypeMatcher);

    QString dbFileName(m_dir + "\\" + validPrototypesDBFile);
    QStringList occurencesInDb = searchOccurencesInFile(dbFileName, functions);
    if(occurencesInDb.length() == functions.length()) {
        std::cout << "OK : All the functions are already in local DB" << std::endl;
        *dbIntern = true;
        return occurencesInDb;
    }

    std::cout << "Prototype matcher : " << prototypeMatcher.toStdString().c_str() << std::endl;
    std::cout << "Files : " << std::endl;
    while (it.hasNext()) {
        const QString& fileName =  it.next();
        //std::cout << fileName.toStdString().c_str() << std::endl;
        occurences << searchOccurencesInFile(fileName, functions);
    }

    for(QStringList::iterator it = occurences.begin(); it != occurences.end(); it++) {
        std::string currentStr = it->toStdString();

        //Doit respecter : le début d'un prototype (le nom de la fonction suivi de '(')
        //Ne doit pas être virtuel pur (doit ne pas respecter patternVirtual)
        //Doit contenir ';' pour être un prototype valide
        if(it->contains(patternPrototypeMatcher) && !it->contains(patternVirtual) && it->contains(";")) {
            result << *it;
            std::cout << currentStr.c_str() << std::endl << std::endl;
        }
    }

    *dbIntern = false;
    return result;
}

/**
* Retourne une liste de fonctions a partir de leur prototype
* (utilise l'espace et la parenthese pour trouver le nom de la fonction)
*/
QStringList Win32APISeeker::getFunctionsOf(QStringList& prototypes) {
    QStringList result;
    QString lParenthesisStr("(");
    QString rParenthesisStr(")");
    int parenthesisCount = 0;
    QString spaceStr(" ");
    for(QStringList::iterator prototype = prototypes.begin(); prototype != prototypes.end(); prototype++) {
        QString& current = (*prototype);

        /* Position de la parenthese la plus a droite du prototype */
        int rParenthesisIndex = current.lastIndexOf(rParenthesisStr);
        if(rParenthesisIndex > 0) {

            /* On compte toutes les parentheses en effectuant +1 pour parenthese droite et -1 pour gauche.
             * Quand notre compte est a 0, on s'arrete : on a trouve la fin du nom de la fonction */
            parenthesisCount++;
            int cursor;
            for(cursor = rParenthesisIndex-1; parenthesisCount > 0 && cursor >= 0; cursor--) {
                QChar currentChar = current.at(cursor);
                if(currentChar == lParenthesisStr.at(0)) {
                    parenthesisCount--;
                } else if(currentChar == rParenthesisStr.at(0)) {
                    parenthesisCount++;
                }
            }

            /* On recupere donc ici soit une parenthese gauche, soit -1 si rien n'est trouve */
            int lParenthesisIndex = cursor;
            if(lParenthesisIndex > 0) {
                /* On cherche ensuite le premier espace avant le nom de la fonction */
                int spaceIndex = current.left(lParenthesisIndex).lastIndexOf(spaceStr);
                if(spaceIndex > 0) {
                    QString func = current.mid(spaceIndex+1, lParenthesisIndex - spaceIndex).trimmed();
                    result << func;
                    std::cout << func.toStdString() << std::endl;
                }
            }
        }
    }
    return result;
}

/**
  Traduit une liste de chaînes de caractères en une regex équivalente
  (pour match la regex, la chaîne testée doit contenir une des chaînes de
  caractères)
*/
QString Win32APISeeker::getRegexFromStrings(QStringList& list) {
    //Matches the functions' name
    QString qStrFuncName;
    for(QStringList::iterator it = list.begin(); it != list.end(); it++) {
        qStrFuncName += (*it + "|");
    }

    if(!qStrFuncName.isEmpty()) {
        qStrFuncName = qStrFuncName.left(qStrFuncName.length()-1);
    }

    return qStrFuncName;
}

/**
    Récupère le début d'un prototype (lit en arrière depuis "pos", puis s'arrête dès qu'on trouve '#' ou ';')
**/
QString getStartPrototype(QFile& file, const qint64 pos, const qint64 finalPos) {
    int count = 0;
    QString ch;
    QString result;

    file.seek(pos-2);

    while (file.pos() > 0 && ch != ";" && ch != "#") {
        //Read 1 char
        ch = file.read(1);



        //Count line or not
        if (ch == "\n") {
            count++;
            //Go back
            if(!file.seek(file.pos()- 3)) {
                std::cout << "error while seek" << std::endl;
                break;
            }
        } else {
            //Go back
            if(!file.seek(file.pos()- ch.length() - 1)) {
                std::cout << "error while seek" << std::endl;
                break;
            }
        }
    }

    //std::cout << "count = " << count << std::endl;

    //first line doesn't matter
    if(count > 0) {
        file.readLine();
        count--;
    }

    QString line;
    for(int i = 0; i < count; i++) {
        line = file.readLine();
        if(!line.contains(preprocessorPattern)) {
            result += line;
        }
    }

    file.seek(finalPos);
    return result;
}

/**
  A partir d'une ligne 'line' lue dans un fichier 'in' matchant un nom de fonction,
  on en extrait le prototype complet
*/
QString extractPrototypeFromLine(QFile& in, QString& line) {
    QString lineBuffer;

    if(!line.contains(preprocessorPattern)) {
        /* récupération du début du prototype */
        lineBuffer += getStartPrototype(in, in.pos() - line.length() - 1, in.pos());

        /* récupération de la fin du prototype */
        for(; !in.atEnd() && !line.contains(separatorPattern); line = in.readLine()) {
            lineBuffer += (line + " ");
        }
        if(line.contains(separatorPattern)) {
            lineBuffer += line;
        }
    } else {
        lineBuffer = line;
    }

    return lineBuffer;
}

/**
  Récupère tous les occurences détectées comme prototypes potentiels des fonctions "functions"
  dans le fichier "fileName"
*/
QStringList Win32APISeeker::searchOccurencesInFile(const QString& fileName, QStringList& functions) {
    QFile file(fileName);

    if(file.open(QFile::ReadOnly | QFile::Text)) {
        return searchOccurencesInFile(file, functions);
    } else {
        std::cout << "Error while opening the file " << fileName.toStdString() << " in the current directory \"" << m_dir.toStdString() << "\"" << std::endl;
        return QStringList();
    }
}

/**
*/
QStringList Win32APISeeker::searchOccurencesInFile(QFile& file, QStringList& functions) {
    QStringList result;

    /* liste des fonctions sous forme de regex */
    QRegularExpression functionPattern(getRegexFromStrings(functions));
    while(!file.atEnd()) {
        QString line = file.readLine();
        if(line.contains(functionPattern)) {
            QRegularExpression endLine("\n");
            QString spaceStr(" ");
            result << extractPrototypeFromLine(file, line).replace(endLine, spaceStr);
        }
    }
    file.reset();
    file.seek(0);
    return result;
}

Win32APISeeker& Win32APISeeker::getInstance() {
    static Win32APISeeker instance;
    return instance;
}

QStringList Win32APISeeker::writeValidatedPrototypes(QStringList& prototypes) {

    QString fileName(m_dir + "\\" + validPrototypesDBFile);

    QFile file(fileName);
    QFile fileRead(fileName);
    QStringList functionsList;
    if(file.open(QFile::Append | QFile::Text) && fileRead.open(QFile::ReadOnly | QFile::Text)) {
        for(QStringList::iterator it = prototypes.begin(); it != prototypes.end(); it++) {

            QStringList current;
            current << *it;
            QStringList function = getFunctionsOf(current);
            functionsList.append(function);
            if(searchOccurencesInFile(fileRead, function).length() == 0) {
                file.write((it->toStdString()+ "\n").c_str(), it->length() + 1);
            }
        }
        file.close();
    }
    return functionsList;
}

void Win32APISeeker::setDirectory(std::string path) {
    m_dir = QString(path.c_str());
}
