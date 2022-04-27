#include "Logic.h"
#include "FightInput.h"
#include "Hitbox.h"
#include "Box.h"
#include "Action.h"

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

// if aFacingRight is true, then flip box b. Else, flip box a
bool Box::collides(const Box& b, float offsetax, float offsetay, float offsetbx, float offsetby, bool aFacingRight, bool bFacingRight) const {
  float ax = x, axend = xend;
  float bx = b.x, bxend = b.xend;
  if (!aFacingRight) {
    ax *= -1;
    axend *= -1;
    std::swap(ax, axend);
  }
  if (!bFacingRight) {
    bx *= -1;
    bxend *= -1;
    std::swap(bx, bxend);
  }
  //UE_LOG(LogTemp, Display, TEXT("Box collides(): (x %f y %f xend %f yend %f) (x %f y %f xend %f yend %f), (offsetax %f offset ay %f offsetbx %f offsetby %f)"), ax, y, axend, yend, bx, b.y, bxend, b.yend, offsetax, offsetay, offsetbx, offsetby);
  return
    // TODO: i think some of these can be removed since ax<axend an bx<bxend
    !(((ax+offsetax) < (bx+offsetbx)) &&
      ((axend+offsetax) < (bx+offsetbx))) &&
    !(((ax+offsetax) > (bxend+offsetbx)) &&
      ((axend+offsetax) > (bxend+offsetbx))) &&
    !(((y+offsetay) < (b.y+offsetby)) &&
      ((yend+offsetay) < (b.y+offsetby))) &&
    !(((y+offsetay) > (b.yend+offsetby)) &&
      ((yend+offsetay) > (b.yend+offsetby)));
}

float Box::collisionExtent(const Box& b, float offsetax, float offsetay, float offsetbx, float offsetby, bool aFacingRight, bool bFacingRight) const {
  float ax = x, axend = xend;
  float bx = b.x, bxend = b.xend;
  if (!aFacingRight) {
    ax *= -1;
    axend *= -1;
    std::swap(ax, axend);
  }
  if (!bFacingRight) {
    bx *= -1;
    bxend *= -1;
    std::swap(bx, bxend);
  }
  ax = ax+offsetax;
  axend = axend+offsetax;
  bx = bx+offsetbx;
  bxend = bxend+offsetbx;
  if (!(((y+offsetay) < (b.y+offsetby)) &&
        ((yend+offsetay) < (b.y+offsetby))) &&
      !(((y+offsetay) > (b.yend+offsetby)) &&
        ((yend+offsetay) > (b.yend+offsetby)))) {
    // boxes overlap on y axis
    if (axend < bx) {
      // no overlap on x axis
      return 0.0;
    }
    else if (ax > bxend) {
      // no overlap on x axis
      return 0.0;
    }
    else if (ax <= bx) {
      // A overlaps the left side of B; suggest move A left
      return bx-axend;
    }
    else // (axend >= bxend)
           {
      // A overlaps the right side of B; suggest move A right
      return bxend-ax;
    }
  }
  else {
    // no overlap on y axis
    return 0.0;
  }
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
bool Hitbox::collides(const Box& b, int aframe, int bframe, float offsetax, float offsetay, float offsetbx, float offsetby, bool aFacingRight, bool bFacingRight) const {
  const std::vector<Box>* aboxes = at(aframe);
  if (!aboxes) // at least one box is empty; no collision
    return false;
  for (auto& abox: *aboxes) {
    if (abox.collides(b, offsetax, offsetay, offsetbx, offsetby, aFacingRight, bFacingRight))
      return true;
  }
  return false;
}

// - b: other hitbox we are checking for collision with
// - aframe: frame of our hitboxes to check for collision
// - bframe: frame of b's hitboxes to check for collision
bool Hitbox::collides(const Hitbox& b, int aframe, int bframe, float offsetax, float offsetay, float offsetbx, float offsetby, bool aFacingRight, bool bFacingRight) const {
  const std::vector<Box>* aboxes = at(aframe);
  const std::vector<Box>* bboxes = b.at(bframe);
  if (!(aboxes && bboxes)) // at least one box is empty; no collision
    return false;
  for (auto& abox: *aboxes) {
    for (auto& bbox: *bboxes) {
      if (abox.collides(bbox, offsetax, offsetay, offsetbx, offsetby, aFacingRight, bFacingRight))
        return true;
    }
  }
  return false;
}

HAction::HAction(int h): h(h) {};
HAction::HAction(): HAction(-1) {};

HCharacter HAction::character() const {
  return HCharacter(actions[h].character);
}

const Box& HAction::collision() const {
  const std::optional<Box>& b = actions[h].collision;
  if (b.has_value())
    return b.value();
  else
    return character().collision();
}

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

HCharacter::HCharacter(int h): h(h) {};

const Box& HCharacter::collision() const {
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

HAction HCharacter::sthp() const {
  return characters[h].sthp;
}

bool HCharacter::operator==(const HCharacter& b) const {
  return h == b.h;
}

bool HCharacter::operator!=(const HCharacter& b) const {
  return !(*this == b);
}

void Player::TryStartingNewAction(int frame, AFightInput& input, bool isOnLeft) {
  if (frame - actionStart >= action.lockedFrames()) {
    std::optional<HAction> newActionO = input.action(action, isOnLeft, frame);
    if (newActionO.has_value()) {
      HAction newAction = newActionO.value();
      // don't restart action if it is walking or idling
      if (!(newAction.isWalkOrIdle() && (newAction == action))) {
        action = newAction;
        actionStart = frame;
      }
      // do update player direction regardless if we are merely
      // continuing walk/idle
      isFacingRight = isOnLeft;
    }
  }
}

// returns the amount of correction needed to move player out of the bound
float Player::collidesWithBoundary(int boundary, bool isRightBound) {
  const Box& b = action.collision();
  int x = b.x, xend = b.xend;
  if (!isFacingRight) {
    x *= -1;
    xend *= -1;
    std::swap(x, xend);
  }
  x += pos.Y;
  xend += pos.Y;
  if (isRightBound && (xend > boundary)) {
    return boundary-xend;
  }
  else if (!isRightBound && (x < boundary)) {
    return boundary-x;
  }
  else {
    return 0.0;
  }
}

bool ALogic::collides(const Box &p1b, const Box &p2b, const Frame &f, int targetFrame) {
  return p1b.collides(p2b,
                      f.p1.pos.Y, f.p1.pos.Z,
                      f.p2.pos.Y, f.p2.pos.Z,
                      f.p1.isFacingRight,
                      f.p2.isFacingRight);
}

bool ALogic::collides(const Hitbox &p1b, const Box &p2b, const Frame &f, int targetFrame) {
  return p1b.collides(p2b,
                      targetFrame - f.p1.actionStart,
                      targetFrame - f.p2.actionStart,
                      f.p1.pos.Y, f.p1.pos.Z,
                      f.p2.pos.Y, f.p2.pos.Z,
                      f.p1.isFacingRight,
                      f.p2.isFacingRight);
}

bool ALogic::collides(const Box &p1b, const Hitbox &p2b, const Frame &f, int targetFrame) {
  return p2b.collides(p1b,
                      targetFrame - f.p2.actionStart,
                      targetFrame - f.p1.actionStart,
                      f.p2.pos.Y, f.p2.pos.Z,
                      f.p1.pos.Y, f.p1.pos.Z,
                      f.p2.isFacingRight,
                      f.p1.isFacingRight);
}

bool ALogic::collides(const Hitbox &p1b, const Hitbox &p2b, const Frame &f, int targetFrame) {
  return p1b.collides(p2b,
                      targetFrame - f.p1.actionStart,
                      targetFrame - f.p2.actionStart,
                      f.p1.pos.Y, f.p1.pos.Z,
                      f.p2.pos.Y, f.p2.pos.Z,
                      f.p1.isFacingRight,
                      f.p2.isFacingRight);
}

// returns the amount of adjustment player P needs
float ALogic::playerCollisionExtent(const Player &p, const Player &q, int targetFrame) {
  const Box &pb = p.action.collision();
  const Box &qb = q.action.collision();
  return qb.collisionExtent(pb, p.pos.Y, p.pos.Z, q.pos.Y, q.pos.Z, p.isFacingRight, q.isFacingRight);
}

void ALogic::HandlePlayerBoundaryCollision(Frame &f, int targetFrame, bool doRightBoundary) {
  float stageBound = doRightBoundary ? stageBoundRight.Y : stageBoundLeft.Y;
  int p1CollisionAdj = f.p1.collidesWithBoundary(stageBound, doRightBoundary);
  int p2CollisionAdj = f.p2.collidesWithBoundary(stageBound, doRightBoundary);
  f.p1.pos.Y += p1CollisionAdj;
  f.p2.pos.Y += p2CollisionAdj;
  if ((p1CollisionAdj != 0.0) && (p2CollisionAdj == 0.0)) {
    // if p2 collides with p1, also move p2
    float collisionAdj = playerCollisionExtent(f.p2, f.p1, targetFrame);
    //f.p2.pos.Y += collisionAdj;
    UE_LOG(LogTemp, Display, TEXT("ALogic: p1 collides with %s"), doRightBoundary ? TEXT("right") : TEXT("left"));
  }
  if ((p2CollisionAdj != 0.0) && (p1CollisionAdj == 0.0)) {
    // if p1 collides with p2, also move p1
    float collisionAdj = playerCollisionExtent(f.p1, f.p2, targetFrame);
    f.p1.pos.Y += collisionAdj;
    //UE_LOG(LogTemp, Display, TEXT("ALogic: p2 collides with %s"), doRightBoundary ? TEXT("right") : TEXT("left"));
  }
  else if ((p1CollisionAdj != 0.0) && (p2CollisionAdj != 0.0)) {
    // at least one player must be jumping. Let the leftmost player
    // take the corner
    float collisionAdj;
    if (doRightBoundary) {
      collisionAdj = std::min(p1CollisionAdj, p2CollisionAdj);
    }
    else {
      collisionAdj = std::max(p1CollisionAdj, p2CollisionAdj);
    }
    f.p1.pos.Y += collisionAdj;
    f.p2.pos.Y += collisionAdj;
    //UE_LOG(LogTemp, Display, TEXT("ALogic: p1 and p2 collide with %s"), doRightBoundary ? TEXT("right") : TEXT("left"));
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
  f.p1.isFacingRight = true;
  f.p2.isFacingRight = false;
  frames.push(f);

  UE_LOG(LogTemp, Display, TEXT("ALogic: BeginPlay"));
}

void ALogic::beginFight() {
  _beginFight = true;
  p1Input->beginFight = true;
  p2Input->beginFight = true;
  UE_LOG(LogTemp, Display, TEXT("ALogic: Begin fight!"));
}

bool ALogic::IsP1OnLeft(const Frame& f) {
  return f.p1.pos.Y <= f.p2.pos.Y;
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
  bool isP1OnLeft = IsP1OnLeft(newFrame);
  newFrame.p1.TryStartingNewAction(targetFrame, *p1Input, isP1OnLeft);
  newFrame.p2.TryStartingNewAction(targetFrame, *p2Input, !isP1OnLeft);

  // compute player positions (if they are in a moving action). This
  // includes checking collision boxes and not letting players walk
  // out of bounds.
  int p1Direction = newFrame.p1.isFacingRight ? 1 : -1;
  int p2Direction = newFrame.p2.isFacingRight ? 1 : -1;
  FVector p1v = p1Direction*newFrame.p1.action.velocity();
  FVector p2v = p2Direction*newFrame.p2.action.velocity();
  newFrame.p1.pos += p1v;
  newFrame.p2.pos += p2v;
  // recompute who is on left, useful in the case of a jumping cross
  // up
  isP1OnLeft = IsP1OnLeft(newFrame);

  // check for player-player collisions
  float collisionAdj = playerCollisionExtent(newFrame.p1, newFrame.p2, targetFrame);
  if (collisionAdj != 0.0) {
    float p1CollisionAdj = 0.5 * collisionAdj;
    float p2CollisionAdj = -0.5 * collisionAdj;
    newFrame.p1.pos.Y += p1CollisionAdj;
    newFrame.p2.pos.Y += p2CollisionAdj;
    // if (!(((p1v.Y > 0) && (p2v.Y > 0)) ||
    //       ((p1v.Y < 0) && (p2v.Y < 0)))) {
    //   // players are moving into eachother. dampen how much they push
    //   // eachother by moving them back closer to where they were on
    //   // the previous frame
    //   float newMean = newFrame.p1.pos.Y + newFrame.p2.pos.Y;
    //   float oldMean = frames.last().p1.pos.Y + frames.last().p2.pos.Y;
    //   float meanAdj = 0.25 * (oldMean-newMean);
    //   newFrame.p1.pos.Y += meanAdj;
    //   newFrame.p2.pos.Y += meanAdj;
    // }
  }

  // check for player-boundary collisions
  HandlePlayerBoundaryCollision(newFrame, targetFrame, false);
  HandlePlayerBoundaryCollision(newFrame, targetFrame, true);

  // check hitboxes, compute damage. Don't forget the case of ties.
  if (collides(newFrame.p1.action.hitbox(), newFrame.p2.action.hurtbox(), newFrame, targetFrame) ||
      collides(newFrame.p1.action.hitbox(), newFrame.p2.action.collision(), newFrame, targetFrame)) {
    // hit p2
    UE_LOG(LogTemp, Display, TEXT("ALogic: P2 was hit"));
    newFrame.p2.health -= 10;
  }
  if (collides(newFrame.p1.action.hurtbox(), newFrame.p2.action.hitbox(), newFrame, targetFrame) ||
      collides(newFrame.p1.action.collision(), newFrame.p2.action.hitbox(), newFrame, targetFrame)) {
    // hit p1
    UE_LOG(LogTemp, Display, TEXT("ALogic: P1 was hit"));
    newFrame.p1.health -= 10;
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

  // TODO: needsRollback needs to take into account delay
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
    //UE_LOG(LogTemp, Display, TEXT("Logic: TICK %i!"), frame);
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
