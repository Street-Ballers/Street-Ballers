#pragma once

#include "Box.h"
#include <vector>
#include <utility>

typedef std::pair<int, std::vector<Box>> hitbox_pair;

class Hitbox {
public:
  // At each frame, there are a set of hitboxes to check. The outer
  // vector orders the hitboxes by time. The pair is a pair of (end
  // frame, hitboxes).
  std::vector<hitbox_pair> boxes;

  static hitbox_pair make_pair(int endFrame, std::vector<Box> boxes);

  // constructor that takes a vector whos format is given above
  Hitbox(std::vector<hitbox_pair> boxes): boxes(boxes) {};

  // constructor that takes a single vector of Boxes. Constructs a
  // Hitbox where this vector is active at all frames.
  Hitbox(std::vector<Box> boxes);

  // just constructs an empty hitbox
  Hitbox(): boxes({}) {};

  const std::vector<Box>* at(int frame) const;

  // - b: other hitbox we are checking for collision with
  // - aframe: frame of our hitboxes to check for collision
  // - bframe: frame of b's hitboxes to check for collision
  bool collides(const Box& b, int aframe, int bframe, float offsetax, float offsetay, float offsetbx, float offsetby, bool aFacingRight, bool bFacingRight) const;

  // - b: other hitbox we are checking for collision with
  // - aframe: frame of our hitboxes to check for collision
  // - bframe: frame of b's hitboxes to check for collision
  bool collides(const Hitbox& b, int aframe, int bframe, float offsetax, float offsetay, float offsetbx, float offsetby, bool aFacingRight, bool bFacingRight) const;
};
