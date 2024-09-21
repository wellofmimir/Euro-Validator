#include <QCoreApplication>
#include <QPair>
#include <QString>
#include <QMap>
#include <QSet>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QScopedPointer>
#include <QSettings>
#include <QUuid>
#include <QDebug>
#include <QDir>
#include <QPair>

#include <QtConcurrent/QtConcurrent>
#include <QFutureInterface>
#include <QFuture>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QtHttpServer>
#include <QHostAddress>

#include <QCryptographicHash>
#include <QRegularExpression>

#include <optional>
#include <algorithm>

static const QString REGEX_NEUE_SERIE {"[A-Z][A-Z][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]"};
static const QString REGEX_ALTE_SERIE {"[A-Z][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]"};

static const QMap<QChar, qint64> buchstabeToPosition
{
    {'A', 1},
    {'B', 2},
    {'C', 3},
    {'D', 4},
    {'E', 5},
    {'F', 6},
    {'G', 7},
    {'H', 8},
    {'I', 9},
    {'J', 10},
    {'K', 11},
    {'L', 12},
    {'M', 13},
    {'N', 14},
    {'O', 15},
    {'P', 16},
    {'Q', 17},
    {'R', 18},
    {'S', 19},
    {'T', 20},
    {'U', 21},
    {'V', 22},
    {'W', 23},
    {'X', 24},
    {'Y', 25},
    {'Z', 26}
};

static const QSet<QChar> buchstaben
{
    'A',
    'B',
    'C',
    'D',
    'E',
    'F',
    'G',
    'H',
    'I',
    'J',
    'K',
    'L',
    'M',
    'N',
    'O',
    'P',
    'Q',
    'R',
    'S',
    'T',
    'U',
    'V',
    'W',
    'X',
    'Y',
    'Z'
};

static const QMap<QString, QString> buchstabeTo9erRest
{
    {"Z", "9"},
    {"Y", "1"},
    {"X", "2"},
    {"W", "3"},
    {"V", "4"},
    {"U", "5"},
    {"T", "6"},
    {"S", "7"},
    {"R", "8"},
    {"P", "1"},
    {"N", "3"},
    {"M", "4"},
    {"L", "5"},
    {"K", "6"},
    {"J", "7"},
    {"H", "8"},
    {"G", "1"},
    {"F", "2"},
    {"E", "3"},
    {"D", "4"},
    {"C", "5"},
    {"B", "6"},
};

static const QMap<QString, QString> buchstabeToLand //alte Serie
{
    {"Z", "Belgium"},
    {"Y", "Greece"},
    {"X", "Germany"},
    {"W", "Denmark"},
    {"V", "Spain"},
    {"U", "France"},
    {"T", "Ireland"},
    {"S", "Italy"},
    {"R", "Luxembourg"},
    {"P", "Netherlands"},
    {"N", "Austria"},
    {"M", "Portugal"},
    {"L", "Finnland"},
    {"K", "Sweden"},
    {"J", "United Kingdom"},
    {"H", "Slovenia"},
    {"G", "Cyprus"},
    {"F", "Malta"},
    {"E", "Slovakia"},
    {"D", "Estonia"},
    {"C", "Latvia"},
    {"B", "Lithuania"},
};

static const QMap<QString, QString> buchstabenkomboTo9erRest
{
    {"EA", "1"},
    {"EB", "0"},
    {"EC", "8"},
    {"EM", "7"},
    {"FA", "0"},
    {"MA", "2"},
    {"MC", "0"},
    {"MD", "8"},
    {"NA", "1"},
    {"NB", "0"},
    {"NC", "8"},
    {"NZ", "3"},
    {"PA", "8"},
    {"PB", "7"},
    {"PC", "6"},
    {"RA", "6"},
    {"RB", "5"},
    {"RC", "4"},
    {"RD", "3"},
    {"RP", "0"},
    {"SA", "5"},
    {"SB", "4"},
    {"SC", "3"},
    {"SD", "2"},
    {"SE", "1"},
    {"SF", "0"},
    {"TA", "4"},
    {"TC", "2"},
    {"TD", "1"},
    {"UA", "3"},
    {"UB", "2"},
    {"UC", "1"},
    {"UD", "0"},
    {"UE", "8"},
    {"UF", "7"},
    {"VA", "2"},
    {"VB", "1"},
    {"VC", "0"},
    {"VH", "4"},
    {"WA", "1"},
    {"WB", "0"},
    {"XA", "0"},
    {"XZ", "2"},
    {"YA", "8"},
    {"ZA", "7"},
    {"ZB", "6"},
    {"ZC", "5"},
    {"ZD", "4"}
};

static const QMap<QChar, QString> laenderkodierungen
{
    {'J', "Great Britain"},
    {'K', "Sweden"},
    {'L', "Finnland"},
    {'M', "Portugal"},
    {'N', "Austria"},
    {'P', "Netherlands"},
    {'R', "Luxembourg"},
    {'S', "Italy"},
    {'T', "Ireland"},
    {'U', "France"},
    {'V', "Spain"},
    {'W', "Denmark"},
    {'X', "Germany"},
    {'Y', "Greece"},
    {'Z', "Belgium"}
};

static const QMap<QChar, QPair<QString, QString> > druckereien
{
    {'D', {QPair<QString, QString>{"Polska Wytwórnia Papierów Wartosciowych", "Poland"}}},
    {'E', {QPair<QString, QString>{"Francois Charles Oberthur Fiduciaire", "France"}}},
    {'F', {QPair<QString, QString>{"Oberthur Bulgarien", "Bulgaria"}}},
    {'H', {QPair<QString, QString>{"De La Rue Currency (Loughton)", "United Kingdom"}}},
    {'J', {QPair<QString, QString>{"De La Rue Currency (Gateshead)", "United Kingdom"}}},
    {'M', {QPair<QString, QString>{"Valora", "Portugal"}}},
    {'N', {QPair<QString, QString>{"Österreichische Banknoten- und Sicherheitsdruck GmbH", "Austria"}}},
    {'P', {QPair<QString, QString>{"Johan Enschede Security Printing BV", "Netherlands"}}},
    {'R', {QPair<QString, QString>{"Bundesdruckerei GmbH", "Germany"}}},
    {'S', {QPair<QString, QString>{"Banca d’Italia", "Italy"}}},
    {'T', {QPair<QString, QString>{"Central Bank of Ireland", "Ireland"}}},
    {'U', {QPair<QString, QString>{"Banque de France", "France"}}},
    {'V', {QPair<QString, QString>{"Fàbrica National de Mondea y Timbre", "Spain"}}},
    {'W', {QPair<QString, QString>{"Giesecke & Devrient (Leipzig)", "Germany"}}},
    {'X', {QPair<QString, QString>{"Giesecke & Devrient (München)", "Germany"}}},
    {'Y', {QPair<QString, QString>{"Bank of Greece", "Greece"}}},
    {'Z', {QPair<QString, QString>{"Banque Nationale de Belgique", "Belgium"}}}
};

bool isSerialnumberValid(const QString &serialnumber)
{
    if (serialnumber.size() != 12)
        return false;

    if (!QRegularExpression{REGEX_ALTE_SERIE}.match(serialnumber).hasMatch() && !QRegularExpression{REGEX_NEUE_SERIE}.match(serialnumber).hasMatch())
        return false;

    static std::function<QString(const QString &)> seriennummerBuchstabenUmwandeln = [](const QString &serialnumber) -> QString
    {
        QString newSerialnumber;

        for (const QChar &zeichen : serialnumber)
        {
            if (buchstaben.contains(zeichen))
            {
                newSerialnumber += QString::number(buchstabeToPosition.value(zeichen));
                continue;
            }

            newSerialnumber += zeichen;
        }

        return newSerialnumber;
    };

    static std::function<qint64(const QString &)> berechneQuersumme = [](const QString &serialnumber)
    {
        qint64 quersumme {0};
        bool ok {false};

        for (const QChar &digit : serialnumber)
            quersumme += QString{digit}.toInt(&ok);

        return quersumme;
    };

    std::function<bool(const QString &)> neueBerechnungsvorschrift = [&](const QString &serialnumber) -> bool
    {
        const bool pruefzifferKorrekt = [](const QString &serialnumber) -> bool
        {
            const QString umgewandelteSerialnumber {seriennummerBuchstabenUmwandeln(serialnumber)};
            const QString umgewandelteSerialnumberOhnePruefziffer {umgewandelteSerialnumber.left(umgewandelteSerialnumber.size() - 1)};

            const qint64 quersumme {berechneQuersumme(umgewandelteSerialnumberOhnePruefziffer)};
            const qint64 rest {quersumme % 9};
            const qint64 pruefziffer {7 - rest};

            if (pruefziffer == 0 && serialnumber.endsWith('9'))
                return true;

            if (pruefziffer == -1 && serialnumber.endsWith('8'))
                return true;

            if (serialnumber.endsWith(QString::number(pruefziffer)))
                return true;

            return false;

        }(serialnumber);

        if (!pruefzifferKorrekt)
            return false;

        const bool kontrollnummerKorrekt = [](const QString &serialnumber) -> bool
        {
            const QString neunerRest {buchstabenkomboTo9erRest.value(serialnumber.left(2))};
            const QString seriennummerOhneBuchstabenAmAnfang {serialnumber.mid(2, serialnumber.size())};

            const qint64 quersumme {berechneQuersumme(seriennummerOhneBuchstabenAmAnfang)};
            qint64 neueQuersumme {quersumme};

            while (neueQuersumme > 9)
                neueQuersumme = berechneQuersumme(QString::number(neueQuersumme));

            if (neueQuersumme == 9 && neunerRest.toInt() == 0)
                return true;

            if (neueQuersumme == neunerRest.toInt())
                return true;

            return false;

        }(serialnumber);

        if (pruefzifferKorrekt && kontrollnummerKorrekt)
            return true;

        return false;
    };

    std::function<bool(const QString &)> alteBerechnungsVorschrift = [](const QString &serialnumber)
    {
        const bool pruefzifferKorrekt = [](const QString &serialnumber) -> bool
        {
            const qint64 quersumme {0};
            const qint64 rest {quersumme % 9};
            const qint64 pruefziffer {8 - rest};

            if (pruefziffer == 0 && serialnumber.endsWith('9'))
                return true;

            if (serialnumber.endsWith(QString::number(pruefziffer)))
                return true;

            return false;

        }(serialnumber);

        if (!pruefzifferKorrekt)
            return false;

        const bool kontrollnummerKorrekt = [](const QString &serialnumber) -> bool
        {
            const QString neunerRest {buchstabeTo9erRest.value(serialnumber.first(1))};

            const QString seriennummerOhneBuchstabenAmAnfang {serialnumber.mid(1, serialnumber.size())};

            const qint64 quersumme {berechneQuersumme(seriennummerOhneBuchstabenAmAnfang)};
            qint64 neueQuersumme {quersumme};

            while (neueQuersumme > 9)
                neueQuersumme = berechneQuersumme(QString::number(neueQuersumme));

            if (neueQuersumme == 9 && neunerRest.toInt() == 0)
                return true;

            if (neueQuersumme == neunerRest.toInt())
                return true;

            return false;

        }(serialnumber);

        if (pruefzifferKorrekt && kontrollnummerKorrekt)
            return true;

        return false;
    };

    return neueBerechnungsvorschrift(serialnumber) || alteBerechnungsVorschrift(serialnumber);
}

static const QString rapidApiKey {"1e21c3855a6b04f468cbb555999fcbfaf3dc31581b2b2ad0d0d407349157462ae49dde8b45dfe7163ae2ea0311697b9335484e2cf073441babc8171132e0861c"};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication app {argc, argv};

    const quint16 PORT {50002};
    const QScopedPointer<QHttpServer> httpServer {new QHttpServer {&app}};

    httpServer->route("/ping", QHttpServerRequest::Method::Get,
    [](const QHttpServerRequest &request) -> QFuture<QHttpServerResponse>
    {
        qDebug() << "Ping verarbeitet";

#ifdef QT_DEBUG
        Q_UNUSED(request)
#else
        const bool requestIsFromRapidAPI = [](const QHttpServerRequest &request) -> bool
        {
            for (const QPair<QByteArray, QByteArray> &header : request.headers())
                if (header.first == "X-RapidAPI-Proxy-Secret" && QCryptographicHash::hash(header.second, QCryptographicHash::Sha512).toHex() == rapidApiKey)
                    return true;

            return false;

        }(request);

        if (!requestIsFromRapidAPI)
            return QtConcurrent::run([]()
            {
                return QHttpServerResponse
                {
                    QJsonObject
                    {
                        {"Message", "HTTP-Requests allowed only via RapidAPI-Gateway."}
                    }
                };
            });
#endif
        return QtConcurrent::run([]()
        {
            return QHttpServerResponse
            {
                QJsonObject
                {
                    {"Message", "pong"}
                }
            };
        });
    });

    httpServer->route("/validate", QHttpServerRequest::Method::Get    |
                                   QHttpServerRequest::Method::Put     |
                                   QHttpServerRequest::Method::Head    |
                                   QHttpServerRequest::Method::Trace   |
                                   QHttpServerRequest::Method::Patch   |
                                   QHttpServerRequest::Method::Delete  |
                                   QHttpServerRequest::Method::Options |
                                   QHttpServerRequest::Method::Connect |
                                   QHttpServerRequest::Method::Unknown,
    [](const QHttpServerRequest &request) -> QFuture<QHttpServerResponse>
    {
#ifdef QT_DEBUG
        Q_UNUSED(request)
#else
       const bool requestIsFromRapidAPI = [](const QHttpServerRequest &request) -> bool
       {
           for (const QPair<QByteArray, QByteArray> &header : request.headers())
           {
               if (header.first == "X-RapidAPI-Proxy-Secret" && QCryptographicHash::hash(header.second, QCryptographicHash::Sha512).toHex() == rapidApiKey)
                   return true;
           }

           return false;

       }(request);

       if (!requestIsFromRapidAPI)
           return QtConcurrent::run([]()
           {
               return QHttpServerResponse
               {
                   QJsonObject
                   {
                       {"Message", "HTTP-Requests allowed only via RapidAPI-Gateway."}
                   }
               };
           });
#endif
       return QtConcurrent::run([]()
       {
           return QHttpServerResponse
           {
               QJsonObject
               {
                   {"Message", "The used HTTP-Method is not implemented."}
               }
           };
       });
    });

    httpServer->route("/validate", QHttpServerRequest::Method::Post,
    [](const QHttpServerRequest &request) -> QFuture<QHttpServerResponse>
    {
        qDebug() << "Anfrage von IP: " << request.remoteAddress().toString();

#ifdef QT_DEBUG
        Q_UNUSED(request)
#else
        const bool requestIsFromRapidAPI = [](const QHttpServerRequest &request) -> bool
        {
            for (const QPair<QByteArray, QByteArray> &header : request.headers())
            {
                if (header.first == "X-RapidAPI-Proxy-Secret" && QCryptographicHash::hash(header.second, QCryptographicHash::Sha512).toHex() == rapidApiKey)
                    return true;
            }

            return false;

        }(request);

        if (!requestIsFromRapidAPI)
            return QtConcurrent::run([]()
            {
                return QHttpServerResponse
                {
                    QJsonObject
                    {
                        {"Message", "HTTP-Requests allowed only via RapidAPI-Gateway."}
                    }
                };
            });
#endif

        if (request.body().isEmpty())
            return QtConcurrent::run([]()
            {
                return QHttpServerResponse
                {
                    QJsonObject
                    {
                        {"Message", "HTTP-Request body is empty."}
                    }
                };
            });

        const QJsonDocument jsonDocument {QJsonDocument::fromJson(request.body())};

        if (jsonDocument.isNull())
            return QtConcurrent::run([]()
            {
                return QHttpServerResponse
                {
                    QJsonObject
                    {
                        {"Message", "Invalid data sent. Please send a valid JSON-Object."}
                    }
                };
            });

        const QJsonObject jsonObject {jsonDocument.object()};

        if (jsonObject.isEmpty())
            return QtConcurrent::run([]()
            {
                return QHttpServerResponse
                {
                    QJsonObject
                    {
                        {"Message", "Invalid data sent. Please send a valid JSON-Object."}
                    }
                };
            });


        if (!jsonObject.contains("Serialnumber"))
             return QtConcurrent::run([]()
             {
                 return QHttpServerResponse
                 {
                     QJsonObject
                     {
                         {"Message", "Invalid data sent. Serialnumber-key is missing."}
                     }
                 };
             });

        if (jsonObject.value("Serialnumber").toString().isEmpty())
             return QtConcurrent::run([]()
             {
                 return QHttpServerResponse
                 {
                     QJsonObject
                     {
                         {"Message", "Invalid data sent. Serialnumber-value is missing."}
                     }
                 };
             });

        if (jsonObject.value("Serialnumber").toString().size() != 12)
            return QtConcurrent::run([=]()
            {
                return QHttpServerResponse
                {
                    QJsonObject
                    {
                        {"Valid",   false},
                        {"Message", "The submitted value is not a valid serialnumber of an EUR banknote."}
                    }
                };
            });

        if (QRegularExpression{REGEX_NEUE_SERIE}.match(jsonObject.value("Serialnumber").toString()).hasMatch())
            return QtConcurrent::run([=]()
            {
                return QHttpServerResponse
                {
                    QJsonObject
                    {
                        {"Valid", isSerialnumberValid(jsonObject.value("Serialnumber").toString())},
                        {"Issued by", druckereien.value(jsonObject.value("Serialnumber").toString()[0]).first},
                        {"Issuer located in", druckereien.value(jsonObject.value("Serialnumber").toString()[0]).second}
                    }
                };
            });

        if (QRegularExpression{REGEX_ALTE_SERIE}.match(jsonObject.value("Serialnumber").toString()).hasMatch())
             return QtConcurrent::run([=]()
             {
                 return QHttpServerResponse
                 {
                     QJsonObject
                     {
                         {"Valid", isSerialnumberValid(jsonObject.value("Serialnumber").toString())},
                         {"Issued by", buchstabeToLand.value(jsonObject.value("Serialnumber").toString()[0])},
                     }
                 };
             });

        return QtConcurrent::run([=]()
        {
            return QHttpServerResponse
            {
                QJsonObject
                {
                    {"Valid",   false},
                    {"Message", "The submitted value is not a valid serialnumber of an EUR banknote."}
                }
            };
        });
    });

    if (httpServer->listen(QHostAddress::Any, static_cast<quint16>(PORT)) == 0)
        return -1;

    return a.exec();
}
