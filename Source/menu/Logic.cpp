#include "Logic.h"
#include "FightInput.h"
#include "Hitbox.h"
#include "Box.h"
#include "Action.h"
#include <algorithm>
#include <cmath>
#include <limits>

#define MYLOG(category, message, ...) UE_LOG(LogTemp, category, TEXT("ALogic (%s) " message), (GetWorld()->IsNetMode(NM_ListenServer)) ? TEXT("server") : TEXT("client"), ##__VA_ARGS__)

void RingBuffer::reserve(int size) {
  n = size;
  clear();
}

void RingBuffer::clear() {
  v.clear();
  v.resize(n);
  end = 0;
}

void RingBuffer::push(const Frame& x) {
  end = end+1;
  if (end == n) end = 0;
  v.at(end) = x;
}

const Frame& RingBuffer::last() {
  return v.at(end);
}

void RingBuffer::popn(int m) {
  // assumes that we don't pop off more elements than we have
  end = end - m;
  if (end < 0) end += n;
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
  //MYLOG(Display, "Box collides(): (x %f y %f xend %f yend %f) (x %f y %f xend %f yend %f), (offsetax %f offset ay %f offsetbx %f offsetby %f)", ax, y, axend, yend, bx, b.y, bxend, b.yend, offsetax, offsetay, offsetbx, offsetby);
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
      if (axend >= bxend) {
        // B is inside A; move left/right based on centers
        if ((bx+bxend) <= (ax+axend))
          return bx-axend;
        else
          return ax-bxend;
      }
      else
        // A overlaps the left side of B; suggest move A left
        return bx-axend;
    }
    else if (axend >= bxend) {
      // A overlaps the right side of B; suggest move A right
      return bxend-ax;
    }
    else /* (ax < bx) && (axend < bxend) */ {
      // A is inside B; move left/right based on centers
      if ((bx+bxend) <= (ax+axend))
        return bx-axend;
      else
        return ax-bxend;
    }
  }
  else {
    // no overlap on y axis
    return 0.0;
  }
}

hitbox_pair Hitbox::make_pair(int endFrame, std::vector<Box> boxes) {
  return std::make_pair(endFrame, boxes);
}

Hitbox::Hitbox(std::vector<Box> _boxes) {
  boxes = std::vector({Hitbox::make_pair(std::numeric_limits<int>::max(), _boxes)});
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

void Player::startNewAction(int frame, HAction newAction, bool isOnLeft) {
  actionNumber++;
  action = newAction;
  actionStart = frame;
  isFacingRight = isOnLeft;
}

void Player::TryStartingNewAction(int frame, AFightInput& input, bool isOnLeft) {
  if (action.type() == ActionType::Thrown) {
    if ((frame - actionStart) == action.animationLength()) {
      float knockdownVelocityp = knockdownVelocity;
      doKdAction(frame, isOnLeft, knockdownVelocityp);
    }
  }
  else {
    if (frame - actionStart >= action.specialCancelFrames()) {
      HAction newAction = input.action(action, isOnLeft, frame, actionStart);
      // don't interrupt current action if the new action is just an idle unless we are walking
      if (!((action.isWalkOrIdle() &&
             (newAction == action) &&
             (frame - actionStart < action.animationLength())) ||
            (!action.isWalkOrIdle() &&
             (newAction.type() == ActionType::Idle) &&
             (frame - actionStart < action.animationLength())))) {
        startNewAction(frame, newAction, isOnLeft);
      }
      // do update player direction if we are merely continuing
      // walk/idle
      else if (action.isWalkOrIdle() &&
               (newAction == action) &&
               (frame - actionStart < action.animationLength())) {
        isFacingRight = isOnLeft;
      }
    }
  }
}

void Player::doDamagedAction(int frame) {
  action = action.character().damaged();
  actionStart = frame;
}

void Player::doBlockAction(int frame) {
  action = action.character().block();
  actionStart = frame;
}

void Player::doKdAction(int frame, bool isOnLeft, float knockdownDistance) {
  knockdownVelocity = knockdownDistance / knockdownAirborneLength;
  startNewAction(frame, action.character().kd(), isOnLeft);
}

void Player::doThrownAction(int frame, bool isOnLeft, float knockdownDistance, HAction newAction, Player& q) {
  startNewAction(frame, newAction, isOnLeft);
  knockdownVelocity = knockdownDistance;
  pos = q.pos + (isOnLeft ? -1 : 1) * thrownBoxerPositions[1];
}

// returns the amount of correction needed to move player out of the bound
float Player::collidesWithBoundary(float boundary, bool isRightBound, int targetFrame) {
  const Box& b = action.collision(targetFrame);
  float x = b.x, xend = b.xend;
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

void Player::doMotion(int targetFrame) {
  if (action.type() == ActionType::Jump) {
    pos.Z = 5*jumpHeights[targetFrame - actionStart];
  }
  if (action.type() == ActionType::Thrown) {
    pos += (isFacingRight ? -1 : 1) * (thrownBoxerPositions[targetFrame - actionStart + 1] - thrownBoxerPositions[targetFrame - actionStart]);
  }
  if (action.type() == ActionType::KD) {
    if ((targetFrame - actionStart) < knockdownAirborneLength) {
      pos.Z = knockdownAirborneHeights[targetFrame - actionStart];
      pos.Y += (isFacingRight ? -1 : 1) * knockdownVelocity;
    }
  }
  if ((action.type() == ActionType::DamageReaction) && ((targetFrame - actionStart) == action.animationLength())) {
    ++actionStart;
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
  if ((p.action.type() == ActionType::Thrown) ||
        (q.action.type() == ActionType::Thrown) ||
        ((p.action.type() == ActionType::KD) && (frame - p.actionStart) < knockdownAirborneLength) ||
        ((q.action.type() == ActionType::KD) && (frame - q.actionStart) < knockdownAirborneLength)) {
    return 0.0;
  }
  else {
    const Box &pb = p.action.collision(targetFrame);
    const Box &qb = q.action.collision(targetFrame);
    return qb.collisionExtent(pb, p.pos.Y, p.pos.Z, q.pos.Y, q.pos.Z, p.isFacingRight, q.isFacingRight);
  }
}

void ALogic::HandlePlayerBoundaryCollision(Frame &f, int targetFrame, bool doRightBoundary) {
  float stageBound = doRightBoundary ? stageBoundRight.Y : stageBoundLeft.Y;
  float p1CollisionAdj = f.p1.collidesWithBoundary(stageBound, doRightBoundary, targetFrame);
  float p2CollisionAdj = f.p2.collidesWithBoundary(stageBound, doRightBoundary, targetFrame);
  f.p1.pos.Y += p1CollisionAdj;
  f.p2.pos.Y += p2CollisionAdj;
  if ((p1CollisionAdj != 0.0) && (p2CollisionAdj == 0.0)) {
    // if p2 collides with p1, also move p2
    float collisionAdj = playerCollisionExtent(f.p2, f.p1, targetFrame);
    //f.p2.pos.Y += collisionAdj;
    // MYLOG(Display, "HandlePlayerBoundaryCollision: p1 collides with %s", doRightBoundary ? TEXT("right") : TEXT("left"));
  }
  if ((p2CollisionAdj != 0.0) && (p1CollisionAdj == 0.0)) {
    // if p1 collides with p2, also move p1
    float collisionAdj = playerCollisionExtent(f.p1, f.p2, targetFrame);
    f.p1.pos.Y += collisionAdj;
    //MYLOG(Display, "p2 collides with %s", doRightBoundary ? TEXT("right") : TEXT("left"));
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
    //MYLOG(Display, "p1 and p2 collide with %s", doRightBoundary ? TEXT("right") : TEXT("left"));
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

  MYLOG(Display, TEXT("BeginPlay"));

  // initialize some variables

  frames = RingBuffer();
  frames.reserve(maxRollback+1);

  const int delay = 1;
  const int buffer = 2;
  p1Input->init(maxRollback, buffer, delay);
  p2Input->init(maxRollback, buffer, delay);

  mode = LogicMode::Wait;
  inPreRound = false;
  inEndRound = false;
  frame = 0;
  reset(false);
}

void ALogic::reset(bool flipSpawns) {
  p1Input->reset();
  p2Input->reset();
  // construct initial frame
  Frame f (Player(flipSpawns ? rightStart : leftStart, HActionIdle), Player(flipSpawns ? leftStart : rightStart, HActionGRIdle));
  f.p1.isFacingRight = IsP1OnLeft(f);
  f.p2.isFacingRight = !IsP1OnLeft(f);
  frames.clear();
  frames.push(f);
  frames.push(f);
}

void ALogic::setMode(enum LogicMode m) {
  mode = m;
  p1Input->setMode(m);
  p2Input->setMode(m);
}

void ALogic::preRound() {
  MYLOG(Display, "preRound");
  if (!skipPreRound) {
    setMode(LogicMode::Idle);
    inPreRound = true;
    roundStartFrame = ((frame+PREROUND_TIME)/15)*15;
  }
  reset(false);
  rollbackStopFrame = frame;
  if (skipPreRound) {
    beginRound();
  }
  else {
    if(OnPreRound.IsBound()) {
      OnPreRound.Broadcast();
    }
  }
}
void ALogic::beginRound() {
  MYLOG(Display, "beginRound");
  rollbackStopFrame = frame;
  setMode(LogicMode::Fight);
  if(OnBeginRound.IsBound()) {
    OnBeginRound.Broadcast();
  }
}

void ALogic::endRound() {
  MYLOG(Display, "endRound");
  setMode(LogicMode::Idle);
  inEndRound = true;
  roundStartFrame = ((frame+ENDROUND_TIME)/15)*15;
  rollbackStopFrame = frame;
  if(OnEndRound.IsBound()) {
    OnEndRound.Broadcast();
  }
}

void ALogic::endFight() {
  MYLOG(Display, "endFight");
  setMode(LogicMode::Wait);
  if(OnEndFight.IsBound()) {
    OnEndFight.Broadcast();
  }
}

bool ALogic::IsPlayerOnLeft(const Player& p1, const Player& p2) {
  return p1.pos.Y <= p2.pos.Y;
}

bool ALogic::IsP1OnLeft(const Frame& f) {
  return IsPlayerOnLeft(f.p1, f.p2);
}

// the targetFrame field is required for using the right inputs from
// AFightInputs. The frame buffer's latest frame should be the one
// just before the targetFrame.
void ALogic::computeFrame(int targetFrame) {
  const Frame& lastFrame = frames.last();
  // make a copy of the most recent frame. we will update the values
  // in this newFrame and keep the last one.
  Frame newFrame (lastFrame);
  Player& p1 = newFrame.p1;
  Player& p2 = newFrame.p2;

  // If the player can act and there is a new action waiting, then
  // start the new action
  bool isP1OnLeft = IsP1OnLeft(newFrame);
  if ((p1.action.type() == ActionType::Thrown) && ((targetFrame - p1.actionStart) == p1.action.animationLength()))
    p1.health -= p2.action.damage();
  if ((p2.action.type() == ActionType::Thrown) && ((targetFrame - p2.actionStart) == p2.action.animationLength()))
    p2.health -= p1.action.damage();
  if (p1.hitstun == 0)
    p1.TryStartingNewAction(targetFrame, *p1Input, isP1OnLeft);
  else
    --p1.hitstun;
  if (p2.hitstun == 0)
    p2.TryStartingNewAction(targetFrame, *p2Input, !isP1OnLeft);
  else
    --p2.hitstun;

  // compute player positions (if they are in a moving action). This
  // includes checking collision boxes and not letting players walk
  // out of bounds.
  int p1Direction = p1.isFacingRight ? 1 : -1;
  int p2Direction = p2.isFacingRight ? 1 : -1;
  FVector p1v = p1Direction*p1.action.velocity();
  FVector p2v = p2Direction*p2.action.velocity();
  FVector oldP1Posv = p1.pos,
    oldP2Posv = p2.pos;
  double oldP1Pos = p1.pos.Y,
    oldP2Pos = p2.pos.Y,
    oldPos = (oldP1Pos + oldP2Pos)/2;
  p1.pos += p1v;
  p2.pos += p2v;
  if (std::abs(p1.pos.Y - p2.pos.Y) > 121.0) {
    if (std::abs(p1.pos.Y - oldPos) > std::abs(oldP1Pos - oldPos)) {
      p1.pos = oldP1Posv;
    }
    if (std::abs(p2.pos.Y - oldPos) > std::abs(oldP2Pos - oldPos)) {
      p2.pos = oldP2Posv;
    }
  }
  p1.doMotion(targetFrame);
  p2.doMotion(targetFrame);
  // recompute who is on left, useful in the case of a jumping cross
  // up
  isP1OnLeft = IsP1OnLeft(newFrame);

  // check for player-player collisions
  float collisionAdj = playerCollisionExtent(p1, p2, targetFrame);
  if (collisionAdj != 0.0) {
    float p1CollisionAdj = 0.5 * collisionAdj;
    float p2CollisionAdj = -0.5 * collisionAdj;
    if (p1.pos.Y == p2.pos.Y) {
      // players are on top of eachother; move the higher player in
      // their current velocity direction
      int direction = (p1.action.velocity().Y > 0) ? 1 : -1;
      p1CollisionAdj = direction * std::abs(p1CollisionAdj);
      p2CollisionAdj = -1 * direction * std::abs(p2CollisionAdj);
    }
    p1.pos.Y += p1CollisionAdj;
    p2.pos.Y += p2CollisionAdj;
    // if (!(((p1v.Y > 0) && (p2v.Y > 0)) ||
    //       ((p1v.Y < 0) && (p2v.Y < 0)))) {
    //   // players are moving into eachother. dampen how much they push
    //   // eachother by moving them back closer to where they were on
    //   // the previous frame
    //   float newMean = p1.pos.Y + p2.pos.Y;
    //   float oldMean = frames.last().p1.pos.Y + frames.last().p2.pos.Y;
    //   float meanAdj = 0.25 * (oldMean-newMean);
    //   p1.pos.Y += meanAdj;
    //   p2.pos.Y += meanAdj;
    // }
  }

  // check for player-boundary collisions
  HandlePlayerBoundaryCollision(newFrame, targetFrame, false);
  HandlePlayerBoundaryCollision(newFrame, targetFrame, true);
  isP1OnLeft = IsP1OnLeft(newFrame);

  // check hitboxes, compute damage. Don't forget the case of ties.
  //MYLOG(Display, "hitstop %i", newFrame.hitstop);
  if (newFrame.hitstop == 0) { // only check hitboxes if we are not in hitstop
    if ((p1.action.type() != ActionType::KD) && (p2.action.type() != ActionType::KD) &&
        ((p1.action.type() != ActionType::DamageReaction) || (p1.actionNumber != p2.actionNumber)) &&
        ((p2.action.type() != ActionType::DamageReaction) || (p2.actionNumber != p1.actionNumber))) {
      bool p1Hit = false, p1Block = false, p1Grabbed = false;
      int p1Damage = 0;
      bool p2Hit = false, p2Block = false, p2Grabbed = false;
      int p2Damage = 0;
      float p1KnockdownDistance = -1, p2KnockdownDistance = -1;
      const float chipDamageMultiplier = 0.1;
      if (collides(p1.action.hitbox(), p2.action.hurtbox(), newFrame, targetFrame) ||
          collides(p1.action.hitbox(), Hitbox({p2.action.collision(targetFrame)}), newFrame, targetFrame)) {
        // hit p2
        MYLOG(Display, "P2 was hit");
        if (p1.action.type() == ActionType::Grab){
          p2Grabbed = true;
          p2KnockdownDistance = p1.action.knockdownDistance();
        }
        else {
          p2Hit = true;
          ++p1.actionNumber;
          p2.actionNumber = p1.actionNumber;
          p2.hitstun = p1.action.lockedFrames() - (targetFrame-p1.actionStart);
          p2Damage = p1.action.damage();
          if ((p2.action.type() != ActionType::Jump) && p2Input->isGuarding(!isP1OnLeft, targetFrame)){
            p2Block = true;
            if (p1.action.blockAdvantage() >= 0)
              p2.hitstun += p1.action.blockAdvantage();
            else
              p1.hitstun -= p1.action.blockAdvantage();
            p2.health -= p2Damage * chipDamageMultiplier; // chip damage
          }
          else {
            if (p1.action.hitAdvantage() >= 0)
              p2.hitstun += p1.action.hitAdvantage();
            else
              p1.hitstun -= p1.action.hitAdvantage();
            p2.health -= p2Damage;
            p2KnockdownDistance = p1.action.knockdownDistance();
          }
        }
      }
      if (collides(p1.action.hurtbox(), p2.action.hitbox(), newFrame, targetFrame) ||
          collides(Hitbox({p1.action.collision(targetFrame)}), p2.action.hitbox(), newFrame, targetFrame)) {
        // hit p1
        MYLOG(Display, "P1 was hit");
        if (p2.action.type() == ActionType::Grab) {
          p1Grabbed = true;
          p1KnockdownDistance = p2.action.knockdownDistance();
        }
        else {
          p1Hit = true;
          ++p1.actionNumber;
          p2.actionNumber = p1.actionNumber;
          p1.hitstun = p2.action.lockedFrames() - (targetFrame-p2.actionStart);
          p1Damage = p2.action.damage();
          if ((p1.action.type() != ActionType::Jump) && p1Input->isGuarding(isP1OnLeft, targetFrame)){
            p1Block = true;
            if (p2.action.blockAdvantage() >= 0)
              p1.hitstun += p2.action.blockAdvantage();
            else
              p2.hitstun -= p2.action.blockAdvantage();
            p1.health -= p1Damage * chipDamageMultiplier; // chip damage
          }
          else {
            if (p2.action.hitAdvantage() >= 0)
              p1.hitstun += p2.action.hitAdvantage();
            else
              p2.hitstun -= p2.action.hitAdvantage();
            p1.health -= p1Damage;
            p1KnockdownDistance = p2.action.knockdownDistance();
          }
        }
      }
      if (p1Hit || p2Hit)
        p1Grabbed = p2Grabbed = false; // grabs lose to attacks

      // TODO: should spawn special FX on hit/block here
      if (p1Hit) {
        p1.pos.Z = 0;
        if (p1Block) {
          p1.doBlockAction(targetFrame);
        }
        else {
          if (p1KnockdownDistance >= 0)
            p1.doKdAction(targetFrame, isP1OnLeft, p1KnockdownDistance);
          else
            p1.doDamagedAction(targetFrame);
        }
        newFrame.hitPlayer = 1;
      }
      if (p2Hit) {
        p2.pos.Z = 0;
        if (p2Block) {
          p2.doBlockAction(targetFrame);
        }
        else {
          if (p2KnockdownDistance >= 0)
            p2.doKdAction(targetFrame, !isP1OnLeft, p2KnockdownDistance);
          else
            p2.doDamagedAction(targetFrame);
        }
        newFrame.hitPlayer = 2;
      }
      if (p1Grabbed && p2Grabbed) {
        // both players grabbed at same time; no tech animation so just
        // do block animation with pushback
        p1.doBlockAction(targetFrame);
        p2.doBlockAction(targetFrame);
        newFrame.hitstop = 10;
        newFrame.pushbackPerFrame = 3.0;
      }
      else {
        newFrame.hitstop = std::min(1, std::max(p1Damage, p2Damage)/5);
        newFrame.pushbackPerFrame = 5.0 / newFrame.hitstop;
      }
      if ((p1Hit && p2Hit) || (p1Grabbed && p2Grabbed)) {
        newFrame.hitPlayer = 0;
        // add the hitstop because we won't do real hitstop when ties
        // happen, only pushback
        p1.hitstun += newFrame.hitstop;
        p2.hitstun += newFrame.hitstop;
      }
      if (!(p1Grabbed && p2Grabbed)) {
        if (p1Grabbed) {
          p1.doThrownAction(targetFrame, isP1OnLeft, p1KnockdownDistance, p1.action.character().thrown(), p2);
          p2.startNewAction(targetFrame, p2.action.character().throw_(), !isP1OnLeft);
        }
        if (p2Grabbed) {
          p1.startNewAction(targetFrame, p1.action.character().throw_(), isP1OnLeft);
          p2.doThrownAction(targetFrame, !isP1OnLeft, p2KnockdownDistance, p2.action.character().thrown(), p1);
        }
      }
    }
  }
  else { // we are in hitstop
    // keep the attacking player frozen, do pushback, keep players in bounds
    if ((newFrame.hitPlayer == 1) || (newFrame.hitPlayer == 0)) {
      // do pushback
      p1.pos.Y += (isP1OnLeft ? -1 : 1) * newFrame.pushbackPerFrame;
    }
    if ((newFrame.hitPlayer == 2) || (newFrame.hitPlayer == 0)) {
      // do pushback
      p2.pos.Y += (!isP1OnLeft ? -1 : 1) * newFrame.pushbackPerFrame;
    }
    if (newFrame.hitPlayer == 1) {
      // this causes the freeze on the attacker's animation
      ++p2.actionStart;
    }
    if (newFrame.hitPlayer == 2) {
      // this causes the freeze on the attacker's animation
      ++p1.actionStart;
    }
    // in the case of a tie, we don't freeze any players

    // put players back in bounds
    Player& pleft = isP1OnLeft ? p1 : p2;
    Player& pright = !isP1OnLeft ? p1 : p2;
    int collisionExtent = pleft.collidesWithBoundary(stageBoundLeft.Y, false, targetFrame);
    p1.pos.Y += collisionExtent;
    p2.pos.Y += collisionExtent;
    collisionExtent = pright.collidesWithBoundary(stageBoundRight.Y, true, targetFrame);
    p1.pos.Y += collisionExtent;
    p2.pos.Y += collisionExtent;

    --newFrame.hitstop;
  }

  // check if anyone died, and if so, start new round or end game and
  // stuff. when in online multiplayer, this should also wait for both
  // clients to be at a consistent state
  if ((!inEndRound) && ((p1.health <= 0) || (p2.health <= 0))) {
    if (p1.health <= 0) {
      if (p1.action.type() != ActionType::KD)
        p1.knockdownVelocity = 10.0;
      p1.startNewAction(targetFrame, p1.action.character().defeat(), isP1OnLeft);
    }
    if (p2.health <= 0){
      if (p2.action.type() != ActionType::KD)
        p2.knockdownVelocity = 10.0;
      p2.startNewAction(targetFrame, p2.action.character().defeat(), !isP1OnLeft);
    }
    endRound();
    // do stuff on end fight; declare winner, display message, call
    // reset(roundNumber%2) and beginFight()
  }

  frames.push(newFrame);
}

void ALogic::FightTick() {
  // MYLOG(Display, "FightTick");

  int latestInputFrame = std::max(p1Input->getCurrentFrame(), p2Input->getCurrentFrame());
  int targetFrame = std::max(latestInputFrame, frame+1);

  if (alwaysRollback || p1Input->needsRollback() || p2Input->needsRollback()) {
    MYLOG(Warning, "Rollback");
    // rollbackToFrame is the frame just before the input
    int rollbackToFrame = std::min(p1Input->getNeedsRollbackToFrame(), p2Input->getNeedsRollbackToFrame());
    if (alwaysRollback) rollbackToFrame = frame - maxRollback + 1;
    rollbackToFrame = std::max(rollbackStopFrame, rollbackToFrame);
    if ((frame - rollbackToFrame + 1) > maxRollback) {
      // exceeded maximum rollback. we do not have data old enough to
      // rollback, simulate the fight and guarantee consistency.
      MYLOG(Warning, "MAXIMUM ROLLBACK EXCEEDED!");
      // TODO: quit game or maybe just reset match
    }
    else {
      // pop off all the frames that occur at or after the input
      frames.popn(frame - rollbackToFrame + 1);
    }
    p1Input->clearRollbackFlags();
    p2Input->clearRollbackFlags();
    frame = rollbackToFrame-1;
  }

  while (frame < targetFrame) {
    ++frame;
    // MYLOG(Display, "TICK %i!", frame);
    computeFrame(frame);
  }
}

// Called every frame
void ALogic::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  // MYLOG(Display, "Tick");
  switch (mode) {
  case LogicMode::Idle:
    if (inPreRound && (frame == (roundStartFrame-1))) {
      inPreRound = false;
      beginRound();
    }
    if (inEndRound && (frame == (roundStartFrame-1))) {
      inEndRound = false;
      preRound();
    }
  case LogicMode::Fight:
    FightTick();
    break;
  }
}

const Player& ALogic::getPlayer1() {
  return frames.last().p1;
}

const Player& ALogic::getPlayer2() {
  return frames.last().p2;
}

const Player& ALogic::getPlayer(int playerNumber) {
  switch (playerNumber) {
  case 0: return getPlayer1();
  case 1: return getPlayer2();
  default:
    MYLOG(Error, "getPlayer: playerNumber is not 0 or 1! (player number: %i)", playerNumber);
    return getPlayer1();
  }
}

FVector ALogic::playerPos(int playerNumber) {
  return getPlayer(playerNumber).pos;
}

bool ALogic::playerIsFacingRight(int playerNumber) {
  return getPlayer(playerNumber).isFacingRight;
}

int ALogic::playerHealth(int playerNumber) {
  return getPlayer(playerNumber).health;
}

int ALogic::playerAnimation(int playerNumber) {
  return getPlayer(playerNumber).action.animation();
}

int ALogic::playerFrame(int playerNumber) {
  return (frame - getPlayer(playerNumber).actionStart);
}
