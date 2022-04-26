// Fill out your copyright notice in the Description page of Project Settings.

#include "FightInput.h"
#include <algorithm>

void ButtonRingBuffer::reserve(int size) {
  n = size;
  v.clear();
  v.resize(n, {});
  end = 0;
}

void ButtonRingBuffer::push(const std::optional<enum Button>& x) {
  end = end+1;
  if (end == n) end = 0;
  v[end] = x;
}

std::optional<enum Button>& ButtonRingBuffer::last() {
  return v.at(end);
}

std::optional<enum Button>& ButtonRingBuffer::nthlast(int i) {
  int j = end-i;
  if (j < 0) j += n;
  return v.at(j);
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
  currentFrame = 0;
  needsRollbackToFrame = -1;
  buttonHistory = ButtonRingBuffer();
  buttonHistory.reserve(n);
  UE_LOG(LogTemp, Warning, TEXT("AFightInput: size %i"), buttonHistory.v.size());
  UE_LOG(LogTemp, Warning, TEXT("AFightInput: 2size %i"), buttonHistory.v.size());
  directionHistory = ButtonRingBuffer();
  directionHistory.reserve(n);
  beginFight = false;
};

void AFightInput::ensureFrame(int targetFrame) {
  while (targetFrame > currentFrame) {
    // assume nothing was pressed for frames we are skipping here. The
    // last one we push is actually for the new frame and we will
    // store the recieved inputs in that one.
    buttonHistory.push({});
    directionHistory.push(directionHistory.last());
    ++currentFrame;
  }
}

void AFightInput::buttons(const std::vector<const enum Button>& buttonsPressed, const std::vector<const enum Button>& buttonsReleased, int targetFrame) {
  if (!beginFight) return;
  UE_LOG(LogTemp,
         Warning,
         TEXT("AFightInput buttons(): %s (current frame %i) (target frame %i) (button: %s)"),
         *GetActorLabel(false),
         currentFrame,
         targetFrame,
         (!buttonsPressed.empty()) ? ((buttonsPressed[0]==Button::RIGHT) ? TEXT("right") : TEXT("not right")) : TEXT("none"));

  // check if a rollback will be needed
  if (targetFrame <= currentFrame) {
    if (getNeedsRollbackToFrame()) {
      needsRollbackToFrame = std::min(needsRollbackToFrame, targetFrame);
    }
    else {
      needsRollbackToFrame = targetFrame;
    }
    if (targetFrame <= currentFrame+1 - maxRollback) {
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
  std::optional<enum Button>& dh = directionHistory.nthlast(i);

  // handle presses. this is a really simple implementation that just
  // sets the button pressed to the last button that happens to appear
  // in buttonsPressed.
  if (!buttonsPressed.empty()) {
    for (auto b : buttonsPressed) {
      if (is_button(b)) {
        bh = std::make_optional(b);
      }
      if (!is_button(b)) {
        dh = std::make_optional(b);
      }
    }
  }

  // handle releases. this should probably just ignore everything
  // except directional inputs. we don't have to care when a button is
  // released, except when its a directional input. TODO: keep track
  // of all directions held so that when one is released we can use
  // one of the other currently held ones. _action() will have to pick
  // between which directions to prioritize.
  if (!buttonsReleased.empty()) {
    for (auto b : buttonsReleased) {
      if (!is_button(b) && (directionHistory.last() == b))
        dh = {};
    }
  }

  UE_LOG(LogTemp, Warning, TEXT("AFightInput end of buttons(): %s (i %i) (button: %s) (buttonlast %s)"), *GetActorLabel(false), i, (dh.has_value() && (dh.value() == Button::RIGHT)) ? TEXT("right") : TEXT("not right"), (directionHistory.last().has_value() && (directionHistory.last().value() == Button::RIGHT)) ? TEXT("right") : TEXT("not right"));
}

HAction AFightInput::_action(HAction currentAction, int frame, bool isFacingRight) {
  const HCharacter& c = currentAction.character();
  // for now, if there was a button, output the corresponding attack.
  // If no button, then walk/idle based on directional input.
  UE_LOG(LogTemp, Warning, TEXT("AFightInput _action(): %s (frame %i) (button %s)"), *GetActorLabel(false), frame, directionHistory.nthlast(frame).has_value() ? ((directionHistory.nthlast(frame).value() == Button::RIGHT) ? TEXT("right") : TEXT("not right")) : TEXT("none"));
  if (buttonHistory.nthlast(frame).has_value()) {
    enum Button button = buttonHistory.nthlast(frame).value();
    if (button == Button::HP){
      UE_LOG(LogTemp, Warning, TEXT("AFightInput _action(): %s Do StHP"), *GetActorLabel(false));
      return c.sthp();
    }
  }
  if (directionHistory.nthlast(frame).has_value()) {
    // translate directional input based on character direction
    enum Button button = directionHistory.nthlast(frame).value();
    if (((button == Button::RIGHT) && isFacingRight) ||
        ((button == Button::LEFT) && !isFacingRight))
      button = Button::FORWARD;
    else
      button = Button::BACK;

    if (button == Button::FORWARD)
      return c.walkForward();
    if (button == Button::BACK)
      return c.walkBackward();
  }
  return c.idle();
}

HAction AFightInput::action(HAction currentAction, bool isFacingRight, int targetFrame) {
  int frameBefore = currentFrame;
  ensureFrame(targetFrame);
  //UE_LOG(LogTemp, Warning, TEXT("AFightInput action(): %s (current frame %i) (target frame %i)"), *GetActorLabel(false), currentFrame, targetFrame);

  int frame = (currentFrame - targetFrame)+delay;
  // Try decoding an action based on the inputs at `frame'. If the
  // action we decode is walking or idling, then try using the inputs
  // one frame earlier. Repeat until we find an action that isn't
  // idling or walking, or we have tried all of the `input
  // buffer' frames.
  HAction mostRecentAction = _action(currentAction, frame, isFacingRight);
  UE_LOG(LogTemp, Warning, TEXT("AFightInput action(): %s (current frame %i) (target frame %i) (action: %s)"), *GetActorLabel(false), currentFrame, targetFrame, (mostRecentAction == HActionIdle) ? TEXT("idle") : TEXT("not idle"));
  HAction action = mostRecentAction;
  while (action.isWalkOrIdle() && (++frame <= (delay+buffer))) {
    action = _action(currentAction, frame, isFacingRight);
  }
  return action.isWalkOrIdle() ? mostRecentAction : action;
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
