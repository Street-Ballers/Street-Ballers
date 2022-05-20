#pragma once

// "Button" here includes directional input. They are relative to the
// character's orientation. This enum is overloaded in meaning.
enum class Button {
  LP=0, HP=1, LK=2, HK=3,
  UP=4, DOWN=5, LEFT=6, RIGHT=7,
  FORWARD, BACK,
  UPFORWARD, UPBACK, DOWNFORWARD, DOWNBACK, NEUTRAL,
  QCFP
};
