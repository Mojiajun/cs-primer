#include "timestamp.hpp"
#include <sys/time.h>

const int Timestamp::kResolution = 1000 * 1000;

Timestamp::Timestamp() { Update(); }
Timestamp::Timestamp(Timestamp::TimePoint usec) : epoch_usec_(usec) {}
Timestamp::Timestamp(const Timestamp& timestamp)
    : epoch_usec_(timestamp.epoch_usec_) {}
Timestamp& Timestamp::operator=(Timestamp timestamp) {
  Swap(*this, timestamp);
  return *this;
}
Timestamp& Timestamp::AddTimespan(const Timespan& timespan) {
  epoch_usec_ += timespan.span_usec_;
}
void Timestamp::Update() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  epoch_usec_ = static_cast<TimePoint>(tv.tv_sec * kResolution + tv.tv_usec);
}
Timespan Timestamp::Elapsed() const {
  Timestamp now;
  return now - *this;
}
bool Timestamp::IsElapsed(const Timespan timespan) const {
  Timestamp now;
  Timespan diff = now - *this;
  return diff >= timespan;
}

Timespan TimespanFromNow(Timestamp when) {
  Timestamp now;
  return when - now;
}