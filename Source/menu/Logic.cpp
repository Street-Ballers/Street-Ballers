#include "Logic.h"
#include "FightInput.h"
#include "Hitbox.h"
#include "Box.h"
#include "Action.h"
#include <algorithm>

void RingBuffer::reserve(int size) {
  n = size;
  if (n>0)
    v = (Frame*) malloc(n*sizeof(Frame));
  start = 0;
  end = -1;
}

void RingBuffer::push(const Frame& x) {
  end = end+1;
  if (end == n) end = 0;
  v[end] = x;
  if (end == start) {
    start = start+1;
    if (start == n) start = 0;
  }
}

const Frame& RingBuffer::last() {
  return v[end];
}

void RingBuffer::popn(int m) {
  // assumes that we don't pop off more elements than we have
  end = end - m;
  if (end < 0) end += n;
}

RingBuffer::~RingBuffer() {
  if (v) free(v);
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

void Player::TryStartingNewAction(int frame, AFightInput& input, bool isFacingRight) {
  if (frame - actionStart >= action.lockedFrames()) {
    std::optional<HAction> newAction = input.action(action, isFacingRight, frame);
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
}

// Called when the game starts or when spawned
void ALogic::BeginPlay()
{
  Super::BeginPlay();

  p1Input->init(maxRollback, 2, 2);
  p2Input->init(maxRollback, 2, 2);
  frames = RingBuffer();
  frames.reserve(maxRollback+1);
  frame = 0;
  _beginFight = false;

  // construct initial frame
  Frame f (Player(leftStart, HActionIdle), Player(rightStart, HActionIdle));
  frames.push(f);

  UE_LOG(LogTemp, Display, TEXT("ALogic: BeginPlay"));
}

void ALogic::beginFight() {
  _beginFight = true;
  p1Input->beginFight = true;
  p2Input->beginFight = true;
  UE_LOG(LogTemp, Display, TEXT("ALogic: Begin fight!"));
}

bool ALogic::IsP1FacingRight(const Player& p1, const Player& p2) {
  return p1.pos.Y <= p2.pos.Y;
}

// the targetFrame field is required for using the right inputs from
// AFightInputs. The frame buffer's latest frame should be the one
// just before the targetFrame.
void ALogic::computeFrame(int targetFrame) {
  const Frame& lastFrame = frames.last();
  // make a copy of the most recent frame. we will update the values
  // in this newFrame and keep the last one.
  Frame newFrame (lastFrame);

  // If the player can act and there is a new action waiting, then
  // start the new action
  bool isP1FacingRight = IsP1FacingRight(newFrame.p1, newFrame.p2);
  newFrame.p1.TryStartingNewAction(targetFrame, *p1Input, isP1FacingRight);
  newFrame.p2.TryStartingNewAction(targetFrame, *p2Input, !isP1FacingRight);

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
  if (newFrame.p1.action.hitbox().collides(newFrame.p2.action.hurtbox(), targetFrame - newFrame.p1.actionStart, targetFrame - newFrame.p2.actionStart, newFrame.p1.pos.X, newFrame.p1.pos.Y, newFrame.p2.pos.X, newFrame.p2.pos.Y)) {
    // hit p2
  }
  if (newFrame.p1.action.hurtbox().collides(newFrame.p2.action.hitbox(), targetFrame - newFrame.p1.actionStart, targetFrame - newFrame.p2.actionStart, newFrame.p1.pos.X, newFrame.p1.pos.Y, newFrame.p2.pos.X, newFrame.p2.pos.Y)) {
    // hit p1
  }

  // check if anyone died, and if so, start new round or end game and
  // stuff. when in online multiplayer, this should also wait for both
  // clients to be at a consistent state

  frames.push(newFrame);
}

// Called every frame
void ALogic::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  int latestInputFrame = std::max(p1Input->getCurrentFrame(), p2Input->getCurrentFrame());
  int targetFrame = std::max(latestInputFrame, frame+1);

  if (p1Input->needsRollback() || p2Input->needsRollback()) {
    UE_LOG(LogTemp, Warning, TEXT("ALogic: Rollback"));
    int rollbackToFrame = std::min(p1Input->getNeedsRollbackToFrame(), p2Input->getNeedsRollbackToFrame());
    if ((1 + frame - rollbackToFrame) > maxRollback) {
      // exceeded maximum rollback. we do not have data old enough to
      // rollback, simulate the fight and guarantee consistency.
      UE_LOG(LogTemp, Warning, TEXT("ALogic: MAXIMUM ROLLBACK EXCEEDED!"));
      // TODO: quit game or maybe just reset match
    }
    else {
      // pop off all the frames that occur at or after the input
      frames.popn(1 + frame - rollbackToFrame);
    }
    p1Input->clearRollbackFlags();
    p2Input->clearRollbackFlags();
    frame = rollbackToFrame-1;
  }

  while (frame < targetFrame) {
    ++frame;
    UE_LOG(LogTemp, Display, TEXT("Logic: TICK %i!"), frame);
    computeFrame(frame);
  }

  // UE_LOG(LogTemp, Display, TEXT("Logic: TICK!"));
}

const Player& ALogic::getPlayer1() {
  return frames.last().p1;
}

const Player& ALogic::getPlayer2() {
  return frames.last().p2;
}
