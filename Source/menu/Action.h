#pragma once

#include "Hitbox.h"

// Defines
// - animation
// - hitboxes
// - hurtboxes
// - maps select inputs to other Actions in the case of e.g. target
//   combos
typedef struct Action {
  Hitbox hitbox;
  Hitbox hurtbox;
  int lockedFrames; // number of frames that the player is locked into
                    // this action
} Action;

// handle to an action because references and pointers are bad
class HAction {
private:
  int _h;
public:
  HAction(int h);
  HAction();

  const Hitbox& hitbox(const std::vector<Action>& a);
  const Hitbox& hurtbox(const std::vector<Action>& a);
  int lockedFrames(const std::vector<Action>& a);
};

