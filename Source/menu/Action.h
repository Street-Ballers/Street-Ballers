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

  Action(Hitbox hitbox, Hitbox hurtbox, int lockedFrames, FVector velocity): hitbox(hitbox), hurtbox(hurtbox), lockedFrames(lockedFrames), velocity(velocity) {};

  Action(Hitbox hitbox, Hitbox hurtbox, int lockedFrames): Action(hitbox, hurtbox, lockedFrames, FVector(0.0, 0.0, 0.0)) {};
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
};

enum HActionI {
  HActionIdleI,
  HActionWalkForwardI,
  HActionStPI
};

#define HActionIdle (HAction(HActionIdleI))
#define HActionWalkForward (HAction(HActionWalkForwardI))
#define HActionStP (HAction(HActionStPI))
