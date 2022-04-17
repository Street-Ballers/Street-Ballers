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

  const Hitbox& hitbox();
  const Hitbox& hurtbox();
  int lockedFrames();
  FVector velocity();
  bool isWalkOrIdle();

  bool operator==(const HAction& b) const;
  bool operator!=(const HAction& b) const;
};

enum HActionI {
  HActionIdleI,
  HActionWalkForwardI,
  HActionStPI
};

#define HActionIdle (HAction(HActionIdleI))
#define HActionWalkForward (HAction(HActionWalkForwardI))
#define HActionStP (HAction(HActionStPI))
