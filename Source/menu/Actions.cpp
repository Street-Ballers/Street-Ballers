#include <utility>
#include "Action.h"

Action HAction::actions[8];
Character HCharacter::characters[8];

void HAction::init() {
  actions[IActionIdle] = Action(IChar1, {}, Hitbox({}), Hitbox({}), 0, true);
  actions[IActionWalkBackward] = Action(IChar1, {}, Hitbox({}), Hitbox({}), 0, 0, true, FVector(0.0, -2.0, 0.0));
  actions[IActionWalkForward] = Action(IChar1, {}, Hitbox({}), Hitbox({}), 0, 0, true, FVector(0.0, 4.0, 0.0));
  actions[IActionDamaged] = Action(IChar1, {}, Hitbox({}), Hitbox({}), 0, true);
  actions[IActionBlock] = Action(IChar1, {}, Hitbox({}), Hitbox({}), 0, true);

  actions[IActionStHP] = Action(IChar1,
                                {},
                                Hitbox({
                                    // TODO: make a convenience function for
                                    // this make_pair nonsense
                                    std::make_pair<int, std::vector<Box>>(1, {}),
                                    std::make_pair<int, std::vector<Box>>(2, {Box(0.0, 100.0, 150.0, 200.0)})}),
                                Hitbox({}),
                                10,
                                6);

  // actions[IActionIdle].character = IChar1;
  check(actions[IActionIdle].character == IChar1);
  check(HActionIdle.character() == HChar1);
}

void HCharacter::init() {
  characters[IChar1] = Character(Box::make_centeredx(100.0, 200.0), HActionIdle, HActionWalkForward, HActionWalkBackward, HActionDamaged, HActionBlock, HActionStHP);
}
