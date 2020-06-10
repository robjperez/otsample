#include <string>

class UIState {
public:
  bool isSessionConnected;
  bool isPublishing;

  UIState() : isSessionConnected(false) {};

  const std::string connectButtonText() {
    return this->isSessionConnected ? "Disconnect" : "Connect";
  }

  const std::string publishButtonText() {
    return this->isPublishing ? "Unpublish" : "Publish";
  }
};
