#include "Logic.h"
#include "FightInput.h"
#include "Hitbox.h"
#include "Box.h"
#include "Action.h"
#include <algorithm>
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

HCharacter HAction::character() const {
  return HCharacter(actions[h].character);
}

const Hitbox& HAction::collision() const {
  const std::optional<Hitbox>& b = actions[h].collision;
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

int HAction::damage() const {
  return actions[h].damage;
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
  return actions[h].isWalkOrIdle;
}

enum EAnimation HAction::animation() const {
  return actions[h].animation;
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
  check(h == 0);
  return characters[h].idle;
}

HAction HCharacter::walkForward() const {
  return characters[h].walkForward;
}

HAction HCharacter::walkBackward() const {
  return characters[h].walkBackward;
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

bool HCharacter::operator==(const HCharacter& b) const {
  return h == b.h;
}

bool HCharacter::operator!=(const HCharacter& b) const {
  return !(*this == b);
}

void Player::TryStartingNewAction(int frame, AFightInput& input, bool isOnLeft) {
  if (frame - actionStart > action.lockedFrames()) {
    std::optional<HAction> newActionO = input.action(action, isOnLeft, frame);
    if (newActionO.has_value()) {
      HAction newAction = newActionO.value();
      // don't interrupt current action if the new action is just an idle unless we are walking
      if (!((action.isWalkOrIdle() &&
             (newAction == action) &&
             (frame - actionStart <= action.animationLength())) ||
            (!action.isWalkOrIdle() &&
             (newAction == action.character().idle()) &&
             (frame - actionStart <= action.animationLength())))) {
        action = newAction;
        actionStart = frame;
      }
      // do update player direction regardless if we are merely
      // continuing walk/idle
      isFacingRight = isOnLeft;
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

// returns the amount of correction needed to move player out of the bound
float Player::collidesWithBoundary(float boundary, bool isRightBound, int targetFrame) {
  const Box& b = action.collision().at(targetFrame)->at(0);
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
  const Box &pb = p.action.collision().at(targetFrame)->at(0);
  const Box &qb = q.action.collision().at(targetFrame)->at(0);
  return qb.collisionExtent(pb, p.pos.Y, p.pos.Z, q.pos.Y, q.pos.Z, p.isFacingRight, q.isFacingRight);
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
    MYLOG(Display, "HandlePlayerBoundaryCollision: p1 collides with %s", doRightBoundary ? TEXT("right") : TEXT("left"));
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

  p1Input->init(maxRollback, 2, 2);
  p2Input->init(maxRollback, 2, 2);

  mode = LogicMode::Wait;
  frame = 0;
  reset(false);
}

void ALogic::reset(bool flipSpawns) {
  p1Input->reset();
  p2Input->reset();
  // construct initial frame
  Frame f (Player(flipSpawns ? rightStart : leftStart, HActionIdle), Player(flipSpawns ? leftStart : rightStart, HActionIdle));
  f.p1.isFacingRight = IsP1OnLeft(f);
  f.p2.isFacingRight = !IsP1OnLeft(f);
  frames.clear();
  frames.push(f);
}

void ALogic::setMode(enum LogicMode m) {
  mode = m;
  p1Input->setMode(m);
  p2Input->setMode(m);
}

void ALogic::preRound() {
  MYLOG(Display, "preRound");
  if (skipPreRound) {
    reset(false);
    beginRound();
  }
  else {
    setMode(LogicMode::Idle);
    inPreRound = true;
    roundStartFrame = ((frame+60)/15)*15;
    reset(false);
    if(OnPreRound.IsBound()) {
      OnPreRound.Broadcast();
    }
  }
}
void ALogic::beginRound() {
  MYLOG(Display, "beginRound");
  setMode(LogicMode::Fight);
  if(OnBeginRound.IsBound()) {
    OnBeginRound.Broadcast();
  }
}

void ALogic::endRound() {
  MYLOG(Display, "endRound");
  setMode(LogicMode::Idle);
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
  p1.pos += p1v;
  p2.pos += p2v;
  // recompute who is on left, useful in the case of a jumping cross
  // up
  isP1OnLeft = IsP1OnLeft(newFrame);

  // check for player-player collisions
  float collisionAdj = playerCollisionExtent(p1, p2, targetFrame);
  if (collisionAdj != 0.0) {
    float p1CollisionAdj = 0.5 * collisionAdj;
    float p2CollisionAdj = -0.5 * collisionAdj;
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
    bool p1Hit = false, p1Block = false;
    int p1Damage = 0;
    bool p2Hit = false, p2Block = false;
    int p2Damage = 0;
    if (collides(p1.action.hitbox(), p2.action.hurtbox(), newFrame, targetFrame) ||
        collides(p1.action.hitbox(), p2.action.collision(), newFrame, targetFrame)) {
      // hit p2
      MYLOG(Display, "P2 was hit");
      p2Hit = true;
      p2.hitstun = p1.action.lockedFrames() - (targetFrame-p1.actionStart);
      if (p2Input->isGuarding(!isP1OnLeft, targetFrame)){
        p2Block = true;
        // later we could make hitstun a property of an attack so that
        // we can control the frame advantage. For now, we set the
        // victim to recover one frame after the attacker on block
        p2.hitstun += 1;
      }
      else {
        p2Damage = p1.action.damage();
        // later we could make hitstun a property of an attack so that
        // we can control the frame advantage. For now, we set the
        // victim to recover three frames after the attacker on hit
        p2.hitstun += 3;
      }
      p2.health -= p2Damage;
    }
    if (collides(p1.action.hurtbox(), p2.action.hitbox(), newFrame, targetFrame) ||
        collides(p1.action.collision(), p2.action.hitbox(), newFrame, targetFrame)) {
      // hit p1
      MYLOG(Display, "P1 was hit");
      p1Hit = true;
      p1.hitstun = p2.action.lockedFrames() - (targetFrame-p2.actionStart);
      if (p1Input->isGuarding(isP1OnLeft, targetFrame)){
        p1Block = true;
        // later we could make hitstun a property of an attack so that
        // we can control the frame advantage. For now, we set the
        // victim to recover one frame after the attacker on block
        p1.hitstun += 1;
      }
      else {
        p1Damage = p2.action.damage();
        // later we could make hitstun a property of an attack so that
        // we can control the frame advantage. For now, we set the
        // victim to recover three frames after the attacker on hit
        p1.hitstun += 3;
      }
      p1.health -= p1Damage;
    }

    // TODO: should spawn special FX on hit/block here
    if (p1Hit) {
      if (p1Block) {
        p1.doBlockAction(targetFrame);
      }
      else {
        p1.doDamagedAction(targetFrame);
      }
      newFrame.hitPlayer = 1;
    }
    if (p2Hit) {
      if (p2Block) {
        p2.doBlockAction(targetFrame);
      }
      else {
        p2.doDamagedAction(targetFrame);
      }
      newFrame.hitPlayer = 2;
    }
    newFrame.hitstop = std::max(p1Damage, p2Damage)/5;
    if (p1Hit && p2Hit) {
      newFrame.hitPlayer = 0;
      // add the hitstop because we won't do real hitstop when ties
      // happen, only pushback
      p1.hitstun += newFrame.hitstop;
      p2.hitstun += newFrame.hitstop;
    }
  }
  else { // we are in hitstop
    // keep the attacking player frozen, do pushback, keep players in bounds
    if ((newFrame.hitPlayer == 1) || (newFrame.hitPlayer == 0)) {
      // do pushback
      p1.pos.Y += (isP1OnLeft ? -1 : 1) * 10;
    }
    if ((newFrame.hitPlayer == 2) || (newFrame.hitPlayer == 0)) {
      // do pushback
      p2.pos.Y += (!isP1OnLeft ? -1 : 1) * 10;
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
  if ((p1.health == 0) || (p2.health == 0)) {
    endRound();
    // do stuff on end fight; declare winner, display message, call
    // reset(roundNumber%2) and beginFight()
  }

  frames.push(newFrame);
}

void ALogic::FightTick() {
  MYLOG(Display, "FightTick");

  int latestInputFrame = std::max(p1Input->getCurrentFrame(), p2Input->getCurrentFrame());
  int targetFrame = std::max(latestInputFrame, frame+1);

  if (p1Input->needsRollback() || p2Input->needsRollback()) {
    MYLOG(Warning, "Rollback");
    int rollbackToFrame = std::min(p1Input->getNeedsRollbackToFrame(), p2Input->getNeedsRollbackToFrame());
    if ((1 + frame - rollbackToFrame) > maxRollback) {
      // exceeded maximum rollback. we do not have data old enough to
      // rollback, simulate the fight and guarantee consistency.
      MYLOG(Warning, "MAXIMUM ROLLBACK EXCEEDED!");
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
    //MYLOG(Display, "TICK %i!", frame);
    computeFrame(frame);
  }
}

// Called every frame
void ALogic::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  MYLOG(Display, "Tick");
  switch (mode) {
  case LogicMode::Idle:
    if (inPreRound && (frame == (roundStartFrame-1))) {
      inPreRound = false;
      beginRound();
    }
    ++frame;
    break;
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

enum EAnimation ALogic::playerAnimation(int playerNumber) {
  return getPlayer(playerNumber).action.animation();
}

int ALogic::playerFrame(int playerNumber) {
  return (frame - getPlayer(playerNumber).actionStart);
}
