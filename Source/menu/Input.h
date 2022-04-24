#pragma once

#include "Action.h"
#include <optional>

// "Button" here includes directional input. They are relative to the
// character's orientation.
enum class Button {LP, HP, LK, HK, FORWARD, BACK, UP, DOWN, LEFT, RIGHT};

// ideally we'd only have one RingBuffer<T> class but unreal doesn't
// like templates and I don't want to figure out how to build it as an
// external library that can still be distributed to many platforms
// just yet.
class ButtonRingBuffer {
private:
  std::vector<std::optional<enum Button>> v;
  int n;
  int start;
  int end;

public:
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
class Input {
private:
  int n;

  // number of frames to "buffer" inputs. if there are no actions in
  // the latest frame to decode besides walking, then use inputs from
  // the latest frame within `buffer` frames away that results in an
  // action.
  int buffer;

  // artificial input delay. action() should decode an action based on
  // the inputs `delay` frames ago.
  int delay;

  // for now, just allow one button and direction at a time
  ButtonRingBuffer buttonHistory;
  ButtonRingBuffer directionHistory;

  bool is_button(const enum Button& b);
  // bool is_none(const Button& b);

  // return action using input `frame` frames ago as latest input
  HAction _action(HAction currentAction, int frame, bool isFacingRight);

public:
  Input(): n(0), buffer(0), delay(0) {};

  void init(int _maxRollback, int _buffer, int _delay);

  // When no button is pressed in a frame, this will be called with
  // an empty vector
  void buttonsPressed(const std::vector<const enum Button>& buttons);

  // this should probably just ignore everything except directional
  // inputs. we don't have to care when a button is released, except
  // when its a directional input.
  void buttonsReleased(const std::vector<const enum Button>& buttons);

  // Returns the currently decoded action
  HAction action(HAction currentAction, bool isFacingRight);
};
