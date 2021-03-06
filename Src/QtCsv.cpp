#include "QtCsv.h"

#include <QDebug>
#include <QTextCodec>

QtCsv::QtCsv()
{
    m_file = NULL;
}

QtCsv::~QtCsv()
{
    if (m_file)
        delete m_file;
}

bool QtCsv::open(const QString &fileName)
{
    m_file = new QFile(fileName);
    if (!m_file->open(QIODevice::ReadWrite)) {
        qDebug()<<"[error] "<<QString("Open %1 file failed!").arg(fileName);
        return false;
    }
    else {
        return true;
    }
}

QStringList QtCsv::splitCSVLine(const QString &lineStr)
{
    QStringList strList;
    QString str;

    int length = lineStr.count();
    int usageCount = 0;
    int repeatCount = 0;

    for(int i = 0; i < length; ++i) {
        if(lineStr[i] != '\"') {
            repeatCount = 0;
            if(lineStr[i] != ',') {
                str.append(lineStr[i]);
            }
            else {
                if(usageCount % 2) {
                    str.append(',');
                }
                else {
                    strList.append(str.trimmed());
                    usageCount = 0;
                    str.clear();
                }
            }
        }
        else {
            ++usageCount;
            ++repeatCount;
            if(repeatCount == 4) {
                str.append('\"');
                repeatCount = 0;
                usageCount -= 4;
            }
        }
    }
    strList.append(str.trimmed());

    return strList;
}

QList<QVariantMap> QtCsv::readAll()
{
    QList<QVariantMap> data;

    if (m_file == NULL)
        return data;

    bool isFirstLine = true;
    QStringList titles;

    while (!m_file->atEnd()) {
        QByteArray line = m_file->readLine();
        QStringList contents = splitCSVLine(QString::fromLocal8Bit(line));
        qDebug()<<contents;
        if (isFirstLine) {
            isFirstLine = false;
            titles = contents;
        }

        QVariantMap item;
        for (int i = 0; i < contents.count(); i++) {
            if (i >= titles.count())
                continue;

            item.insert(/*item.find(titles[i]), */titles[i], contents.at(i));
        }

        data.append(item);
    }

    return data;
}

bool QtCsv::write(const QList<QVariantMap> &maps)
{
    if (m_file == NULL)
        return false;

    m_file->resize(0);

    /* 将内容写入到文件 */
    QString content;
    QString titles;
    for (int i = 0; i < maps.size(); i++) {
        const QVariantMap &item = maps[i];

        QMapIterator<QString, QVariant> iter(item);
        while (iter.hasNext()) {
            iter.next();

            if (i == 0) {
                titles.append(QString("%1,").arg(iter.key()));
                if (!iter.hasNext())
                    titles.append(QString("\n"));
            }
            else {
                content.append(QString("%1,").arg(iter.value().toString()));
                if (!iter.hasNext())
                    content.append(QString("\n"));
            }
        }

        if (i == 0)
            m_file->write(titles.toLocal8Bit(), titles.size());

        m_file->write(content.toLocal8Bit(), content.size());
    }

    return true;
}

void QtCsv::close()
{
    m_file->close();
}
