#include <utility>
#include "Action.h"

Action HAction::actions[N_ACTIONS];
Character HCharacter::characters[N_CHARACTERS];

const float jumpXVel = 2.3;

void HAction::init() {
  actions[IActionIdle]
    = Action(IChar1,
             EAnimation::Idle,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             0,
             8,
             ActionType::Idle);

  actions[IActionWalkForward]
    = Action(IChar1,
             EAnimation::WalkForward,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             0,
             24,
             ActionType::Walk,
             FVector(0.0,
                     1.3,
                     0.0));

  actions[IActionWalkBackward]
    = Action(IChar1,
             EAnimation::WalkBackward,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             0,
             HActionWalkForward.animationLength(),
             ActionType::Walk,
             (-2.0/3.0)*HActionWalkForward.velocity());

  actions[IActionDamaged]
    = Action(IChar1,
             EAnimation::Damaged,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             0,
             8,
             ActionType::DamageReaction);

  actions[IActionBlock]
    = Action(IChar1,
             EAnimation::Block,
             {},
             Hitbox(),
             Hitbox(),
             0, // leave these all 0s
             0,
             0,
             0,
             0,
             ActionType::DamageReaction);

  actions[IActionStHP]
    = Action(IChar1, // character
             EAnimation::StHP, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox({ // hitbox
                 Hitbox::make_pair(6, {}),
                 Hitbox::make_pair(8, {Box(0.0, 17.0, 21.0, 40.0)}),
                 }),
             Hitbox({ // hurtbox
                 Hitbox::make_pair(2, {Box(0.0, 17.0, 26.0, 30.0)}),
                 Hitbox::make_pair(5, {Box(0.0, 17.0, 16.0, 25.0)}),
                 Hitbox::make_pair(15, {Box(0.0, 17.0, 20.0, 39.0),
                                       Box(-13.0, 25.0, 0.0, 36.0)})}),
             14, // damage
             -4, // blockAdvantage
             2, // hitAdvantage
             15, // lockedFrames (number of frames before player can cancel)
             16, // animationLength
             ActionType::Other, // ActionType
             FVector(0, 0, 0), // velocity
             9, // specialCancelFrames
             {{Button::QCFP, HActionSpecial}} // chains
             );

  actions[IActionStLP]
    = Action(IChar1, // character
             EAnimation::StLP, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox({ // hitbox
                 Hitbox::make_pair(2, {}),
                 Hitbox::make_pair(4, {Box(0.0, 21.0, 25.0, 29.0)})}),
             Hitbox({ // hurtbox
                 Hitbox::make_pair(2, {}),
                 Hitbox::make_pair(4, {Box(0.0, 17.0, 26.0, 30.0)}),
                 Hitbox::make_pair(6, {Box(0.0, 17.0, 20.0, 30.0)})}),
             6, // damage
             2,
             1,
             6, // lockedFrames (number of frames before player can cancel)
             7, // animationLength
             ActionType::Other, // ActionType
             FVector(0, 0, 0), // velocity
             4, // specialCancelFrames
             {{Button::LP, HActionStLP},
              {Button::HP, HActionTC}} // chains
             );

  actions[IActionFJump]
    = Action(IChar1,
             EAnimation::FJump,
             Hitbox({
                 Hitbox::make_pair(8, {Box::make_centeredx(11.0, 50.0)}),
                 Hitbox::make_pair(JUMP_LENGTH-1, {Box(-6.0, 4.0, 11.0, 35.0)})}),
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             JUMP_LENGTH,
             JUMP_LENGTH,
             ActionType::Jump,
             FVector(0.0, jumpXVel, 0.0));

  actions[IActionGrab]
    = Action(IChar1, // character
             EAnimation::Grab, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox({ // hitbox
                 Hitbox::make_pair(3, {}),
                 Hitbox::make_pair(4, {Box(0.0, 21.0, 17.0, 29.0)})}),
             Hitbox({ // hurtbox
                 Hitbox::make_pair(3, {Box(0.0, 17.0, 26.0, 30.0)}),
                 Hitbox::make_pair(9, {Box(0.0, 17.0, 26.0, 30.0)}),
                 Hitbox::make_pair(14, {Box(0.0, 17.0, 16.0, 25.0)})}),
             0, // damage
             0,
             0,
             15, // lockedFrames (number of frames before player can cancel)
             15, // animationLength
             ActionType::Grab, // ActionType
             FVector(0, 0, 0),
             15,
             {},
             44.0
             );

  actions[IActionThrow]
    = Action(IChar1, // character
             EAnimation::Throw, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox(),
             Hitbox(), // hurtbox
             14, // damage
             0,
             0,
             32, // lockedFrames (number of frames before player can cancel)
             32 // animationLength
             );

  actions[IActionThrown]
    = Action(IChar1, // character
             EAnimation::Thrown, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox(), // hitbox
             Hitbox(), // hurtbox
             0, // damage
             0,
             0,
             THROWN_BOXER_LENGTH, // lockedFrames (number of frames before player can cancel)
             THROWN_BOXER_LENGTH, // animationLength
             ActionType::Thrown
             );

  actions[IActionThrownGR]
    = Action(IChar1, // character
             EAnimation::ThrownGR, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox(), // hitbox
             Hitbox(), // hurtbox
             0, // damage
             0,
             0,
             THROWN_GR_LENGTH, // lockedFrames (number of frames before player can cancel)
             THROWN_GR_LENGTH, // animationLength
             ActionType::Thrown
             );

  actions[IActionKD]
    = Action(IChar1, // character
             EAnimation::KD, // animation
             Hitbox({Box(-16, 0, 23, 10)}), // collision box (note that this is an std::optional)
             Hitbox(), // hitbox
             Hitbox(), // hurtbox
             0, // damage
             0,
             0,
             30, // lockedFrames (number of frames before player can cancel)
             30, // animationLength
             ActionType::KD
             );

  actions[IActionDefeat]
    = Action(IChar1, // character
             EAnimation::Defeat, // animation
             Hitbox({Box(-16, 0, 23, 10)}), // collision box (note that this is an std::optional)
             Hitbox(), // hitbox
             Hitbox(), // hurtbox
             0, // damage
             0,
             0,
             150, // lockedFrames (number of frames before player can cancel)
             150, // animationLength
             ActionType::KD
             );

  actions[IActionSpecial]
    = Action(IChar1, // character
             EAnimation::Special, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox({ // hitbox
                 Hitbox::make_pair(7, {}),
                 Hitbox::make_pair(9, {Box(0.0, 17.0, 26.0, 40.0)}),
                 }),
             Hitbox({ // hurtbox
                 Hitbox::make_pair(3, {Box(0.0, 17.0, 26.0, 30.0)}),
                 Hitbox::make_pair(7, {Box(0.0, 17.0, 16.0, 25.0)}),
                 Hitbox::make_pair(13, {Box(0.0, 17.0, 20.0, 39.0),
                                       Box(-13.0, 25.0, 0.0, 36.0)})}),
             16, // damage
             -7, // blockAdvantage
             0, // hitAdvantage
             18, // lockedFrames (number of frames before player can cancel)
             18, // animationLength
             ActionType::Other, // ActionType
             FVector(0, 0, 0), // velocity
             0,
             {}, // chains
             66.0
             );

  actions[IActionTC]
    = Action(IChar1, // character
             EAnimation::StHP, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox({ // hitbox
                 Hitbox::make_pair(2, {}),
                 Hitbox::make_pair(5, {Box(0.0, 17.0, 22.0, 40.0)}),
               }),
             Hitbox({ // hurtbox
                 Hitbox::make_pair(7, {Box(0.0, 17.0, 16.0, 25.0)}),
                 Hitbox::make_pair(15, {Box(0.0, 17.0, 20.0, 39.0),
                                        Box(-13.0, 25.0, 0.0, 36.0)})}),
             10, // damage
             -10, // blockAdvantage
             HActionStHP.hitAdvantage(), // hitAdvantage
             11, // lockedFrames (number of frames before player can cancel)
             13, // animationLength
             ActionType::Other, // ActionType
             FVector(0, 0, 0), // velocity
             5,
             HActionStHP.chains() // chains
             );

  // Grave robber

  actions[IActionGRIdle]
    = Action(ICharGR,
             EAnimation::GRIdle,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             0,
             16,
             ActionType::Idle);

  actions[IActionGRWalkBackward]
    = Action(ICharGR,
             EAnimation::GRWalkBackward,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             0,
             16,
             ActionType::Walk,
             FVector(0.0,
                     -1.0,
                     0.0));

  actions[IActionGRWalkForward]
    = Action(ICharGR,
             EAnimation::GRWalkForward,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             0,
             16,
             ActionType::Walk,
             FVector(0.0,
                     1.0,
                     0.0));

  actions[IActionGRDamaged]
    = Action(ICharGR,
             EAnimation::GRDamaged,
             {},
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             0,
             8,
             ActionType::DamageReaction);

  actions[IActionGRBlock]
    = Action(ICharGR,
             EAnimation::GRBlock,
             {},
             Hitbox(),
             Hitbox(),
             0, // leave these all 0s
             0,
             0,
             0,
             0,
             ActionType::DamageReaction);

  actions[IActionGRStHP]
    = Action(ICharGR, // character
             EAnimation::GRStHP, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox({ // hitbox
                 Hitbox::make_pair(1, {}),
                 Hitbox::make_pair(2, {Box(0.0, 17.0, 22.0, 34.0)})}),
             Hitbox(), // hurtbox
             10, // damage
             0,
             0,
             9, // lockedFrames (number of frames before player can cancel)
             9, // animationLength
             ActionType::Other, // ActionType
             FVector(0, 0, 0), // velocity
             6, // specialCancelFrames
             {{Button::QCFP, HActionGRFJump}} // chains
             );

  actions[IActionGRStLP]
    = Action(ICharGR, // character
             EAnimation::GRStLP, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox({ // hitbox
                 Hitbox::make_pair(1, {}),
                 Hitbox::make_pair(2, {Box(0.0, 17.0, 22.0, 34.0)})}),
             Hitbox(), // hurtbox
             5, // damage
             0,
             3,
             5, // lockedFrames (number of frames before player can cancel)
             8, // animationLength
             ActionType::Other, // ActionType
             FVector(0, 0, 0), // velocity
             3, // specialCancelFrames
             {{Button::LP, HActionGRStLP}} // chains
             );

  actions[IActionGRFJump]
    = Action(ICharGR,
             EAnimation::GRFJump,
             Hitbox({Box::make_centeredx(11.0, 34.0)}),
             Hitbox(),
             Hitbox(),
             0,
             0,
             0,
             JUMP_LENGTH,
             JUMP_LENGTH,
             ActionType::Jump,
             FVector(0.0, jumpXVel, 0.0));

  actions[IActionGRThrown]
    = Action(ICharGR, // character
             EAnimation::GRThrown, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox(), // hitbox
             Hitbox(), // hurtbox
             0, // damage
             0,
             0,
             THROWN_BOXER_LENGTH, // lockedFrames (number of frames before player can cancel)
             THROWN_BOXER_LENGTH, // animationLength
             ActionType::Thrown
             );

  actions[IActionGRKD]
    = Action(ICharGR, // character
             EAnimation::GRKD, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox(), // hitbox
             Hitbox(), // hurtbox
             0, // damage
             0,
             0,
             30, // lockedFrames (number of frames before player can cancel)
             30, // animationLength
             ActionType::KD
             );

  actions[IActionGRDefeat]
    = Action(ICharGR, // character
             EAnimation::GRDefeat, // animation
             {}, // collision box (note that this is an std::optional)
             Hitbox(), // hitbox
             Hitbox(), // hurtbox
             0, // damage
             0,
             0,
             150, // lockedFrames (number of frames before player can cancel)
             150, // animationLength
             ActionType::KD
             );
}

void HCharacter::init() {
  characters[IChar1]
    = Character(Hitbox({Box::make_centeredx(22.0, 34.0)}),
                HActionIdle,
                HActionWalkForward,
                HActionWalkBackward,
                HActionFJump,
                HActionDamaged,
                HActionBlock,
                HActionStHP,
                HActionStLP,
                HActionGrab,
                HActionThrow,
                HActionThrown,
                HActionThrownGR,
                HActionKD,
                HActionDefeat,
                {{Button::QCFP, HActionSpecial}});

  characters[ICharGR]
    = Character(Hitbox({Box::make_centeredx(22.0, 34.0)}),
                HActionGRIdle,
                HActionGRWalkForward,
                HActionGRWalkBackward,
                HActionGRFJump,
                HActionGRDamaged,
                HActionGRBlock,
                HActionGRStHP,
                HActionGRStLP,
                HActionGRIdle,
                HActionGRIdle,
                HActionGRThrown,
                HActionGRIdle,
                HActionGRKD,
                HActionGRDefeat,
                {});
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

float knockdownAirborneHeights[knockdownAirborneLength] = {
  28.0,
  28.0,
  28.0,
  27.0,
  26.0,
  24.0,
  19.0,
  14.0,
  8.0,
  0.0
};

FVector thrownBoxerPositions[THROWN_BOXER_LENGTH+1] = {
  FVector(0.0, 0.0, 0.0),
  FVector(0.0, 22.0, 0.0),
  FVector(0.0, 19.0, 0.0),
  FVector(0.0, 17.0, 0.0),
  FVector(0.0, 16.0, 0.0),
  FVector(0.0, 15.0, 0.0),
  FVector(0.0, 15.0, 0.0),
  FVector(0.0, 15.0, 0.0),
  FVector(0.0, 15.0, 0.0),
  FVector(0.0, 15.0, 0.0),
  FVector(0.0, 15.0, 0.0),
  FVector(0.0, 27.0, 0.0)
};

// FVector thrownGRPositions[THROWN_GR_LENGTH];

std::map<enum Button, std::vector<std::vector<enum Button>>> motionCommands;

void init_actions() {
  HAction::init();
  HCharacter::init();

  // two ways to input QCFP
  motionCommands[Button::QCFP] = {{Button::DOWN, Button::DOWNFORWARD, Button::FORWARD, Button::HP},
                                  {Button::DOWN, Button::FORWARD, Button::HP}};
}
