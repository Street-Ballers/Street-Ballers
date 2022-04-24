#include "Logic.h"
#include "Input.h"
#include "Hitbox.h"
#include "Box.h"
#include "Action.h"

void RingBuffer::reserve(int size) {
  n = size;
  if (n>0)
    v = (Frame*) malloc(n*sizeof(Frame));
  start = 0;
  end = 0;
}

void RingBuffer::push(const Frame& x) {
  end = (end+1) % n;
  v[end] = x;
  if (end == start)
    start = (start+1) % n;
}

const Frame& RingBuffer::last() {
  return v[end];
}

RingBuffer::~RingBuffer() {
  if (v) free(v);
}

void ButtonRingBuffer::reserve(int size) {
  n = size;
  v.reserve(n);
  start = 0;
  end = 0;
}

void ButtonRingBuffer::push(const std::optional<enum Button>& x) {
  end = (end+1) % n;
  v[end] = x;
  if (end == start)
    start = (start+1) % n;
}

std::optional<enum Button>& ButtonRingBuffer::last() {
  return v[end];
}

std::optional<enum Button>& ButtonRingBuffer::nthlast(int i) {
  return v[end-i];
}

bool Input::is_button(const enum Button& b) {
  switch (b) {
  case Button::LP:
  case Button::HP:
  case Button::LK:
  case Button::HK:
    return true;
  default:
    return false;
  }
}

// bool Input::is_none(const Button& b) {
//   return b == Button::NONE;
// }

void Input::init(int _maxRollback, int _buffer, int _delay) {
  buffer = _buffer;
  delay = _delay;
  n = _maxRollback+buffer;
  buttonHistory.reserve(n);
  directionHistory.reserve(n);
  buttonHistory.push({});
  directionHistory.push({});
};

void Input::buttonsPressed(const std::vector<const enum Button>& buttons) {
  buttonHistory.push({});
  directionHistory.push(directionHistory.last());
  if (!buttons.empty()) {
    for (auto b : buttons) {
      if (is_button(b))
        buttonHistory.last() = std::make_optional(b);
      if (!is_button(b))
        directionHistory.last() = std::make_optional(b);
    }
  }
}

void Input::buttonsReleased(const std::vector<const enum Button>& buttons) {
  if (!buttons.empty()) {
    for (auto b : buttons) {
      if (!is_button(b) && (directionHistory.last() == b))
        directionHistory.last() = {};
    }
  }
}

HAction Input::_action(HAction currentAction, int frame, bool isFacingRight) {
  // for now, if there was a button, output the corresponding attack.
  // If no button, then walk/idle based on directional input.
  if (buttonHistory.nthlast(frame).has_value()) {
    enum Button button = buttonHistory.nthlast(frame).value();
    if (button == Button::LP)
      return HActionStP;
  }
  if (directionHistory.nthlast(frame).has_value()) {
    enum Button button = directionHistory.nthlast(frame).value();
    if (((button == Button::RIGHT) && isFacingRight) ||
        ((button == Button::LEFT) && !isFacingRight))
      button = Button::FORWARD;
    else
      button = Button::BACK;

    if (button == Button::FORWARD)
      return HActionWalkForward;
    if (button == Button::BACK)
      return HActionWalkBackward;
  }
  return HActionIdle;
}

HAction Input::action(HAction currentAction, bool isFacingRight) {
  int frame = delay;
  HAction action = _action(currentAction, frame, isFacingRight);
  while ((++frame < n) && (frame < buffer)) {
    HAction a = _action(currentAction, frame, isFacingRight);
    if (!a.isWalkOrIdle())
      action = a;
  }
  return action;
}

bool Box::collides(const Box& b, float offsetax, float offsetay, float offsetbx, float offsetby) const {
  return
    !(((x+offsetax) < (b.x+offsetbx)) &&
      ((xend+offsetax) < (b.x+offsetbx))) &&
    !(((x+offsetax) > (b.xend+offsetbx)) &&
      ((xend+offsetax) > (b.xend+offsetbx))) &&
    !(((y+offsetay) < (b.y+offsetby)) &&
      ((yend+offsetay) < (b.y+offsetby))) &&
    !(((y+offsetay) > (b.yend+offsetby)) &&
      ((yend+offsetay) > (b.yend+offsetby)));
}

const std::vector<Box>* Hitbox::at(int frame) const {
  // scan through boxes for the last pair that starts at or before
  // frame, and return the corresponding vector
  auto i =
    find_if(boxes.begin(),
            boxes.end(),
            [frame](std::pair<int, std::vector<Box>> x){
              return x.first >= frame;
            });
  if (i == boxes.end())
    return nullptr;
  else
    return &(i->second);
}

// - b: other hitbox we are checking for collision with
// - aframe: frame of our hitboxes to check for collision
// - bframe: frame of b's hitboxes to check for collision
bool Hitbox::collides(const Hitbox& b, int aframe, int bframe, float offsetax, float offsetay, float offsetbx, float offsetby) const {
  const std::vector<Box>* aboxes = at(aframe);
  const std::vector<Box>* bboxes = b.at(bframe);
  if (!(aboxes && bboxes)) // at least one box is empty; no collision
    return false;
  for (auto& abox: *aboxes) {
    for (auto& bbox: *bboxes) {
      if (abox.collides(bbox, offsetax, offsetay, offsetbx, offsetby))
        return true;
    }
  }
  return false;
}

HAction::HAction(int h): h(h) {};
HAction::HAction(): HAction(-1) {};

const Action HAction::actions[] = {
  [HActionIdleI] = Action(Hitbox({}), Hitbox({}), 0, true),
  [HActionWalkBackwardI] = Action(Hitbox({}), Hitbox({}), 0, true, FVector(0.0, -2.0, 0.0)),
  [HActionWalkForwardI] = Action(Hitbox({}), Hitbox({}), 0, true, FVector(0.0, 4.0, 0.0)),
  [HActionStPI] = Action(Hitbox({}), Hitbox({}), 0),
};

const Hitbox& HAction::hitbox() const {
  return actions[h].hitbox;
}

const Hitbox& HAction::hurtbox() const {
  return actions[h].hurtbox;
}

int HAction::lockedFrames() const {
  return actions[h].lockedFrames;
}

FVector HAction::velocity() const {
  return actions[h].velocity;
}

bool HAction::isWalkOrIdle() const {
  return actions[h].isWalkOrIdle;
}

bool HAction::operator==(const HAction& b) const {
  return h == b.h;
}

bool HAction::operator!=(const HAction& b) const {
  return !(*this == b);
}

void Player::TryStartingNewAction(int frame, Input& input, bool isFacingRight) {
  if (frame - actionStart >= action.lockedFrames()) {
    std::optional<HAction> newAction = input.action(action, isFacingRight);
    if (newAction.has_value()) {
      action = newAction.value();
      actionStart = frame;
    }
  }
}


// Sets default values for this component's properties
ALogic::ALogic(): frame(0)
{
  // Set this component to be initialized when the game starts, and to
  // be ticked every frame. You can turn these features off to improve
  // performance if you don't need them.
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;

  frames.reserve(maxRollback);
  p1Input.init(maxRollback, 2, 2);
  p2Input.init(maxRollback, 2, 2);
}

// Called when the game starts or when spawned
void ALogic::BeginPlay()
{
  Super::BeginPlay();

  // construct initial frame
  Frame f (Player(leftStart, HActionIdle), Player(rightStart, HActionIdle));
  frames.push(f);
}

bool ALogic::IsP1FacingRight(const Player& p1, const Player& p2) {
  return p1.pos.Y <= p2.pos.Y;
}

// Called every frame
void ALogic::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  ++frame;
  const Frame& lastFrame = frames.last();
  // make a copy of the most recent frame. we will update the values
  // in this newFrame and keep the last one.
  Frame newFrame (lastFrame);

  // input will be sent to p1Input/p2Input by LogicPlayerController.
  // When there is only one player, we need to send empty inputs to
  // p2Input.
  // p2Input.buttonsPressed({});

  // If the player can act and there is a new action waiting, then
  // start the new action
  bool isP1FacingRight = IsP1FacingRight(newFrame.p1, newFrame.p2);
  //newFrame.p1.TryStartingNewAction(frame, p1Input, isP1FacingRight);
  //newFrame.p2.TryStartingNewAction(frame, p2Input, !isP1FacingRight);

  // compute player positions (if they are in a moving action). This
  // includes checking collision boxes and not letting players walk
  // out of bounds.
  int p1Direction = IsP1FacingRight(newFrame.p1, newFrame.p2) ? 1 : -1;
  int p2Direction = -1*p1Direction;
  newFrame.p1.pos += p1Direction*newFrame.p1.action.velocity();
  newFrame.p2.pos += p2Direction*newFrame.p2.action.velocity();
  p1Direction = (newFrame.p1.pos.Y > newFrame.p2.pos.Y) ? -1 : 1;
  p2Direction = -1*p1Direction;

  // check hitboxes, compute damage. Don't forget the case of ties.
  if (newFrame.p1.action.hitbox().collides(newFrame.p2.action.hurtbox(), frame - newFrame.p1.actionStart, frame - newFrame.p2.actionStart, newFrame.p1.pos.X, newFrame.p1.pos.Y, newFrame.p2.pos.X, newFrame.p2.pos.Y)) {
    // hit p2
  }
  if (newFrame.p1.action.hurtbox().collides(newFrame.p2.action.hitbox(), frame - newFrame.p1.actionStart, frame - newFrame.p2.actionStart, newFrame.p1.pos.X, newFrame.p1.pos.Y, newFrame.p2.pos.X, newFrame.p2.pos.Y)) {
    // hit p1
  }

  // check if anyone died, and if so, start new round or end game and
  // stuff. when in online multiplayer, this should also wait for both
  // clients to be at a consistent state.

  frames.push(newFrame);

  // UE_LOG(LogTemp, Display, TEXT("Logic: TICK!"));
}

const Player& ALogic::getPlayer1() {
  return frames.last().p1;
}

const Player& ALogic::getPlayer2() {
  return frames.last().p2;
}
