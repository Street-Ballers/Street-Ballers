// Fill out your copyright notice in the Description page of Project Settings.

#include "FightInput.h"
#include <algorithm>
#include <limits>

#define MYLOG(category, message, ...) UE_LOG(LogTemp, category, TEXT("AFightInput (%s %s) " message), *GetActorLabel(false), (GetWorld()->IsNetMode(NM_ListenServer)) ? TEXT("server") : TEXT("client"), ##__VA_ARGS__)

#define LATENCY_HISTORY_SIZE 10

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

void intRingBuffer::reserve(int size) {
  n = size;
  clear();
}

void intRingBuffer::clear() {
  v.clear();
  v.resize(n, 0);
  end = 0;
}

void intRingBuffer::push(int x) {
  end = end+1;
  if (end == n) end = 0;
  v.at(end) = x;
}

int intRingBuffer::last() {
  return v.at(end);
}

int intRingBuffer::first() {
  return nthlast(n-1);
}

int intRingBuffer::nthlast(int i) {
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
  n = maxRollback+buffer+delay+30;
  buttonHistory.reserve(n);
  directionHistoryX.reserve(n);
  directionHistoryY.reserve(n);
  mode = LogicMode::Wait;
  lastInputFrame = currentFrame = 0;
  latencyHistory.reserve(LATENCY_HISTORY_SIZE);
  avgLatency = avgLatency = 0;
  reset();
}

void AFightInput::reset() {
  clearRollbackFlags();
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

FString ButtonRingBuffer::toString() {
  FString s;
  for (int i = 0; i < n; ++i) {
    std::optional<enum Button> o = nthlast(i);
    if (o.has_value())
      s.Append(buttonToString(o.value()));
    else
      s.Append(FString("None"));
    s.Append(FString(" "));
  }
  return s;
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
  if ((mode != LogicMode::Fight) && (mode != LogicMode::Idle)) return;
  // MYLOG(Display,
  //       TEXT("buttons(): (current frame %i) (target frame %i) (buttonsPressed %s) (buttonsReleased %s)"),
  //       currentFrame,
  //       targetFrame,
  //       *encodedButtonsToString(buttonsPressed),
  //       *encodedButtonsToString(buttonsReleased));

  lastInputFrame = targetFrame; // assumes calls maintain order

  // check if a rollback will be needed
  if (targetFrame <= (currentFrame-delay)) {
    needsRollbackToFrame = std::min(needsRollbackToFrame, targetFrame+delay);
    if // (targetFrame <= currentFrame+1 - delay - maxRollback)
      ((currentFrame - needsRollbackToFrame) >= maxRollback) {
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
    if (decodeButton(b, buttonsPressed)) {
      dxh = std::make_optional(b);
      for (int j = 1; j <= i; ++j)
        directionHistoryX.nthlast(i-j) = dxh;
    }
  }
  for (auto b: {Button::UP, Button::DOWN}) {
    if (decodeButton(b, buttonsPressed)) {
      dyh = std::make_optional(b);
      for (int j = 1; j <= i; ++j)
        directionHistoryY.nthlast(i-j) = dyh;
    }
  }

  // handle releases. this should probably just ignore everything
  // except directional inputs. we don't have to care when a button is
  // released, except when its a directional input. TODO: keep track
  // of all directions held so that when one is released we can use
  // one of the other currently held ones. _action() will have to pick
  // between which directions to prioritize.
  for (auto b: {Button::LEFT, Button::RIGHT}) {
    if (decodeButton(b, buttonsReleased) && (directionHistoryX.last() == b)) {
      dxh = {};
      for (int j = 1; j <= i; ++j)
        directionHistoryX.nthlast(i-j) = {};
    }
  }
  for (auto b: {Button::UP, Button::DOWN}) {
    if (decodeButton(b, buttonsReleased) && (directionHistoryY.last() == b)) {
      dyh = {};
      for (int j = 1; j <= i; ++j)
        directionHistoryY.nthlast(i-j) = {};
    }
  }
}

void AFightInput::ClientButtons_Implementation(int8 buttonsPressed, int8 buttonsReleased, int targetFrame, int avgLatencyOther_) {
  //MYLOG(Display, "ClientButtons");
  avgLatencyOther = avgLatencyOther_;
  int latency = currentFrame - targetFrame;
  avgLatency += latency - latencyHistory.first();
  latencyHistory.push(latency);
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

  // first we will determine the "button".
  std::vector<enum Button> buttons;

  // first try motion commands; they have the highest priority
  enum Button newButton = Button::NEUTRAL;
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
  if (newButton != Button::NEUTRAL)
    buttons.push_back(newButton);

  // try a normal attack
  if (buttonHistory.nthlast(frame).has_value()) {
    buttons.push_back(buttonHistory.nthlast(frame).value());
  }

  // try directional input
  buttons.push_back(toSingleDirection(translateDirection(directionHistoryX.nthlast(frame), isOnLeft), directionHistoryY.nthlast(frame)));

  // now with our "button" we pick an action

  // first try chains; these have highest priority
  for (auto b : buttons) {
    if (actionFrame >= currentAction.specialCancelFrames()) {
      auto i = currentAction.chains().find(b);
      if (i != currentAction.chains().end()) return i->second;
    }
  }

  if (actionFrame < currentAction.lockedFrames())
    return currentAction.character().idle(); // idle doesn't interrupt
                                             // any actions so this is
                                             // safe as a "do nothing"
                                             // return value

  for (auto b : buttons) {
    // try specials
    for (auto i : currentAction.character().specials()) {
      if (i.first == b) return i.second;
    }

    // try normals and motion
    switch (b) {
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
    case Button::LP:
      return c.stlp();
    case Button::LK:
      return c.grab();
    }
  }

  // just idle if no other action was chosen
  return c.idle();
}

bool AFightInput::checkMotionCommand(std::vector<enum Button>& motion, int m, int frame, bool isOnLeft) {
  if (m == motion.size())
    return true;
  if (frame >= n-3)
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
  if (mode == LogicMode::Idle)
    return currentAction.character().idle();

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

  for (int i = 1; (i < buffer) && action.isWalkOrIdle(); ++i) {
    action = _action(currentAction, frame+i, isOnLeft, actionFrame);
  }
  // MYLOG(Display,
  //       "action() (action %i) (current frame %i) (target frame %i) (actionFrame %i) (new action %i)",
  //       currentAction.animation(),
  //       currentFrame,
  //       targetFrame,
  //       actionFrame,
  //       (action.isWalkOrIdle() ? mostRecentAction : action).animation());
  // MYLOG(Display, "buttonHistory: %s", *(buttonHistory.toString()));
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
  return needsRollbackToFrame != std::numeric_limits<int>::max();
}

int AFightInput::getNeedsRollbackToFrame() {
  return needsRollbackToFrame;
}

void AFightInput::clearRollbackFlags() {
  needsRollbackToFrame = std::numeric_limits<int>::max();
}

int AFightInput::getAvgLatency() const {
  return avgLatency;
}

float AFightInput::getDesync() const {
  return (avgLatencyOther - avgLatency) / ((float) LATENCY_HISTORY_SIZE);
}

bool AFightInput::hasRecievedInputForFrame(int frame) const {
  return frame <= (lastInputFrame+delay);
}
