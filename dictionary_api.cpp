#include "dictionary_api.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QDebug>
#include <QMessageBox>

Example::Example()
{}

Example::~Example()
{}

Translation::Translation()
{}

Translation::~Translation()
{}

DictionaryEntry::DictionaryEntry()
{}

DictionaryEntry::~DictionaryEntry()
{}

YandexDictionary::YandexDictionary()
{}

YandexDictionary::~YandexDictionary()
{}


void Translation::showTranslation()
{
    QString header = (this->text + " "
                      + this->pos + " "
                      + this->gen + " "
                      + this->asp + " "
                      + this->num); // Header of a translation.
    qDebug() << header.trimmed();

    if(!this->synonyms.isEmpty())
    {
        QStringList::const_iterator iter;
        qDebug() << "Synonyms:";
        for(iter = this->synonyms.constBegin(); iter != this->synonyms.constEnd(); ++iter)
        {
            qDebug() << *iter;
        }
    }

    if(!this->meanings.isEmpty())
    {
        QStringList::const_iterator iter;
        qDebug() << "Meanings:";
        for(iter = this->meanings.constBegin(); iter != this->meanings.constEnd(); ++iter)
        {
            qDebug() << *iter;
        }
    }

    int i = 1;  // An example number.
    QVector<Example>::iterator it;
    for(it = this->examples.begin(); it != this->examples.end(); ++it, ++i)
    {
        qDebug() << "Example" << i;
        it->showExample();
    }
}

void DictionaryEntry::showEntry()
{
    QString header = (this->text + " [" + this->ts + "] " + this->pos); // Header of an entry.
    qDebug() << header.trimmed();   // Get rid of spaces in the beginning and in the end of a string.

    int i = 1;  // A translation number.
    QVector<Translation>::iterator iter;
    for(iter = this->translations.begin(); iter != this->translations.end(); ++iter, ++i)
    {
        qDebug() << "Translation" << i;
        iter->showTranslation();
    }
}

void YandexDictionary::getLanguages(QMap<QString, QString> languages_)
{
    this->languages = languages_;
}

void YandexDictionary::getTranslationDirections()
{
    // Create a request.
    QUrl url(QString("https://dictionary.yandex.net/api/v1/dicservice.json/getLangs?")
             + QString("key=") + dict_api_key
             );

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // Send the request.
    QEventLoop loop;
    QNetworkAccessManager manager;
    connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    // Receive a reply.
    QNetworkReply* reply = manager.get(request);
    loop.exec();

    // Parse the reply.

    QByteArray bytes = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if (doc.isObject())
    {
        QJsonObject obj = doc.object();
        // Check the response codes.
        if(obj.contains("code") && obj.contains("message"))
        {
            QMessageBox messageBox;
            messageBox.critical(0,QString::number(obj.value("code").toDouble()),obj.value("message").toString());
            messageBox.setFixedSize(500,200);
            // TODO: Error handling.
        }
    }
    else if(doc.isArray())
    {
        QJsonArray dirs = doc.array();
        for(QJsonValue v : dirs)
        {
            directions << v.toString();
        }
    }
}

QVector<DictionaryEntry> YandexDictionary::getDictionaryEntry(QString& source_lang, QString& target_lang, QString& text)
{
    QString dict_dir = languages.key(source_lang) + "-" + languages.key(target_lang);


    // Create a request.
    QUrl url(QString("https://dictionary.yandex.net/api/v1/dicservice.json/lookup?")
             + QString("key=") + dict_api_key
             + QString("&lang=") + dict_dir
             + QString("&text=") + text
             );

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // Send the request.
    QEventLoop loop;
    QNetworkAccessManager manager;
    connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    // Receive a reply.
    QNetworkReply* reply = manager.get(request);
    loop.exec();

    // Parse the reply.
    QByteArray bytes = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(bytes);\
    QJsonObject obj = doc.object(); // Keys are ("def", "head")

    // Check the response codes.
    if(obj.contains("code") && obj.contains("message"))
    {

        QMessageBox messageBox;
        messageBox.critical(0,QString::number(obj.value("code").toDouble()),obj.value("message").toString());
        messageBox.setFixedSize(500,200);
        messageBox.show();
        // TODO: Error handling.
    }


    QVector<DictionaryEntry> entries;
    QJsonArray def = obj.value("def").toArray();    // Dictionary entries.
    for (QJsonValue v : def)
    {
        DictionaryEntry entry;
        QJsonObject o = v.toObject();
        //qDebug() << o.keys();
        if(o.contains("text"))  // Text of an entry.
        {
            QString word = o.value("text").toString();
            entry.setText(word);
        }

        if(o.contains("ts"))    // Transcription.
        {
            QString ts =o.value("ts").toString();
            entry.setTranscription(ts);
        }

        if(o.contains("pos"))   // Part of speech.
        {
            QString pos = o.value("pos").toString();
            entry.setPartOfSpeech(pos);
        }
        if(o.contains("tr"))    // Translations.
        {
            QJsonArray tr = o.value("tr").toArray();
            for (QJsonValue val : tr)
            {
                Translation translation;
                QJsonObject obj = val.toObject();
                //qDebug() << obj.keys();
                if(obj.contains("text"))// Text of a translation.
                {
                    QString txt = obj.value("text").toString();
                    translation.setText(txt);
                }
                if(obj.contains("pos")) // Part of speech.
                {
                    QString ps = obj.value("pos").toString();
                    translation.setPartOfSpeech(ps);
                }
                if(obj.contains("asp")) // The aspect of a verb.
                {
                    QString asp = obj.value("asp").toString();
                    translation.setAspect(asp);
                }
                if(obj.contains("num")) // The form of a noun and its variations: plural, etc.
                {
                    QString num = obj.value("num").toString();
                    translation.setNounForm(num);
                }
                if(obj.contains("gen")) // Gender.
                {
                    QString gen = obj.value("gen").toString();
                    translation.setGender(gen);
                }
                if(obj.contains("syn")) // Synonyms.
                {
                    QJsonArray syn = obj.value("syn").toArray();
                    for(QJsonValue value : syn)
                    {
                        QString syn_txt = value.toObject().value("text").toString();
                        translation.addSynonym(syn_txt);
                    }
                }
                if(obj.contains("mean"))// Meanings.
                {
                    QJsonArray mean = obj.value("mean").toArray();
                    for(QJsonValue value : mean)
                    {
                        QString mean_txt = value.toObject().value("text").toString();
                        translation.addMeaning(mean_txt);
                    }
                }
                entry.addTranslation(translation);  // Add another translation.
            }
        }
        entries.push_back(entry);   // Add another entry.
    }

    return entries;
}
