#include <utility>
#include "Action.h"

const Action HAction::actions[] = {
  [IActionIdle] = Action(IChar1, {}, Hitbox({}), Hitbox({}), 0, true),
  [IActionWalkBackward] = Action(IChar1, {}, Hitbox({}), Hitbox({}), 0, 0, true, FVector(0.0, -2.0, 0.0)),
  [IActionWalkForward] = Action(IChar1, {}, Hitbox({}), Hitbox({}), 0, 0, true, FVector(0.0, 4.0, 0.0)),

  [IActionStHP] = Action(IChar1,
                         {},
                         Hitbox({
                             // TODO: make a convenience function for
                             // this make_pair nonsense
                             std::make_pair<int, std::vector<Box>>(1, {}),
                             std::make_pair<int, std::vector<Box>>(2, {Box(0.0, 100.0, 150.0, 200.0)})}),
                         Hitbox({}),
                         10,
                         6),
};

const Character HCharacter::characters[] = {
  [IChar1] = Character(Box::make_centeredx(100.0, 200.0), HActionIdle, HActionWalkForward, HActionWalkBackward, HActionStHP)
};
