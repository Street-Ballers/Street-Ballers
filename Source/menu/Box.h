#pragma once

class Box {
  float x;
  float y;
  float xend;
  float yend;
public:
  constexpr Box(float x, float y, float xend, float yend): x(x), y(y), xend(xend), yend(yend) {};

  // convenience function for making collision boxes, which are often
  // centered at x=0 and whos bottom is at y=0
  static constexpr Box make_centeredx(float width, float height) {
    return Box(-1*width/2, 0, width/2, height);
  }

  bool collides(const Box& b, float offsetax, float offsetay, float offsetbx, float offesetby, bool aFacingRight) const;
};
