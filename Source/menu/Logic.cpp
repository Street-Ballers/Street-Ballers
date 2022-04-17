#include "Logic.h"
#include "Input.h"
#include "Hitbox.h"
#include "Box.h"
#include "Action.h"

void Input::buttonPressed(HAction currentAction, const enum Button& button) {

}

void Input::buttonReleased(HAction currentAction, const enum Button& button) {

}

// Returns the currently decoded action
std::optional<HAction> Input::action() {
  return {};
}

bool Box::collides(const Box& b) {
  return false;
}

std::vector<Box> Hitbox::at(int frame) {
  // scan through boxes for the last pair that starts at or before
  // frame, and return the corresponding vector
  return {};
}

// - b: other hitbox we are checking for collision with
// - aframe: frame of our hitboxes to check for collision
// - bframe: frame of b's hitboxes to check for collision
bool Hitbox::collides(const Hitbox& b, int aframe, int bframe) {
  return false;
}

HAction::HAction(int h): h(h) {};
HAction::HAction(): HAction(-1) {};

const Action HAction::actions[] = {
  [HActionIdleI] = Action(Hitbox({}), Hitbox({}), 0),
  [HActionWalkForwardI] = Action(Hitbox({}), Hitbox({}), 0, FVector(0.0, 4.0, 0.0)),
  [HActionStPI] = Action(Hitbox({}), Hitbox({}), 0),
};

const Hitbox& HAction::hitbox() {
  return actions[h].hitbox;
}

const Hitbox& HAction::hurtbox() {
  return actions[h].hurtbox;
}

int HAction::lockedFrames() {
  return actions[h].lockedFrames;
}

FVector HAction::velocity() {
  return actions[h].velocity;
}

void Player::buttonPressed(const enum Button& button) {
  input.buttonPressed(action, button);
}

void Player::buttonReleased(const enum Button& button) {
  input.buttonReleased(action, button);
}

void RingBuffer::reserve(int size) {
  n = size;
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
  free(v);
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
}

// Called when the game starts or when spawned
void ALogic::BeginPlay()
{
  Super::BeginPlay();

  // construct initial frame
  Frame f (Player(leftStart, HActionWalkForward), Player(rightStart, HActionIdle));
  frames.push(f);
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

  // if a button was pressed or released, send it to p1Input/p2Input

  // If the player can act and there is a new action waiting, then
  // start the new action
  // if (frame - lastFrame.p1.actionStart >= lastFrame.p1.action.lockedFrames({})) {
  //   // if p1Input.action() != none, then newFrame.p1Action =
  //   // p2Input.action() and newFrame.p1ActionStart = frame
  // }
  // same for p2

  // compute player positions (if they are in a moving action). This
  // includes checking collision boxes and not letting players walk
  // out of bounds.
  int p1Direction = (newFrame.p1.pos.Y > newFrame.p2.pos.Y) ? -1 : 1;
  int p2Direction = -1*p1Direction;
  newFrame.p1.pos += p1Direction*newFrame.p1.action.velocity();
  newFrame.p2.pos += p2Direction*newFrame.p2.action.velocity();
  p1Direction = (newFrame.p1.pos.Y > newFrame.p2.pos.Y) ? -1 : 1;
  p2Direction = -1*p1Direction;

  // check hitboxes, compute damage. Don't forget the case of ties.

  // check if anyone died, and if so, start new round or end game and
  // stuff. when in online multiplayer, this should also wait for both
  // clients to be at a consistent state.

  frames.push(newFrame);

  // just demonstrating ability to move a character from this class
  character1->SetActorLocation(newFrame.p1.pos, false, nullptr, ETeleportType::None);
  character2->SetActorLocation(newFrame.p2.pos, false, nullptr, ETeleportType::None);
}
