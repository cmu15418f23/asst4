#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "common.h"
#include <memory>

// NOTE: Do not remove or edit funcations and variables in this class definition
class QuadTreeNode {
public:
  bool isLeaf = 0;

  // four child nodes are stored in following order:
  //  x0, y0 --------------- x1, y0
  //    |           |           |
  //    |children[0]|children[1]|
  //    | ----------+---------  |
  //    |children[2]|children[3]|
  //    |           |           |
  //  x0, y1 ----------------- x1, y1
  // where x0 < x1 and y0 < y1.

  std::unique_ptr<QuadTreeNode> children[4];

  std::vector<Particle> particles;
};

inline float boxPointDistance(Vec2 bmin, Vec2 bmax, Vec2 p) {
  float dx = fmaxf(fmaxf(bmin.x - p.x, p.x - bmax.x), 0.0f);
  float dy = fmaxf(fmaxf(bmin.y - p.y, p.y - bmax.y), 0.0f);
  return sqrt(dx * dx + dy * dy);
}

// NOTE: Do not remove or edit funcations and variables in this class definition
class QuadTree {
public:
  std::unique_ptr<QuadTreeNode> root = nullptr;
  // the bounds of all particles
  Vec2 bmin, bmax;

  void getParticles(std::vector<Particle> &particles, Vec2 position,
                    float radius) const {
    particles.clear();
    getParticlesImpl(particles, root.get(), bmin, bmax, position, radius);
  }

  static void buildQuadTree(const std::vector<Particle> &particles,
                            QuadTree &tree) {
    // find bounds
    Vec2 bmin(1e30f, 1e30f);
    Vec2 bmax(-1e30f, -1e30f);

    for (auto &p : particles) {
      bmin.x = fminf(bmin.x, p.position.x);
      bmin.y = fminf(bmin.y, p.position.y);
      bmax.x = fmaxf(bmax.x, p.position.x);
      bmax.y = fmaxf(bmax.y, p.position.y);
    }

    // build nodes
    tree.bmin = bmin;
    tree.bmax = bmax;

    tree.root = buildQuadTreeImpl(particles, bmin, bmax);
  }

private:
  static std::unique_ptr<QuadTreeNode>
  buildQuadTreeImpl(const std::vector<Particle> &particles, Vec2 bmin,
                    Vec2 bmax) {
    // TODO: paste your sequential implementation in Assignment 3 here.
    // (or you may also rewrite a new version)
    return nullptr;
  }

  static void getParticlesImpl(std::vector<Particle> &particles,
                               QuadTreeNode *node, Vec2 bmin, Vec2 bmax,
                               Vec2 position, float radius) {
    if (node->isLeaf) {
      for (auto &p : node->particles)
        if ((position - p.position).length() < radius)
          particles.push_back(p);
      return;
    }
    Vec2 pivot = (bmin + bmax) * 0.5f;
    Vec2 size = (bmax - bmin) * 0.5f;
    for (int i = 0; i < 4; i++) {
      Vec2 childBMin;
      childBMin.x = (i & 1) ? pivot.x : bmin.x;
      childBMin.y = ((i >> 1) & 1) ? pivot.y : bmin.y;
      Vec2 childBMax = childBMin + size;
      if (boxPointDistance(childBMin, childBMax, position) <= radius)
        getParticlesImpl(particles, node->children[i].get(), childBMin,
                         childBMax, position, radius);
    }
  }
};

#endif
