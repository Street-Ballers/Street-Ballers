#include <utility>
#include "Action.h"

Action HAction::actions[8];
Character HCharacter::characters[8];

void HAction::init() {
  actions[IActionIdle] = Action(IChar1, EAnimation::Idle, {}, Hitbox(), Hitbox(), 0, 0, 0, true);
  actions[IActionWalkBackward] = Action(IChar1, EAnimation::WalkBackward, {}, Hitbox(), Hitbox(), 0, 0, 0, true, FVector(0.0, -2.0, 0.0));
  actions[IActionWalkForward] = Action(IChar1, EAnimation::WalkForward, {}, Hitbox(), Hitbox(), 0, 0, 0, true, FVector(0.0, 4.0, 0.0));
  actions[IActionDamaged] = Action(IChar1, EAnimation::Damaged, {}, Hitbox(), Hitbox(), 0, 0, 0, true);
  actions[IActionBlock] = Action(IChar1, EAnimation::Block, {}, Hitbox(), Hitbox(), 0, 0, 0, true);

  actions[IActionStHP] = Action(IChar1,
                                EAnimation::StHP,
                                {},
                                Hitbox({
                                    // TODO: make a convenience function for
                                    // this make_pair nonsense
                                    Hitbox::make_pair(1, {}),
                                    Hitbox::make_pair(2, {Box(0.0, 100.0, 150.0, 200.0)})}),
                                Hitbox(),
                                10,
                                6,
                                6);

  // actions[IActionIdle].character = IChar1;
  check(actions[IActionIdle].character == IChar1);
  check(HActionIdle.character() == HChar1);
}

void HCharacter::init() {
  characters[IChar1] = Character(Hitbox({Box::make_centeredx(100.0, 200.0)}), HActionIdle, HActionWalkForward, HActionWalkBackward, HActionDamaged, HActionBlock, HActionStHP);
}
