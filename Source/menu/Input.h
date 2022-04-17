#pragma once

#include "Action.h"
#include <optional>

// "Button" here includes directional input
enum class Button {LP, HP, LK, HK, LEFT, RIGHT, UP, DOWN};

// This class will decode input sequences. In the case that each move
// is triggered by a single button press, this is simply mapping the
// most recent button to a move. In the case of chorded moves or
// motion commands, we have to keep track of all buttons pressed over
// time.
class Input {
public:
  void buttonPressed(HAction currentAction, const enum Button& button);

  void buttonReleased(HAction currentAction, const enum Button& button);

  // Returns the currently decoded action
  std::optional<HAction> action();
};
