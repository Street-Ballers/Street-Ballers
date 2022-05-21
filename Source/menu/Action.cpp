#include "Action.h"

int HAction::damage() const {
  return actions[h].damage;
}

int HAction::blockAdvantage() const {
  return actions[h].blockAdvantage;
}

int HAction::hitAdvantage() const {
  return actions[h].hitAdvantage;
}

int HAction::lockedFrames() const {
  return actions[h].lockedFrames;
}

int HAction::animationLength() const {
  return actions[h].animationLength;
}

FVector HAction::velocity() const {
  return actions[h].velocity;
}

bool HAction::isWalkOrIdle() const {
  return (actions[h].type == ActionType::Idle) || (actions[h].type == ActionType::Walk);
}

enum ActionType HAction::type() const {
  return actions[h].type;
}

enum EAnimation HAction::animation() const {
  return actions[h].animation;
}

int HAction::specialCancelFrames() const {
  return actions[h].specialCancelFrames;
}

const std::map<enum Button, HAction>& HAction::chains() const {
  return actions[h].chains;
}

bool HAction::operator==(const HAction& b) const {
  return h == b.h;
}

bool HAction::operator!=(const HAction& b) const {
  return !(*this == b);
}

const Hitbox& HCharacter::collision() const {
  return characters[h].collision;
}

HAction HCharacter::idle() const {
  return characters[h].idle;
}

HAction HCharacter::walkForward() const {
  return characters[h].walkForward;
}

HAction HCharacter::walkBackward() const {
  return characters[h].walkBackward;
}

HAction HCharacter::fJump() const {
  return characters[h].fJump;
}

HAction HCharacter::damaged() const {
  return characters[h].damaged;
}

HAction HCharacter::block() const {
  return characters[h].block;
}

HAction HCharacter::sthp() const {
  return characters[h].sthp;
}

HAction HCharacter::stlp() const {
  return characters[h].stlp;
}

HAction HCharacter::grab() const {
  return characters[h].grab;
}

HAction HCharacter::throw_() const {
  return characters[h].throw_;
}

HAction HCharacter::thrown() const {
  return characters[h].thrown;
}

HAction HCharacter::thrownGR() const {
  return characters[h].thrownGR;
}

HAction HCharacter::kd() const {
  return characters[h].kd;
}

bool HCharacter::operator==(const HCharacter& b) const {
  return h == b.h;
}

bool HCharacter::operator!=(const HCharacter& b) const {
  return !(*this == b);
}
