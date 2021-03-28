#ifndef SEEDER_HPP_INCLUDED
#define SEEDER_HPP_INCLUDED
#ifdef DECENTRALIZED_UPDATE_ENABLED
#include <QObject>
#include <QJsonObject>
#include <QSharedPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class SeederPrivate;

class Seeder : public QObject {
    Q_OBJECT
    QSharedPointer<SeederPrivate> m_Private;
  public:
    Seeder(QNetworkAccessManager*, QObject *parent = nullptr);
  public Q_SLOTS:
    void start(QJsonObject);
    void cancel();
  Q_SIGNALS:
    void started();
    void canceled();
    void error(short);

    void logger(QString);
    void torrentStatus(int,int);
};
#endif // DECENTRALIZED_UPDATE_ENABLED
#endif // SEEDER_HPP_INCLUDED
