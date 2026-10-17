
#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QVector>

class BeatTapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double bpm READ bpm NOTIFY bpmChanged)

public:
    explicit BeatTapper(QObject *parent = nullptr);

    double bpm() const { return bpm_; }

    Q_INVOKABLE void tap();
    Q_INVOKABLE void reset();

private:
    QElapsedTimer clock_;
    qint64 lastTapMs_ = -1;

    QVector<double> intervals_;
    double bpm_ = 0.0;

    static constexpr int maxIntervals_ = 5;
    static constexpr double alpha_ = 0.35;

signals:
    void bpmChanged(double bpm);
    void beatDetected();
};
