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
  check(j >= 0);
  check(j < n);
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

enum Button toSingleDirection(std::optional<enum Button> dx, std::optional<enum Button> dy) {
  if (dx.has_value()) {
    if (dx.value() == Button::FORWARD) {
      if (dy.has_value() && (dy.value() == Button::DOWN))
        return Button::DOWNFORWARD;
      else if (dy.has_value() && (dy.value() == Button::UP))
        return Button::UPFORWARD;
      else
        return Button::FORWARD;
    }
    else if (dx.value() == Button::BACK) {
      if (dy.has_value() && (dy.value() == Button::DOWN))
        return Button::DOWNBACK;
      if (dy.has_value() && (dy.value() == Button::UP))
        return Button::UPBACK;
      else
        return Button::BACK;
    }
  }

  if (dy.has_value())
    return dy.value();
  else
    return Button::NEUTRAL;
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
  case Button::UPBACK: return TEXT("UPBACK"); break;
  case Button::UPFORWARD: return TEXT("UPFORWARD"); break;
  case Button::DOWNBACK: return TEXT("DOWNBACK"); break;
  case Button::DOWNFORWARD: return TEXT("DOWNFORWARD"); break;
  case Button::NEUTRAL: return TEXT("NEUTRAL"); break;
  case Button::QCFP: return TEXT("QCFP"); break;
  default: return TEXT("0");
  }
}

FString AFightInput::encodedButtonsToString(int8 e) {
  FString r;
  for (auto b : {Button::LP, Button::HP, Button::LK, Button::HK,
                 Button::UP, Button::DOWN, Button::LEFT, Button::RIGHT}) {
    if (decodeButton(b, e)) {
      r.Append(buttonToString(b));
      r.Append(FString(" "));
    }
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
  // MYLOG(Display,
  //       TEXT("buttons(): (current frame %i) (target frame %i) (buttonsPressed %s) (buttonsReleased %s)"),
  //       currentFrame,
  //       targetFrame,
  //       *encodedButtonsToString(buttonsPressed),
  //       *encodedButtonsToString(buttonsReleased));

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

std::optional<enum Button> AFightInput::translateDirection(std::optional<enum Button>& d, bool isOnLeft) {
  if (d.has_value())
    return std::make_optional(translateDirection(d.value(), isOnLeft));
  else
    return {};
}

int AFightInput::computeIndex(int targetFrame) {
  return (currentFrame - targetFrame)+delay;
}

HAction AFightInput::_action(HAction currentAction, int frame, bool isOnLeft, int actionFrame) {
  const HCharacter& c = currentAction.character();
  // MYLOG(Warning, "_action(): %s (frame %i) (button %s)", *GetActorLabel(false), frame, directionHistory.nthlast(frame).has_value() ? ((directionHistory.nthlast(frame).value() == Button::RIGHT) ? TEXT("right") : TEXT("not right")) : TEXT("none"));

  // first we will determine the "button"
  enum Button newButton = Button::NEUTRAL;

  // first try motion commands; they have the highest priority
  if (buttonHistory.nthlast(frame).has_value()) {
    for (auto i = motionCommands.begin(); i != motionCommands.end(); ++i) {
      for (auto j = i->second.begin(); j != i->second.end(); ++j) {
        if (j->back() == buttonHistory.nthlast(frame).value()) {
          if (checkMotionCommand(*j, 1, frame, isOnLeft)) {
            MYLOG(Display, "_action(): %s!", buttonToString(i->first));
            newButton = i->first;
          }
        }
      }
    }
  }

  // try a normal attack
  if (newButton == Button::NEUTRAL) {
    if (buttonHistory.nthlast(frame).has_value()) {
      newButton = buttonHistory.nthlast(frame).value();
    }
  }

  // try directional input
  if (newButton == Button::NEUTRAL)
    newButton = toSingleDirection(translateDirection(directionHistoryX.nthlast(frame), isOnLeft), directionHistoryY.nthlast(frame));

  // now with our "button" we pick an action

  // first try chains; these have highest priority
  if (actionFrame >= currentAction.specialCancelFrames()) {
    auto i = currentAction.chains().find(newButton);
    if (i != currentAction.chains().end()) return i->second;
  }

  if (actionFrame < currentAction.lockedFrames())
    return currentAction.character().idle(); // idle doesn't interrupt
                                             // any actions so this is
                                             // safe as a "do nothing"
                                             // return value

  switch (newButton) {
  case Button::NEUTRAL:
  case Button::DOWNFORWARD:
  case Button::DOWNBACK:
    return c.idle();
  case Button::FORWARD:
    return c.walkForward();
  case Button::BACK:
  case Button::UPBACK:
    return c.walkBackward();
  case Button::UPFORWARD:
    return c.fJump();
  case Button::HP:
    return c.sthp();
  }

  // just idle if no other action was chosen
  return c.idle();
}

bool AFightInput::checkMotionCommand(std::vector<enum Button>& motion, int m, int frame, bool isOnLeft) {
  if (m == motion.size())
    return true;
  if (frame >= n)
    return false;

  check(frame >= 0);
  check((motion.size() - m - 1) < motion.size());
  // MYLOG(Display,
  //       "checkMotionCommand() %i %i %s %s %s",
  //       m,
  //       frame,
  //       buttonToString(toSingleDirection(translateDirection(directionHistoryX.nthlast(frame), isOnLeft), directionHistoryY.nthlast(frame))),
  //       directionHistoryX.nthlast(frame).has_value() ? buttonToString(translateDirection(directionHistoryX.nthlast(frame).value(), isOnLeft)) : TEXT("None"),
  //       directionHistoryY.nthlast(frame).has_value() ? buttonToString(directionHistoryY.nthlast(frame).value()) : TEXT("None"));
  for (int i = 0; i < 4; ++i) { // this 4 is the number of frames we allow between inputs
    if (toSingleDirection(translateDirection(directionHistoryX.nthlast(frame+i), isOnLeft), directionHistoryY.nthlast(frame+i)) == motion[motion.size() - m - 1]) {
      if (checkMotionCommand(motion, m+1, frame+i+1, isOnLeft))
        return true;
    }
  }

  return false;
}

HAction AFightInput::action(HAction currentAction, bool isOnLeft, int targetFrame, int actionStart) {
  int frameBefore = currentFrame;
  int actionFrame = targetFrame - actionStart;
  ensureFrame(targetFrame);
  //MYLOG(Warning, "action(): %s (current frame %i) (target frame %i)", *GetActorLabel(false), currentFrame, targetFrame);

  int frame = computeIndex(targetFrame);
  // Try decoding an action based on the inputs at `frame'. If the
  // action we decode is walking or idling, then try using the inputs
  // one frame earlier. Repeat until we find an action that isn't
  // idling or walking, or we have tried all of the `input
  // buffer' frames.
  HAction mostRecentAction = _action(currentAction, frame, isOnLeft, actionFrame);
  // MYLOG(Warning, "action(): %s (current frame %i) (target frame %i) (action: %s)", *GetActorLabel(false), currentFrame, targetFrame, (mostRecentAction == HActionIdle) ? TEXT("idle") : TEXT("not idle"));
  HAction action = mostRecentAction;

  while (action.isWalkOrIdle() && (++frame < (delay+buffer))) {
    action = _action(currentAction, frame, isOnLeft, actionFrame);
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
