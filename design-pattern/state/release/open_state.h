#ifndef STATE_RELEASE_OPEN_STATE_H
#define STATE_RELEASE_OPEN_STATE_H

#include "network_state.h"

namespace release {
class OpenState : public NetworkState {
 public:
  static NetworkState* GetInstance();
  void Operation1() override;
  void Operation2() override;

 private:
  static NetworkState* instance_;
};
}  // namespace release

#endif