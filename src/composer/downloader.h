#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QPixmap>

class Downloader : public QObject {
    Q_OBJECT

public:
    Downloader(QObject *parent = nullptr) : QObject(parent) {

    }

    void init() {
        nam = new QNetworkAccessManager(this);
        connect(nam, &QNetworkAccessManager::finished, this, &Downloader::downloadFinished);
    }

    void download(const QString &raw_url) {
        const QUrl url = QUrl(raw_url);
        QNetworkRequest request(url);
        nam->get(request);
    }

public slots:
    virtual void downloadFinished(QNetworkReply *reply) = 0;

private:
    QNetworkAccessManager *nam;
};
#endif // DOWNLOADER_H
