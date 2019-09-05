#pragma once
#include <stdint.h>
#include <time.h>
#include <utility>
#include "copyalbe_tag.hpp"

class Timestamp;
class Timespan : private Copyable {
 public:
  typedef int64_t TimeDiff;
  static const int kMillseconds;
  static const int kSeconds;

 public:
  Timespan();
  explicit Timespan(TimeDiff usec);
  Timespan(TimeDiff sec, TimeDiff usec);
  Timespan(const Timespan& timespan);

  TimeDiff ToSeconds() const {
    return static_cast<TimeDiff>(span_usec_ / kSeconds);
  };
  TimeDiff ToMilliseconds() const {
    return static_cast<TimeDiff>(span_usec_ / kMillseconds);
  }
  TimeDiff ToMicroseconds() const { return span_usec_; }
  struct timespec ToTimespec() const;
  struct timeval ToTimeval() const;

  Timespan& operator=(Timespan timespan) {
    Swap(*this, timespan);
    return *this;
  }
  Timespan& operator+=(const Timespan& timespan) {
    span_usec_ += timespan.span_usec_;
    return *this;
  }
  Timespan& operator-=(const Timespan& timespan) {
    span_usec_ -= timespan.span_usec_;
    return *this;
  }

 public:
  friend Timestamp;

  friend Timespan operator+(const Timespan& lhs, const Timespan& rhs);
  friend Timespan operator-(const Timespan& lhs, const Timespan& rhs);
  friend bool operator<(Timespan lhs, Timespan rhs);
  friend bool operator<=(Timespan lhs, Timespan rhs);
  friend bool operator>(Timespan lhs, Timespan rhs);
  friend bool operator>=(Timespan lhs, Timespan rhs);
  friend bool operator==(Timespan lhs, Timespan rhs);
  friend bool operator!=(Timespan lhs, Timespan rhs);
  friend void Swap(Timespan& lhs, Timespan& rhs);

 private:
  TimeDiff span_usec_;
};

inline void Swap(Timespan& lhs, Timespan& rhs) {
  std::swap(lhs.span_usec_, rhs.span_usec_);
}
inline Timespan operator+(const Timespan& lhs, const Timespan& rhs) {
  return Timespan(lhs.span_usec_ + rhs.span_usec_);
}
inline Timespan operator-(const Timespan& lhs, const Timespan& rhs) {
  return Timespan(lhs.span_usec_ - rhs.span_usec_);
}
inline bool operator<(Timespan lhs, Timespan rhs) {
  return lhs.span_usec_ < rhs.span_usec_;
}
inline bool operator<=(Timespan lhs, Timespan rhs) {
  return lhs.span_usec_ <= rhs.span_usec_;
}
inline bool operator>(Timespan lhs, Timespan rhs) { return !(lhs <= rhs); }
inline bool operator>=(Timespan lhs, Timespan rhs) { return !(lhs < rhs); }
inline bool operator==(Timespan lhs, Timespan rhs) {
  return lhs.span_usec_ == rhs.span_usec_;
}
inline bool operator!=(Timespan lhs, Timespan rhs) { return !(lhs == rhs); }