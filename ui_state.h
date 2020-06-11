#include <string>

class UIState {
public:
  bool isSessionConnected;
  bool isPublishing;
  bool showPublisherButtons;

  bool isSubscribing;
  bool showSubscriberButtons;

  UIState() : isSessionConnected(false) {};

  const std::string connectButtonText() {
    return this->isSessionConnected ? "Disconnect" : "Connect";
  }

  const std::string publishButtonText() {
    return this->isPublishing ? "Unpublish" : "Publish";
  }

  const std::string subscriberButtonText() {
    return this->isSubscribing ? "Unsubscribe" : "Subscribe";
  }
};
