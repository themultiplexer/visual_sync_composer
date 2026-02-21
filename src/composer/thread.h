#ifndef THREAD_H
#define THREAD_H

#include "downloader.h"
#include <QThread>

class WorkerThread : public QThread {
    Q_OBJECT

signals:
    void startDownload(const QString &raw_url);

public slots:
    void onThreadStarted() {
        d = new Downloader(this);
        d->init();
        connect(this, &WorkerThread::startDownload, d, &Downloader::download);
    }

public:
    void run() override;
    Downloader *d;
};

#endif // THREAD_H
