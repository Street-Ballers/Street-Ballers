// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Action.h"
#include <optional>
#include <vector>
#include "FightInput.generated.h"

// "Button" here includes directional input. They are relative to the
// character's orientation.
enum class Button {LP, HP, LK, HK, FORWARD, BACK, UP, DOWN, LEFT, RIGHT};

// ideally we'd only have one RingBuffer<T> class but unreal doesn't
// like templates and I don't want to figure out how to build it as an
// external library that can still be distributed to many platforms
// just yet.
class ButtonRingBuffer {
private:
  int n;
  int end;

public:
  std::vector<std::optional<enum Button>> v;
  void reserve(int size);

  void push(const std::optional<enum Button>& x);

  std::optional<enum Button>& last();

  std::optional<enum Button>& nthlast(int i);
};

// This class will decode input sequences and support replaying input
// in case of rollback. In the case that each move is triggered by a
// single button press, this is simply mapping the most recent button
// to a move. In the case of chorded moves or motion commands, we have
// to keep track of all buttons pressed over time.
UCLASS()
class MENU_API AFightInput : public AInfo {
  GENERATED_BODY()
private:
  int maxRollback;
  int n;

  // number of frames to "buffer" inputs. if there are no actions in
  // the latest frame to decode besides walking, then use inputs from
  // the latest frame within `buffer` frames away that results in an
  // action.
  int buffer;

  // artificial input delay. action() should decode an action based on
  // the inputs `delay` frames ago.
  int delay;

  int currentFrame;
  int needsRollbackToFrame;

  // for now, just allow one button and direction at a time
  ButtonRingBuffer buttonHistory;
  ButtonRingBuffer directionHistory;

  bool is_button(const enum Button& b);
  // bool is_none(const Button& b);

  // Make sure that AFightInput has some data for the new frame. We
  // will either do nothing or "predict" the input (assume nothing was
  // pressed or released).
  void ensureFrame(int targetFrame);

  // return action using input `frame` frames ago as latest input
  HAction _action(HAction currentAction, int frame, bool isOnLeft);

public:
  bool beginFight = false;

  // initialize all member variables
  void init(int _maxRollback, int _buffer, int _delay);

  // The player controller will call this function to say which
  // buttons were pressed and released on the given frame. frame is
  // the frame that the inputs should first appear. It is 1+ the frame
  // number stored in ALogic at the time that this function is called
  // by the player controller.
  void buttons(const std::vector<const enum Button>& buttonsPressed, const std::vector<const enum Button>& buttonsReleased, int targetFrame);


  // Returns the decoded action for the given targetFrame.
  HAction action(HAction currentAction, bool isOnLeft, int targetFrame);

  int getCurrentFrame();
  bool needsRollback();
  int getNeedsRollbackToFrame();
  void clearRollbackFlags();
};
