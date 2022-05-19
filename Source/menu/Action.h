#pragma once

#include "Hitbox.h"
#include <optional>

UENUM(BlueprintType)
enum EAnimation { Idle, WalkBackward, WalkForward, FJump, Damaged, Block, StHP };

enum class ActionType { Idle, Walk, Jump, Other };

// Defines things such as
// - animation
// - hitboxes
// - hurtboxes
// - could map select inputs to other Actions in the case of e.g.
//   target combos
class Action {
public:
  int character;

  enum EAnimation animation;

  std::optional<Hitbox> collision; // leave None to use character's default collision box

  Hitbox hitbox;
  Hitbox hurtbox;
  int damage;
  int lockedFrames; // number of frames that the player is locked into
                    // this action
  int animationLength; // length of the actual animation. The player
                       // may cancel the end of the animation by
                       // pressing a button after lockedFrames have
                       // passed, but if they don't cancel, the action
                       // will continue for a total of animationLength
                       // frames.

  enum ActionType type;

  FVector velocity; // constant velocity that a player moves forward
                    // during a action. Probably only useful for
                    // walking.

  Action(int character, enum EAnimation animation, std::optional<Hitbox> collision, Hitbox hitbox, Hitbox hurtbox, int damage, int lockedFrames, int animationLength, enum ActionType type = ActionType::Other, FVector velocity = FVector(0.0, 0.0, 0.0)): character(character), animation(animation), collision(collision), hitbox(hitbox), hurtbox(hurtbox), damage(damage), lockedFrames(lockedFrames), animationLength(animationLength), type(type), velocity(velocity) {};

  // don't use this constructor
  Action(): Action(-1, EAnimation::Idle, Hitbox(), Hitbox(), Hitbox(), 0, 0, 0) {};
};

class HCharacter;

// handle to an action because references and pointers are bad
class HAction {
private:
  int h;
  #define N_ACTIONS 128
  static Action actions[N_ACTIONS];

public:
  HAction(int h): h(h) {};
  HAction(): HAction(-1) {};
  static void init();

  HCharacter character() const;
  enum EAnimation animation() const;
  const Hitbox& collision() const;
  const Hitbox& hitbox() const;
  const Hitbox& hurtbox() const;
  int damage() const;
  int lockedFrames() const;
  int animationLength() const;
  FVector velocity() const;
  bool isWalkOrIdle() const;
  enum ActionType type() const;

  bool operator==(const HAction& b) const;
  bool operator!=(const HAction& b) const;
};

// this is to assign integers to action names, needed for the next
// step
enum IAction {
  IActionIdle = 0,
  IActionWalkForward = 1,
  IActionWalkBackward = 2,
  IActionDamaged = 3,
  IActionBlock = 4,
  IActionStHP = 5,
  IActionFJump = 6
};

// this is to assign HActions to action names. All other code should
// use these symbols to reference Actions
#define HActionIdle (HAction(IActionIdle))
#define HActionWalkForward (HAction(IActionWalkForward))
#define HActionWalkBackward (HAction(IActionWalkBackward))
#define HActionDamaged (HAction(IActionDamaged))
#define HActionBlock (HAction(IActionBlock))
#define HActionStHP (HAction(IActionStHP))
#define HActionFJump (HAction(IActionFJump))

// Actions themselves are defined in HAction::actions[] in Logic.cpp,
// for now

// A character is a collection of Actions. This class captures the
// regularity in characters, such as each character having walking
// actions and basic button actions. This allows each action to not
// have to store handles to all these common actions.
class Character {
public:
  Hitbox collision; // default collision box
  HAction idle;
  HAction walkForward;
  HAction walkBackward;
  HAction fJump;
  HAction damaged;
  HAction block;
  HAction sthp;
  // crlp, sthk, crlk, guarding, damaged

  Character(Hitbox collision, HAction idle, HAction walkForward, HAction walkBackward, HAction fJump, HAction damaged, HAction block, HAction sthp): collision(collision), idle(idle), walkForward(walkForward), walkBackward(walkBackward), fJump(fJump), damaged(damaged), block(block), sthp(sthp) {};

  // don't use this constructor
  Character(): Character(Hitbox({Box(0, 0, 0, 0)}), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), HAction()) {};
};

class HCharacter {
private:
  int h;
  #define N_CHARACTERS 8
  static Character characters[N_CHARACTERS];

public:
  HCharacter(int h): h(h) {};
  static void init();
  const Hitbox& collision() const;
  HAction idle() const;
  HAction walkForward() const;
  HAction walkBackward() const;
  HAction fJump() const;
  HAction damaged() const;
  HAction block() const;
  HAction sthp() const;

  bool operator==(const HCharacter& b) const;
  bool operator!=(const HCharacter& b) const;
};

enum ICharacter {
  IChar1 = 0
};

#define HChar1 (HCharacter(IChar1))

// This function needs to be called early in the game startup to
// populate the actions and character arrays
void init_actions();

#define JUMP_LENGTH 22
extern float jumpHeights[JUMP_LENGTH];
