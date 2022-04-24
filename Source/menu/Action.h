#pragma once

#include "Hitbox.h"

// Defines
// - animation
// - hitboxes
// - hurtboxes
// - maps select inputs to other Actions in the case of e.g. target
//   combos
class Action {
public:
  Hitbox hitbox;
  Hitbox hurtbox;
  int lockedFrames; // number of frames that the player is locked into
                    // this action
  FVector velocity;
  bool isWalkOrIdle;

  Action(Hitbox hitbox, Hitbox hurtbox, int lockedFrames, bool isWalkOrIdle = false, FVector velocity = FVector(0.0, 0.0, 0.0)): hitbox(hitbox), hurtbox(hurtbox), lockedFrames(lockedFrames), velocity(velocity), isWalkOrIdle(isWalkOrIdle) {};

};

// handle to an action because references and pointers are bad
class HAction {
private:
  int h;
  static const Action actions[];

public:
  HAction(int h);
  HAction();

  const Hitbox& hitbox() const;
  const Hitbox& hurtbox() const;
  int lockedFrames() const;
  FVector velocity() const;
  bool isWalkOrIdle() const;

  bool operator==(const HAction& b) const;
  bool operator!=(const HAction& b) const;
};

// this is to assign integers to action names, needed for the next
// step
enum HActionI {
  HActionIdleI,
  HActionWalkForwardI,
  HActionWalkBackwardI,
  HActionStPI
};

// this is to assign HActions to action names. All other code should
// use these symbols to reference Actions
#define HActionIdle (HAction(HActionIdleI))
#define HActionWalkForward (HAction(HActionWalkForwardI))
#define HActionWalkBackward (HAction(HActionWalkBackwardI))
#define HActionStP (HAction(HActionStPI))

// Actions themselves are defined in HAction::actions[] in Logic.cpp,
// for now
