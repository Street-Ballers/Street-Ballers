#pragma once

#include "Box.h"
#include <vector>
#include <utility>

class Hitbox {
public:
  // At each frame, there are a set of hitboxes to check. The outer
  // vector orders the hitboxes by time. The pair is a pair of (end
  // frame, hitboxes).
  std::vector<std::pair<int, std::vector<Box>>> boxes;

  Hitbox(std::vector<std::pair<int, std::vector<Box>>> boxes): boxes(boxes) {};

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
