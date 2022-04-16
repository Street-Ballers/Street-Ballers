#include "Logic.h"
#include "Input.h"
#include "Hitbox.h"
#include "Box.h"
#include "Action.h"

void Input::buttonPressed(const enum Button& button) {

}

void Input::buttonReleased(const enum Button& button) {

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

HAction::HAction(int h): _h(h) {};
HAction::HAction(): HAction(-1) {};

const Hitbox& HAction::hitbox(const std::vector<Action>& a) {
  return a[_h].hitbox;
}

const Hitbox& HAction::hurtbox(const std::vector<Action>& a) {
  return a[_h].hurtbox;
}

int HAction::lockedFrames(const std::vector<Action>& a) {
  return a[_h].lockedFrames;
}

void RingBuffer::reserve(int size) {
  n = size;
  v = new Frame[n];
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
  delete[] v;
}

// Sets default values for this component's properties
ALogic::ALogic(): frame(0)
{
  // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
  // off to improve performance if you don't need them.
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;

  frames.reserve(maxRollback);
}

// Called when the game starts or when spawned
void ALogic::BeginPlay()
{
  Super::BeginPlay();
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

  // check hitboxes, compute damage (if anyone is in an attacking
  // action). Don't forget the case of ties.

  // check if anyone died, and if so, start new round or end game
  // and stuff

  frames.push(newFrame);

  // just demonstrating ability to move a character from this class
  character1->SetActorLocation(FVector(0.0, (double) ((frame)%500), 0.0), false, nullptr, ETeleportType::None);
}
