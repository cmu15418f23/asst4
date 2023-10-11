#ifndef COMMON_H_
#define COMMON_H_

#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct StartupOptions {
  int numIterations = 1;
  int numParticles = 5;
  float viewportRadius = 10.0f;
  float spaceSize = 10.0f;
  bool loadBalance = false;
  std::string outputFile;
  std::string inputFile;
};

struct StepParameters {
  float deltaTime = 0.2f;
  float cullRadius = 1.0f;
};

inline StepParameters getBenchmarkStepParams(float spaceSize) {
  StepParameters result;
  result.cullRadius = spaceSize / 4.0f;
  result.deltaTime = 0.2f;
  return result;
}

inline StartupOptions parseOptions(int argc, char *argv[]) {
  StartupOptions rs;
  for (int i = 1; i < argc; i++) {
    if (i < argc - 1) {
      if (strcmp(argv[i], "-i") == 0)
        rs.numIterations = atoi(argv[i + 1]);
      else if (strcmp(argv[i], "-s") == 0)
        rs.spaceSize = (float)atof(argv[i + 1]);
      else if (strcmp(argv[i], "-in") == 0)
        rs.inputFile = argv[i + 1];
      else if (strcmp(argv[i], "-n") == 0)
        rs.numParticles = atoi(argv[i + 1]);
      else if (strcmp(argv[i], "-v") == 0)
        rs.viewportRadius = (float)atof(argv[i + 1]);
      else if (strcmp(argv[i], "-o") == 0)
        rs.outputFile = argv[i + 1];
    }
    if (strcmp(argv[i], "-lb") == 0) {
      rs.loadBalance = true;
    }
  }
  return rs;
}

struct Vec2 {
  float x, y;
  Vec2(float vx = 0.0f, float vy = 0.0f) : x(vx), y(vy) {}
  static float dot(Vec2 v0, Vec2 v1) { return v0.x * v1.x + v0.y * v1.y; }
  float &operator[](int i) { return ((float *)this)[i]; }
  Vec2 operator*(float s) const { return Vec2(*this) *= s; }
  Vec2 operator*(Vec2 vin) const { return Vec2(*this) *= vin; }
  Vec2 operator+(Vec2 vin) const { return Vec2(*this) += vin; }
  Vec2 operator-(Vec2 vin) const { return Vec2(*this) -= vin; }
  Vec2 operator-() const { return Vec2(-x, -y); }
  Vec2 &operator+=(Vec2 vin) {
    x += vin.x;
    y += vin.y;
    return *this;
  }
  Vec2 &operator-=(Vec2 vin) {
    x -= vin.x;
    y -= vin.y;
    return *this;
  }
  Vec2 &operator=(float v) {
    x = y = v;
    return *this;
  }
  Vec2 &operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
  }
  Vec2 &operator*=(Vec2 vin) {
    x *= vin.x;
    y *= vin.y;
    return *this;
  }
  float length2() const { return x * x + y * y; }
  float length() const { return sqrt(length2()); }
};

struct Particle {
  int id;
  float mass;
  Vec2 position;
  Vec2 velocity;
};

// Do not modify this function.
inline Vec2 computeForce(const Particle &target, const Particle &attractor,
                         float cullRadius) {
  auto dir = (attractor.position - target.position);
  auto dist = dir.length();
  if (dist < 1e-3f)
    return Vec2(0.0f, 0.0f);
  dir *= (1.0f / dist);
  if (dist > cullRadius)
    return Vec2(0.0f, 0.0f);
  if (dist < 1e-1f)
    dist = 1e-1f;
  const float G = 0.01f;
  Vec2 force = dir * target.mass * attractor.mass * (G / (dist * dist));
  if (dist > cullRadius * 0.75f) {
    float decay = 1.0f - (dist - cullRadius * 0.75f) / (cullRadius * 0.25f);
    force *= decay;
  }
  return force;
}

inline Particle updateParticle(const Particle &pi, Vec2 force,
                               float deltaTime) {
  Particle result = pi;
  result.velocity += force * (deltaTime / pi.mass);
  result.position += result.velocity * deltaTime;
  return result;
}

// These functions are marked inline only because we want to define them in the
// header file.
inline bool loadFromFile(std::string fileName,
                         std::vector<Particle> &particles) {
  std::ifstream f(fileName);
  assert((bool)f && "Cannot open input file");

  std::string line;
  while (std::getline(f, line)) {
    Particle particle;
    std::stringstream sstream(line);
    std::string str;
    std::getline(sstream, str, ' ');
    particle.mass = (float)atof(str.c_str());
    std::getline(sstream, str, ' ');
    particle.position.x = (float)atof(str.c_str());
    std::getline(sstream, str, ' ');
    particle.position.y = (float)atof(str.c_str());
    std::getline(sstream, str, ' ');
    particle.velocity.x = (float)atof(str.c_str());
    std::getline(sstream, str, '\n');
    particle.velocity.y = (float)atof(str.c_str());
    particle.id = (int)particles.size();
    particles.push_back(particle);
  }
  return true;
}

inline void saveToFile(std::string fileName,
                       const std::vector<Particle> &particles) {
  std::ofstream f(fileName);
  assert((bool)f && "Cannot open output file");

  f << std::setprecision(9);
  for (auto p : particles) {
    f << p.mass << " " << p.position.x << " " << p.position.y << " "
      << p.velocity.x << " " << p.velocity.y << std::endl;
  }
  assert((bool)f && "Failed to write to output file");
}

#endif
