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

void Translation::showTranslation() const
{
    QString header = (this->text + " "
                      + this->pos + " "
                      + this->gen + " "
                      + this->asp + " "
                      + this->num); // Header of a translation.
    if(!this->synonyms.isEmpty())
    {
        qDebug() << "Synonyms:";
        for(const auto& word :this->synonyms)
        {
            qDebug() << word;
        }
    }
    if(!this->meanings.isEmpty())
    {
        qDebug() << "Meanings:";
        for(const auto& mean : this->meanings)
        {
            qDebug() << mean;
        }
    }

}

void DictionaryEntry::showEntry()
{
    QString header = (this->text + " [" + this->ts + "] " + this->pos); // Header of an entry.
    int i = 1;  // A translation number.
    for(const auto& trans :this->translations)
    {
        qDebug() << "Translation" << i;
        i++;
        trans.showTranslation();
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
        if(o.contains("text"))          { QString word = o.value("text").toString();    entry.setText(word);}           // Text of an entry.
        if(o.contains("ts"))            {QString ts =o.value("ts").toString();          entry.setTranscription(ts);}    // Transcription.
        if(o.contains("pos"))           {QString pos = o.value("pos").toString();       entry.setPartOfSpeech(pos);}    // Part of speech.
        if(o.contains("tr"))    // Translations.
        {
            QJsonArray tr = o.value("tr").toArray();
            for (QJsonValue val : tr)
            {
                Translation translation;
                QJsonObject const obj = val.toObject();
                //qDebug() << obj.keys();
                if(obj.contains("text"))    {QString txt = obj.value("text").toString();    translation.setText(txt);}          // Text of a translation.
                if(obj.contains("pos"))     {QString ps = obj.value("pos").toString();      translation.setPartOfSpeech(ps);}   // Part of speech.
                if(obj.contains("asp"))     {QString asp = obj.value("asp").toString();     translation.setAspect(asp);}        // The aspect of a verb.
                if(obj.contains("num"))     {QString num = obj.value("num").toString();     translation.setNounForm(num);}      // The form of a noun and its variations: plural, etc.
                if(obj.contains("gen"))     {QString gen = obj.value("gen").toString();     translation.setGender(gen);}         // Gender.
                if(obj.contains("syn"))     // Synonyms.
                {
                    QJsonArray syns = obj.value("syn").toArray();
                    for(const auto& syn : syns)
                    {
                        QString  syn_txt = syn.toObject().value("text").toString();
                        translation.addSynonym(syn_txt);
                    }
                }
                if(obj.contains("mean"))// Meanings.
                {
                    QJsonArray means = obj.value("mean").toArray();
                    for(const auto& mean : means)
                    {
                        QString mean_txt = mean.toObject().value("text").toString();
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
