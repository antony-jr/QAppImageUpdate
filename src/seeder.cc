#ifdef DECENTRALIZED_UPDATE_ENABLED
#include "seeder.hpp"
#include "seeder_p.hpp"
#include "helpers_p.hpp"

Seeder::Seeder(QNetworkAccessManager *manager, QObject *parent)
    : QObject(parent) {
    m_Private = QSharedPointer<SeederPrivate>(new SeederPrivate(manager));
    auto obj = m_Private.data();

    connect(obj, &SeederPrivate::started,
            this, &Seeder::started,
            Qt::DirectConnection);

    connect(obj, &SeederPrivate::canceled,
            this, &Seeder::canceled,
            Qt::DirectConnection);

    connect(obj, &SeederPrivate::error,
            this, &Seeder::error,
            Qt::DirectConnection);

    connect(obj, &SeederPrivate::logger,
            this, &Seeder::logger,
            Qt::DirectConnection);

    connect(obj, &SeederPrivate::torrentStatus,
            this, &Seeder::torrentStatus,
            Qt::DirectConnection);
}

void Seeder::start(QJsonObject info) {
    getMethod(m_Private.data(), "start(QJsonObject)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection,
            Q_ARG(QJsonObject,info));

}

void Seeder::cancel() {
    getMethod(m_Private.data(), "cancel(void)")
    .invoke(m_Private.data(),
            Qt::QueuedConnection);
}
#endif // DECENTRALIZED_UPDATE_ENABLED
