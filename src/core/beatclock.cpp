
#include "beatclock.h"

BeatClock::BeatClock(QObject *parent)
    : QObject(parent)
{
    timer_.setTimerType(Qt::PreciseTimer);
    timer_.setInterval(1);

    connect(&timer_, &QTimer::timeout,
            this, &BeatClock::update);
}

double BeatClock::bpm() const
{
    return bpm_;
}

void BeatClock::setBpm(double bpm)
{
    if (bpm <= 0.0 || bpm_ == bpm)
        return;

    bpm_ = bpm;
    intervalMs_ = 60000.0 / bpm_;

    // Restart the beat phase when BPM changes.
    if (elapsed_.isValid()) {
        elapsed_.restart();
    }

    nextBeatMs_ = intervalMs_;

    emit bpmChanged();
}

void BeatClock::start()
{
    elapsed_.start();
    nextBeatMs_ = intervalMs_;
    timer_.start();
}

void BeatClock::stop()
{
    timer_.stop();
}

void BeatClock::update()
{
    if (!elapsed_.isValid())
        return;

    const qint64 now = elapsed_.elapsed();

    if (now >= nextBeatMs_) {
        emit beatDetected();

        // Advance from the scheduled time to avoid drift.
        do {
            nextBeatMs_ += intervalMs_;
        } while (nextBeatMs_ <= now);
    }
}
