#pragma once

#include "Hitbox.h"
#include <optional>

// Defines
// - animation
// - hitboxes
// - hurtboxes
// - maps select inputs to other Actions in the case of e.g. target
//   combos
class Action {
public:
  int character;

  // TODO: Box should really be a Hitbox here. There should be a
  // Hitbox constructor for making Hitboxes that always return the
  // same Box so that we don't have to write code for Box-Hitbox and
  // Hitbox-Hitbox collisions twice
  std::optional<Box> collision; // leave None to use character's default collision box

  Hitbox hitbox;
  Hitbox hurtbox;
  int damage;
  int lockedFrames; // number of frames that the player is locked into
                    // this action
  FVector velocity;
  bool isWalkOrIdle;

  Action(int character, std::optional<Box> collision, Hitbox hitbox, Hitbox hurtbox, int damage, int lockedFrames, bool isWalkOrIdle = false, FVector velocity = FVector(0.0, 0.0, 0.0)): hitbox(hitbox), hurtbox(hurtbox), damage(damage), lockedFrames(lockedFrames), velocity(velocity), isWalkOrIdle(isWalkOrIdle) {};

};

class HCharacter;

// handle to an action because references and pointers are bad
class HAction {
private:
  int h;
  static const Action actions[];

public:
  HAction(int h);
  HAction();

  HCharacter character() const;
  const Box& collision() const;
  const Hitbox& hitbox() const;
  const Hitbox& hurtbox() const;
  int damage() const;
  int lockedFrames() const;
  FVector velocity() const;
  bool isWalkOrIdle() const;

  bool operator==(const HAction& b) const;
  bool operator!=(const HAction& b) const;
};

// this is to assign integers to action names, needed for the next
// step
enum IAction {
  IActionIdle,
  IActionWalkForward,
  IActionWalkBackward,
  IActionStHP
};

// this is to assign HActions to action names. All other code should
// use these symbols to reference Actions
#define HActionIdle (HAction(IActionIdle))
#define HActionWalkForward (HAction(IActionWalkForward))
#define HActionWalkBackward (HAction(IActionWalkBackward))
#define HActionStHP (HAction(IActionStHP))

// Actions themselves are defined in HAction::actions[] in Logic.cpp,
// for now

// A character is a collection of Actions. This class captures the
// regularity in characters, such as each character having walking
// actions and basic button actions. This allows each action to not
// have to store handles to all these common actions.
class Character {
public:
  Box collision; // default collision box
  HAction idle;
  HAction walkForward;
  HAction walkBackward;
  HAction sthp;
  // crlp, sthk, crlk, guarding, damaged

  Character(Box collision, HAction idle, HAction walkForward, HAction walkBackward, HAction sthp): collision(collision), idle(idle), walkForward(walkForward), walkBackward(walkBackward), sthp(sthp) {};
};

class HCharacter {
private:
  int h;
  static const Character characters[];

public:
  HCharacter(int h);
  const Box& collision() const;
  HAction idle() const;
  HAction walkForward() const;
  HAction walkBackward() const;
  HAction sthp() const;

  bool operator==(const HCharacter& b) const;
  bool operator!=(const HCharacter& b) const;
};

enum ICharacter {
  IChar1
};

#define HChar1 (HCharacter(IChar1))
