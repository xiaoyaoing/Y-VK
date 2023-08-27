
#include "InputEvent.h"

KeyInputEvent::KeyInputEvent(KeyAction action, KeyCode code) : InputEvent(EventSource::KeyBoard), action(action),
                                                               code(code) {}

KeyAction KeyInputEvent::getAction() const {
    return action;
}

KeyCode KeyInputEvent::getCode() const {
    return code;
}

EventSource InputEvent::getSource() const {
    return source;
}

InputEvent::InputEvent(EventSource source) : source(source) {

}

MouseButton MouseButtonInputEvent::getButton() const {
    return button;
}


MouseAction MouseButtonInputEvent::getAction() const {
    return action;
}


float MouseButtonInputEvent::getPosX() const {
    return posX;
}


float MouseButtonInputEvent::getPosY() const {
    return posY;
}

MouseButtonInputEvent::MouseButtonInputEvent(MouseButton button, MouseAction action, float posX,
                                             float posY) : InputEvent(EventSource::Mouse), button(button),
                                                           action(action),
                                                           posX(posX), posY(posY) {}


TouchAction TouchInputEvent::getAction() const {
    return action;
}

int TouchInputEvent::getPointerId() const {
    return pointerId;
}

const size_t &TouchInputEvent::getTouchPoints() const {
    return touchPoints;
}

float TouchInputEvent::getPosX() const {
    return posX;
}

float TouchInputEvent::getPosY() const {
    return posY;
}
