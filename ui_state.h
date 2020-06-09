#include <string>

class UIState {
public:
  bool isSessionConnected;

  UIState() : isSessionConnected(false) {};

  const std::string connectButtonText() {
    return this->isSessionConnected ? "Disconnect" : "Connect";
  }
};
