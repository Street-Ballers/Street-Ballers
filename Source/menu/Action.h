#pragma once

#include "Button.h"
#include "Hitbox.h"
#include <optional>
#include <map>

UENUM(BlueprintType)
enum EAnimation {
Idle = 0,
WalkBackward = 1,
WalkForward = 2,
FJump = 3,
Damaged = 4,
Block = 5,
StHP = 6,
StLP = 7,
Grab = 8,
Throw = 9,
Thrown = 10,
ThrownGR = 11,
KD = 12,
Defeat = 13,
Special = 14,
TC = 15,
GRIdle = 16,
GRWalkBackward = 17,
GRWalkForward = 18,
GRFJump = 19,
GRDamaged = 20,
GRBlock = 21,
GRStHP = 22,
GRStLP = 23,
GRGrab = 24,
GRThrow = 25,
GRThrown = 26,
GRThrownGR = 27,
GRKD = 28,
GRDefeat = 29
};

enum class ActionType { Idle, Walk, Jump, Grab, Throw, Thrown, KD, DamageReaction, Other };

class HAction;

// Defines things such as
// - animation
// - hitboxes
// - hurtboxes
// - map select inputs to other Actions in the case of e.g.
//   target combos
class Action {
public:
  int character;

  enum EAnimation animation;

  std::optional<Hitbox> collision; // leave None to use character's default collision box

  Hitbox hitbox;
  Hitbox hurtbox;
  int damage;
  int blockAdvantage;
  int hitAdvantage;
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

  int specialCancelFrames; // number of frames that the player cannot
                           // special cancel
  std::map<enum Button, HAction> chains; // actions that chain from
                                         // this one when pressed
                                         // after specialCancelFrames
                                         // have passed

  float knockdownDistance; // for hard knockdowns only
  float pushbackDistance; // normal pushback
  bool hitsWalkingBack; // true if the move can hit players walking
                        // backward

  Action(int character, enum EAnimation animation, std::optional<Hitbox> collision, Hitbox hitbox, Hitbox hurtbox, int damage, int blockAdvantage, int hitAdvantage, int lockedFrames, int animationLength, enum ActionType type = ActionType::Other, FVector velocity = FVector(0.0, 0.0, 0.0), int specialCancelFrames = 0, std::map<enum Button, HAction> chains = {}, float knockdownDistance = -1, float pushbackDistance = 7.0, bool hitsWalkingBack = false): character(character), animation(animation), collision(collision), hitbox(hitbox), hurtbox(hurtbox), damage(damage), blockAdvantage(blockAdvantage), hitAdvantage(hitAdvantage), lockedFrames(lockedFrames), animationLength(animationLength), type(type), velocity(velocity), specialCancelFrames(specialCancelFrames), chains(chains), knockdownDistance(knockdownDistance), pushbackDistance(pushbackDistance), hitsWalkingBack(hitsWalkingBack) {};

  // don't use this constructor
  Action(): Action(-1, EAnimation::Idle, Hitbox(), Hitbox(), Hitbox(), 0, 0, 0, 0, 0) {};
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
  const Box& collision(int frame) const;
  const Hitbox& hitbox() const;
  const Hitbox& hurtbox() const;
  int damage() const;
  int blockAdvantage() const;
  int hitAdvantage() const;
  int lockedFrames() const;
  int animationLength() const;
  FVector velocity() const;
  bool isWalkOrIdle() const;
  enum ActionType type() const;
  int specialCancelFrames() const;
  float knockdownDistance() const;
  float pushbackDistance() const;
  bool hitsWalkingBack() const;
  const std::map<enum Button, HAction>& chains() const;

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
  IActionStLP = 6,
  IActionFJump = 7,
  IActionGrab,
  IActionThrow,
  IActionThrown,
  IActionThrownGR,
  IActionKD,
  IActionDefeat,
  IActionSpecial,
  IActionTC,
  IActionGRIdle,
  IActionGRWalkForward,
  IActionGRWalkBackward,
  IActionGRDamaged,
  IActionGRBlock,
  IActionGRStHP,
  IActionGRStLP,
  IActionGRFJump,
  IActionGRThrown,
  IActionGRKD,
  IActionGRDefeat,
};

// this section is to assign HActions to action names. All other code
// should use these symbols to reference Actions. the line below is a
// regexp for generating this automatically.
// - (replace-regexp "^ *I\\(\\w+\\).*$" "#define H\\1 (HAction(I\\1))")
#define HActionIdle (HAction(IActionIdle))
#define HActionWalkForward (HAction(IActionWalkForward))
#define HActionWalkBackward (HAction(IActionWalkBackward))
#define HActionDamaged (HAction(IActionDamaged))
#define HActionBlock (HAction(IActionBlock))
#define HActionStHP (HAction(IActionStHP))
#define HActionStLP (HAction(IActionStLP))
#define HActionFJump (HAction(IActionFJump))
#define HActionGrab (HAction(IActionGrab))
#define HActionThrow (HAction(IActionThrow))
#define HActionThrown (HAction(IActionThrown))
#define HActionThrownGR (HAction(IActionThrownGR))
#define HActionKD (HAction(IActionKD))
#define HActionDefeat (HAction(IActionDefeat))
#define HActionSpecial (HAction(IActionSpecial))
#define HActionTC (HAction(IActionTC))
#define HActionGRIdle (HAction(IActionGRIdle))
#define HActionGRWalkForward (HAction(IActionGRWalkForward))
#define HActionGRWalkBackward (HAction(IActionGRWalkBackward))
#define HActionGRDamaged (HAction(IActionGRDamaged))
#define HActionGRBlock (HAction(IActionGRBlock))
#define HActionGRStHP (HAction(IActionGRStHP))
#define HActionGRStLP (HAction(IActionGRStLP))
#define HActionGRFJump (HAction(IActionGRFJump))
#define HActionGRThrown (HAction(IActionGRThrown))
#define HActionGRKD (HAction(IActionGRKD))
#define HActionGRDefeat (HAction(IActionGRDefeat))

// Actions themselves are defined in HAction::actions[] in Logic.cpp,
// for now

// A character is a collection of Actions. This class captures the
// regularity in characters, such as each character having walking
// actions and basic button actions. This allows each action to not
// have to store handles to all these common actions.
class Character {
public:
  const char* name;
  Hitbox collision; // default collision box
  HAction idle;
  HAction walkForward;
  HAction walkBackward;
  HAction fJump;
  HAction damaged;
  HAction block;
  HAction sthp;
  HAction stlp;
  HAction grab;
  HAction throw_;
  HAction thrown;
  HAction thrownGR;
  HAction kd;
  HAction defeat;
  std::map<enum Button, HAction> specials;

  Character(const char* name, Hitbox collision, HAction idle, HAction walkForward, HAction walkBackward, HAction fJump, HAction damaged, HAction block, HAction sthp, HAction stlp, HAction grab, HAction throw_, HAction thrown, HAction thrownGR, HAction kd, HAction defeat, std::map<enum Button, HAction> specials): collision(collision), idle(idle), walkForward(walkForward), walkBackward(walkBackward), fJump(fJump), damaged(damaged), block(block), sthp(sthp), stlp(stlp), grab(grab), throw_(throw_), thrown(thrown), thrownGR(thrownGR), kd(kd), defeat(defeat), specials(specials) {};

  // don't use this constructor
  Character(): Character("", Hitbox({Box(0, 0, 0, 0)}), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), HAction(), {}) {};
};

class HCharacter {
private:
  int h;
  #define N_CHARACTERS 8
  static Character characters[N_CHARACTERS];

public:
  HCharacter(int h): h(h) {};
  HCharacter(): h(0) {};
  static void init();
  const char* name() const;
  const Hitbox& collision() const;
  HAction idle() const;
  HAction walkForward() const;
  HAction walkBackward() const;
  HAction fJump() const;
  HAction damaged() const;
  HAction block() const;
  HAction sthp() const;
  HAction stlp() const;
  HAction grab() const;
  HAction throw_() const;
  HAction thrown() const;
  HAction thrownGR() const;
  HAction kd() const;
  HAction defeat() const;
  const std::map<enum Button, HAction>& specials() const;

  bool operator==(const HCharacter& b) const;
  bool operator!=(const HCharacter& b) const;
};

enum ICharacter {
  IChar1 = 0,
  ICharGR = 1
};

#define HChar1 (HCharacter(IChar1))
#define HCharGR (HCharacter(ICharGR))

// This function needs to be called early in the game startup to
// populate the actions and character arrays
extern void init_actions();

const int knockdownAirborneLength = 10;
extern float knockdownAirborneHeights[knockdownAirborneLength];

#define JUMP_LENGTH 22
extern float jumpHeights[JUMP_LENGTH];
#define THROWN_BOXER_LENGTH 11
extern FVector thrownBoxerPositions[THROWN_BOXER_LENGTH+1];
#define THROWN_GR_LENGTH 11
extern FVector thrownGRPositions[THROWN_GR_LENGTH+1];

extern std::map<enum Button, std::vector<std::vector<enum Button>>> motionCommands;
