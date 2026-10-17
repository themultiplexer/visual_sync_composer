
#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

class BeatClock : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double bpm READ bpm WRITE setBpm NOTIFY bpmChanged)

public:
    explicit BeatClock(QObject *parent = nullptr);

    double bpm() const;
    void setBpm(double bpm);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

signals:
    void beatDetected();
    void bpmChanged();

private slots:
    void update();

private:
    QTimer timer_;
    QElapsedTimer elapsed_;

    double bpm_ = 120.0;
    double intervalMs_ = 500.0;
    double nextBeatMs_ = 500.0;
};
