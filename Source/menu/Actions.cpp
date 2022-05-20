#include <utility>
#include "Action.h"

Action HAction::actions[N_ACTIONS];
Character HCharacter::characters[N_CHARACTERS];

void HAction::init() {
  actions[IActionIdle]
    = Action(IChar1,
             EAnimation::Idle,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             6,
             ActionType::Idle);

  actions[IActionWalkBackward]
    = Action(IChar1,
             EAnimation::WalkBackward,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             16,
             ActionType::Walk,
             FVector(0.0,
                     -6.0,
                     0.0));

  actions[IActionWalkForward]
    = Action(IChar1,
             EAnimation::WalkForward,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             16,
             ActionType::Walk,
             FVector(0.0,
                     6.0,
                     0.0));

  actions[IActionDamaged]
    = Action(IChar1,
             EAnimation::Damaged,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             7);

  actions[IActionBlock]
    = Action(IChar1,
             EAnimation::Block,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             0);

  actions[IActionStHP]
    = Action(IChar1, // character
             EAnimation::StHP, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox({ // hitbox
                 Hitbox::make_pair(1, {}),
                 Hitbox::make_pair(2, {Box(0.0, 100.0, 150.0, 200.0)})}),
             Hitbox(), // hurtbox
             10, // damage
             9, // lockedFrames (number of frames before player can cancel)
             12, // animationLength
             ActionType::Other, // ActionType
             FVector(0, 0, 0), // velocity
             6, // specialCancelFrames
             {{Button::QCFP, HActionFJump}} // chains
             );

  actions[IActionFJump]
    = Action(IChar1,
             EAnimation::FJump,
             {},
             {},
             Hitbox(),
             0,
             JUMP_LENGTH,
             JUMP_LENGTH,
             ActionType::Jump,
             FVector(0.0, 7.0, 0.0));
}

void HCharacter::init() {
  characters[IChar1]
    = Character(Hitbox({Box::make_centeredx(100.0, 200.0)}),
                HActionIdle,
                HActionWalkForward,
                HActionWalkBackward,
                HActionFJump,
                HActionDamaged,
                HActionBlock,
                HActionStHP);
}

// set xrange [0:22]
// set yrange [0:20]
// f(x) = 10 - 10*((1/11.0)*abs(x-11))**3
// plot f(x) notitle with lines
// print f(0)
// print f(1)
// print f(2)
// print f(3)
// print f(4)
// print f(5)
// print f(6)
// print f(7)
// print f(8)
// print f(9)
// print f(10)
// print f(11)
// print f(12)
// print f(13)
// print f(14)
// print f(15)
// print f(16)
// print f(17)
// print f(18)
// print f(19)
// print f(20)
// print f(21)
// print f(22)

float jumpHeights[JUMP_LENGTH] = {
  0.0,
  2.48685199098422,
  4.5229151014275,
  6.15326821938392,
  7.42299023290759,
  8.37716003005259,
  9.06085649887303,
  9.51915852742299,
  9.79714500375657,
  9.93989481592787,
  9.99248685199098,
  9.99248685199098,
  9.93989481592787,
  9.79714500375657,
  9.51915852742299,
  9.06085649887303,
  8.37716003005259,
  7.42299023290759,
  6.15326821938392,
  4.5229151014275,
  2.48685199098422,
  0.0
};

std::map<enum Button, std::vector<std::vector<enum Button>>> motionCommands;

void init_actions() {
  HAction::init();
  HCharacter::init();

  // two ways to input QCFP
  motionCommands[Button::QCFP] = {{Button::DOWN, Button::DOWNFORWARD, Button::FORWARD, Button::HP},
                                  {Button::DOWN, Button::FORWARD, Button::HP}};
}
