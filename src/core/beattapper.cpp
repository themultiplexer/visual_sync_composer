
#include "beattapper.h"

#include <algorithm>

BeatTapper::BeatTapper(QObject *parent)
    : QObject(parent)
{
    clock_.start();
}

void BeatTapper::tap()
{
    emit beatDetected();

    const qint64 now = clock_.elapsed();

    if (lastTapMs_ < 0) {
        lastTapMs_ = now;
        return;
    }

    const qint64 interval = now - lastTapMs_;

    // Ignore double taps and excessively long gaps.
    if (interval < 250)
        return;

    lastTapMs_ = now;

    // Accept 30–240 BPM.
    if (interval > 2000) {
        reset();
        lastTapMs_ = now;
        return;
    }

    intervals_.append(static_cast<double>(interval));

    if (intervals_.size() > maxIntervals_)
        intervals_.removeFirst();

    // Median rejects occasional timing outliers.
    QVector<double> sorted = intervals_;
    std::sort(sorted.begin(), sorted.end());

    const int n = sorted.size();
    double median = sorted[n / 2];

    if (n % 2 == 0)
        median = (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;

    const double measuredBpm = 60000.0 / median;

    if (bpm_ == 0.0)
        bpm_ = measuredBpm;
    else
        bpm_ = alpha_ * measuredBpm
             + (1.0 - alpha_) * bpm_;

    emit bpmChanged(bpm_);
}

void BeatTapper::reset()
{
    lastTapMs_ = -1;
    intervals_.clear();
    bpm_ = 0.0;

    emit bpmChanged(bpm_);
}
