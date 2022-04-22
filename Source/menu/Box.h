#pragma once

class Box {
  float x;
  float y;
  float xend;
  float yend;
public:
  Box(float x, float y, float xend, float yend): x(x), y(y), xend(xend), yend(yend) {}
  bool collides(const Box& b, float offsetax, float offsetay, float offsetbx, float offesetby) const;
};
