// Fill out your copyright notice in the Description page of Project Settings.

#include "FightInput.h"
#include <algorithm>

#define MYLOG(category, message, ...) UE_LOG(LogTemp, category, TEXT("AFightInput (%s %s) " message), *GetActorLabel(false), (GetWorld()->IsNetMode(NM_ListenServer)) ? TEXT("server") : TEXT("client"), ##__VA_ARGS__)

void ButtonRingBuffer::reserve(int size) {
  n = size;
  clear();
}

void ButtonRingBuffer::clear() {
  v.clear();
  v.resize(n, {});
  end = 0;
}

void ButtonRingBuffer::push(const std::optional<enum Button>& x) {
  end = end+1;
  if (end == n) end = 0;
  v.at(end) = x;
}

std::optional<enum Button>& ButtonRingBuffer::last() {
  return v.at(end);
}

std::optional<enum Button>& ButtonRingBuffer::nthlast(int i) {
  int j = end-i;
  if (j < 0) j += n;
  return v.at(j);
}

AFightInput::AFightInput() {
  bReplicates = true;
}

bool AFightInput::is_button(const enum Button& b) {
  switch (b) {
  case Button::LP:
  case Button::HP:
  case Button::LK:
  case Button::HK:
    return true;
  default:
    return false;
  }
}

// bool AFightInput::is_none(const Button& b) {
//   return b == Button::NONE;
// }

void AFightInput::init(int _maxRollback, int _buffer, int _delay) {
  maxRollback = _maxRollback;
  buffer = _buffer;
  delay = _delay;
  n = maxRollback+buffer+delay+1;
  buttonHistory.reserve(n);
  directionHistoryX.reserve(n);
  directionHistoryY.reserve(n);
  mode = LogicMode::Wait;
  currentFrame = 0;
  reset();
}

void AFightInput::reset() {
  needsRollbackToFrame = -1;
  buttonHistory.clear();
  directionHistoryX.clear();
  directionHistoryY.clear();
}

void AFightInput::setMode(enum LogicMode m) {
  mode = m;
}

void AFightInput::ensureFrame(int targetFrame) {
  while (targetFrame > currentFrame) {
    // assume nothing was pressed for frames we are skipping here. The
    // last one we push is actually for the new frame and we will
    // store the recieved inputs in that one.
    buttonHistory.push({});
    directionHistoryX.push(directionHistoryX.last());
    directionHistoryY.push(directionHistoryY.last());
    ++currentFrame;
  }
}

auto buttonToString(enum Button b) {
  switch (b) {
  case Button::LP: return TEXT("LP"); break;
  case Button::HP: return TEXT("HP"); break;
  case Button::LK: return TEXT("LK"); break;
  case Button::HK: return TEXT("HK"); break;
  case Button::FORWARD: return TEXT("FORWARD"); break;
  case Button::BACK: return TEXT("BACK"); break;
  case Button::UP: return TEXT("UP"); break;
  case Button::DOWN: return TEXT("DOWN"); break;
  case Button::LEFT: return TEXT("LEFT"); break;
  case Button::RIGHT: return TEXT("RIGHT"); break;
  default: return TEXT("0");
  }
}

FString AFightInput::encodedButtonsToString(int8 e) {
  FString r;
  for (auto b : {Button::LP, Button::HP, Button::LK, Button::HK,
                 Button::UP, Button::DOWN, Button::LEFT, Button::RIGHT}) {
    if (decodeButton(b, e))
      r.Append(buttonToString(b));
  }
  return r;
}

int8 AFightInput::encodeButton(enum Button b, int8 encoded) {
  return encoded | (1 << (int) b);
}

int8 AFightInput::unsetButton(enum Button b, int8 encoded) {
  return encoded & (~(1 << (int) b));
}

bool AFightInput::decodeButton(enum Button b, int8 encoded) {
  return (encoded & (1 << (int) b)) != 0;
}

void AFightInput::buttons(int8 buttonsPressed, int8 buttonsReleased, int targetFrame) {
  if (mode != LogicMode::Fight) return;
  MYLOG(Display,
        TEXT("buttons(): (current frame %i) (target frame %i) (buttonsPressed %s) (buttonsReleased %s)"),
        currentFrame,
        targetFrame,
        *encodedButtonsToString(buttonsPressed),
        *encodedButtonsToString(buttonsReleased));

  // check if a rollback will be needed
  if (targetFrame <= (currentFrame-delay)) {
    if (getNeedsRollbackToFrame()) {
      needsRollbackToFrame = std::min(needsRollbackToFrame, targetFrame-delay);
    }
    else {
      needsRollbackToFrame = targetFrame-delay;
    }
    if (targetFrame <= currentFrame+1 - delay - maxRollback) {
      return; // there is nothing that this class can do in this
              // situation. We don't have input data going back that
              // far. Let ALogic decide how to reset or quit the
              // match.
    }
  }

  ensureFrame(targetFrame);

  // get the data for the frame we want to modify
  int i = currentFrame - targetFrame;
  std::optional<enum Button>& bh = buttonHistory.nthlast(i);
  std::optional<enum Button>& dxh = directionHistoryX.nthlast(i);
  std::optional<enum Button>& dyh = directionHistoryY.nthlast(i);

  // handle presses. this is a really simple implementation that just
  // sets the button pressed to the last button/direction that happens
  // to appear in buttonsPressed.
  for (auto b: {Button::LP, Button::HP, Button::LK, Button::HK}) {
    if (decodeButton(b, buttonsPressed))
        bh = std::make_optional(b);
  }
  for (auto b: {Button::LEFT, Button::RIGHT}) {
    if (decodeButton(b, buttonsPressed))
      dxh = std::make_optional(b);
  }
  for (auto b: {Button::UP, Button::DOWN}) {
    if (decodeButton(b, buttonsPressed))
      dyh = std::make_optional(b);
  }

  // handle releases. this should probably just ignore everything
  // except directional inputs. we don't have to care when a button is
  // released, except when its a directional input. TODO: keep track
  // of all directions held so that when one is released we can use
  // one of the other currently held ones. _action() will have to pick
  // between which directions to prioritize.
  for (auto b: {Button::LEFT, Button::RIGHT}) {
    if (decodeButton(b, buttonsReleased) && (directionHistoryX.last() == b))
      dxh = {};
  }
  for (auto b: {Button::UP, Button::DOWN}) {
    if (decodeButton(b, buttonsReleased) && (directionHistoryY.last() == b))
      dyh = {};
  }
}

void AFightInput::ClientButtons_Implementation(int8 buttonsPressed, int8 buttonsReleased, int targetFrame) {
  MYLOG(Display, "ClientButtons");
  buttons(buttonsPressed, buttonsReleased, targetFrame);
}

enum Button AFightInput::translateDirection(const enum Button& d, bool isOnLeft) {
  if (((d == Button::RIGHT) && isOnLeft) ||
      ((d == Button::LEFT) && !isOnLeft))
    return Button::FORWARD;
  else if (((d == Button::LEFT) && isOnLeft) ||
           ((d == Button::RIGHT) && !isOnLeft))
    return Button::BACK;
  else
    return d;
}

int AFightInput::computeIndex(int targetFrame) {
  return (currentFrame - targetFrame)+delay;
}

HAction AFightInput::_action(HAction currentAction, int frame, bool isOnLeft) {
  const HCharacter& c = currentAction.character();
  // for now, if there was a button, output the corresponding attack.
  // If no button, then walk/idle based on directional input.
  // MYLOG(Warning, "_action(): %s (frame %i) (button %s)", *GetActorLabel(false), frame, directionHistory.nthlast(frame).has_value() ? ((directionHistory.nthlast(frame).value() == Button::RIGHT) ? TEXT("right") : TEXT("not right")) : TEXT("none"));
  if (buttonHistory.nthlast(frame).has_value()) {
    enum Button button = buttonHistory.nthlast(frame).value();
    if (button == Button::HP){
      MYLOG(Display, "_action(): Do StHP");
      return c.sthp();
    }
  }
  if (!directionHistoryY.nthlast(frame).has_value() || directionHistoryY.nthlast(frame).value() != Button::DOWN) {
    // player is not holding down
    if (directionHistoryX.nthlast(frame).has_value()) {
      enum Button button = translateDirection(directionHistoryX.nthlast(frame).value(), isOnLeft);
      if (button == Button::FORWARD)
        return c.walkForward();
      if (button == Button::BACK)
        return c.walkBackward();
    }
  }
  else {
    // player is holding down. crouch?
  }
  return c.idle();
}

HAction AFightInput::action(HAction currentAction, bool isOnLeft, int targetFrame) {
  int frameBefore = currentFrame;
  ensureFrame(targetFrame);
  //MYLOG(Warning, "action(): %s (current frame %i) (target frame %i)", *GetActorLabel(false), currentFrame, targetFrame);

  int frame = computeIndex(targetFrame);
  // Try decoding an action based on the inputs at `frame'. If the
  // action we decode is walking or idling, then try using the inputs
  // one frame earlier. Repeat until we find an action that isn't
  // idling or walking, or we have tried all of the `input
  // buffer' frames.
  HAction mostRecentAction = _action(currentAction, frame, isOnLeft);
  // MYLOG(Warning, "action(): %s (current frame %i) (target frame %i) (action: %s)", *GetActorLabel(false), currentFrame, targetFrame, (mostRecentAction == HActionIdle) ? TEXT("idle") : TEXT("not idle"));
  HAction action = mostRecentAction;
  while (action.isWalkOrIdle() && (++frame <= (delay+buffer))) {
    action = _action(currentAction, frame, isOnLeft);
  }
  return action.isWalkOrIdle() ? mostRecentAction : action;
}

bool AFightInput::isGuarding(bool isOnLeft, int targetFrame) {
  int frame = computeIndex(targetFrame);
  return
    directionHistoryX.nthlast(frame).has_value() &&
    (translateDirection(directionHistoryX.nthlast(frame).value(), isOnLeft) == Button::BACK);
}

int AFightInput::getCurrentFrame() {
  return currentFrame;
}

bool AFightInput::needsRollback() {
  return needsRollbackToFrame != -1;
}

int AFightInput::getNeedsRollbackToFrame() {
  return needsRollbackToFrame;
}

void AFightInput::clearRollbackFlags() {
  needsRollbackToFrame = -1;
}
