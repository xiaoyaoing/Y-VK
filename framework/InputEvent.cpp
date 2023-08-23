
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
