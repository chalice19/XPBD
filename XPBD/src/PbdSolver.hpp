// ----------------------------------------------------------------------------
// PbdSolver.hpp
//
//  Created on: 07 Jul 2021
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: Position Based Dynamics Solver (Do not distribute!)
//
// Copyright 2021-2023 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

#ifndef _PBDSOLVER_HPP_
#define _PBDSOLVER_HPP_

#include <cmath>
#include <set>
#include <map>
#include <glm/glm.hpp>
#include <memory>
#include <utility>
#include <random>
#include <chrono>

#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "typedefs.hpp"
#include "Mesh.h"

struct Constraint {
  virtual void project(std::vector<glm::vec3> &x, std::vector<glm::vec3> &x_last, const std::vector<tReal> &w, tReal dt) = 0;

  bool is_zero(tReal x) {
    return (x <= 1e-5) && (x >= -1e-5);
  }

  void reset() {
    _lambda = 0;
  }

  tReal _lambda;                // Lagrangian multiplyer
  tReal _compliance;            // inverse stiffness
  tReal _damp_coef;
};

struct ConstraintAttach : public Constraint {
  explicit ConstraintAttach(const tUint i, const glm::vec3 &p) :
    _i(i), _p(p) {}

  virtual void project(std::vector<glm::vec3> &x, std::vector<glm::vec3> &x_last, const std::vector<tReal> &w, tReal dt)
  {
    x[_i] = _p;
  }

  tUint _i;                     // vertex id
  glm::vec3 _p;                 // fixed position
};

struct ConstraintStretch : public Constraint {
  explicit ConstraintStretch(
    const tUint i, const tUint j, const tReal d, const tReal k, const tReal damp) :
    _i(i), _j(j), _d(d) {
      _lambda = 0.f;
      _compliance = k;
      _damp_coef = damp;
    }

  virtual void project(std::vector<glm::vec3> &x, std::vector<glm::vec3> &x_last, const std::vector<tReal> &w, tReal dt)
  {
    glm::vec3 diff = x[_i] - x[_j];
    tReal dist = glm::length(diff);

    if (is_zero(dist - _d)) {
      return;
    }

    tReal compliance_tilda = _compliance / (dt * dt);
    tReal gamma = compliance_tilda * _damp_coef * dt;

    glm::vec3 n = diff / dist;
    glm::vec3 vel1 = x[_i] - x_last[_i];
    glm::vec3 vel2 = x[_j] - x_last[_j];

    tReal damp_term = gamma * (glm::dot(n, vel1) + glm::dot(-n, vel2));

    tReal dlambda = (-(dist -_d) - compliance_tilda * _lambda - damp_term) /
                    ((1 + gamma) * (w[_i] + w[_j]) + compliance_tilda);

    x[_i] += n * w[_i] * dlambda;
    x[_j] += -n * w[_j] * dlambda;

    _lambda += dlambda;
  }

  tUint _i, _j;                 // indices of two vertices
  tReal _d;                     // initial length
};

struct ConstraintBend : public Constraint {
  explicit ConstraintBend(
    const tUint i1, const tUint i2, const tUint i3, const tUint i4,
    const tReal phi0, const tReal k, const tReal damp) :
    _i1(i1), _i2(i2), _i3(i3), _i4(i4), _phi0(phi0) {
      _lambda = 0.f;
      _compliance = k;
      _damp_coef = damp;
    }

  virtual void project(std::vector<glm::vec3> &x, std::vector<glm::vec3> &x_last, const std::vector<tReal> &w, tReal dt)
  {
    const glm::vec3 p2 = x[_i2] - x[_i1];
    const glm::vec3 p3 = x[_i3] - x[_i1];
    const glm::vec3 p4 = x[_i4] - x[_i1];
    const glm::vec3 n1 = glm::normalize(glm::cross(p2, p3));
    const glm::vec3 n2 = glm::normalize(glm::cross(p2, p4));
    const tReal p2xp3_len = glm::length(glm::cross(p2, p3)) + 1e-5;
    const tReal p2xp4_len = glm::length(glm::cross(p2, p4)) + 1e-5;

    if (is_zero(glm::length(n1)) || is_zero(glm::length(n2))) {
      return;
    }

    const tReal d = glm::clamp(glm::dot(n1, n2), -1.f, 1.f);
    const tReal phi = std::acos(d);

    if (is_zero(phi - _phi0) || is_zero(1 - d * d)) {
      return;
    }

    const glm::vec3 q3 = (glm::cross(p2, n2) + glm::cross(n1, p2) * d) / p2xp3_len;
    const glm::vec3 q4 = (glm::cross(p2, n1) + glm::cross(n2, p2) * d) / p2xp4_len;
    const glm::vec3 q2 = -(glm::cross(p3, n2) + glm::cross(n1, p3) * d) / p2xp3_len
                         -(glm::cross(p4, n1) + glm::cross(n2, p4) * d) / p2xp4_len;
    const glm::vec3 q1 = -q2 - q3 - q4;

    tReal weighted_sum = 1e-6;
    weighted_sum += w[_i1] * glm::dot(q1, q1);
    weighted_sum += w[_i2] * glm::dot(q2, q2);
    weighted_sum += w[_i3] * glm::dot(q3, q3);
    weighted_sum += w[_i4] * glm::dot(q4, q4);
    weighted_sum /= (1. - d * d);

    tReal compliance_tilda = _compliance / (dt * dt);
    tReal gamma = compliance_tilda * _damp_coef * dt;

    glm::vec3 vel1 = x[_i1] - x_last[_i1];
    glm::vec3 vel2 = x[_i2] - x_last[_i2];
    glm::vec3 vel3 = x[_i3] - x_last[_i3];
    glm::vec3 vel4 = x[_i4] - x_last[_i4];
    tReal denom = sqrt(1 - d * d);
    tReal damp_term = 0.;
    damp_term += glm::dot(q1, vel1);
    damp_term += glm::dot(q2, vel2);
    damp_term += glm::dot(q3, vel3);
    damp_term += glm::dot(q4, vel4);
    damp_term *= gamma / denom;

    tReal dlambda = (_phi0 - phi - compliance_tilda * _lambda - damp_term) /
                    ((1 + gamma) * weighted_sum + compliance_tilda);

    x[_i1] += w[_i1] * dlambda * q1 / denom;
    x[_i2] += w[_i2] * dlambda * q2 / denom;
    x[_i3] += w[_i3] * dlambda * q3 / denom;
    x[_i4] += w[_i4] * dlambda * q4 / denom;

    _lambda += dlambda;
  }

  tUint _i1, _i2, _i3, _i4;          // indices of vertices forming two adjacent triangles
  tReal _phi0;                       // initial angle
};


class PbdSolver {
public:
  explicit PbdSolver(
    const tUint num_solve=20,
    const tReal k_stretch=1e-9, const tReal k_bend=10, const tReal k_damp=0.0f,
    const glm::vec3 &gravity=glm::vec3(0.f, -9.8f, 0.f)) :
    _g(gravity), _step(0), _sim_t(0.0f),
    _Ns(num_solve), _kStretch(k_stretch), _kBend(k_bend), _kDamp(k_damp) {}
  virtual ~PbdSolver() {}

  void initSim(const Mesh &mesh)
  {
    _step = 0;
    _sim_t = 0.0f;

    _x = mesh.vertexPositions();
    _x_next = mesh.vertexPositions();
    _idx = mesh.triangleIndices();
    _vertex_number = _x.size();

    // TODO - done: initialize physical variables _v, _f, _w

    for (int i = 0; i < _vertex_number; ++i) {
      _w.push_back(1.0);
      tReal m = 1.0 / _w[i];
      _v.push_back(glm::vec3(0.0));
      _f.push_back(glm::vec3(m * _g[0], m * _g[1], m * _g[2]));
    }


    // create constraints:

    // 1. edge-triangle information

    std::set<std::set<tUint>> connections;                            // list of edges (pair of indexes of the beginning and ending)
    std::map<std::set<tUint>, std::vector<tUint>> tri_neighbors;      // map from edge to the index of the opposite vertice of the triangle it belongs to
                                                                      // assuming each edge would belong to no more than 2 triangles

    for (auto& triangle: _idx) {
      for (int i = 0; i < 3; i += 1) {
        std::set<tUint> edge = {triangle[i], triangle[(i + 1) % 3]};
        connections.insert(edge);
        tri_neighbors[edge].push_back(triangle[(i + 2) % 3]);
      }
    }

    // 2. attachments

    // in one corner
    // for (int i = 0; i < 15; ++i) {
    //   for (int j = 0; j < 3; ++j) {
    //     _constraints.push_back(std::make_shared<ConstraintAttach>(ConstraintAttach(30 * j + i, _x[30 * j + i])));
    //     _w[i + 30 * j] = 0.f;
    //   }
    // }

    // only two corner points

    // glm::vec3 constr_pos = _x[0];
    // _constraints.push_back(std::make_shared<ConstraintAttach>(ConstraintAttach(0, constr_pos)));
    // _constraints.push_back(std::make_shared<ConstraintAttach>(ConstraintAttach(420, _x[420])));
    // _constraints.push_back(std::make_shared<ConstraintAttach>(ConstraintAttach(435, _x[435])));
    // _w[0] = 0.f;
    // _w[420] = 0.f;
    // _w[435] = 0.f;

    // a table
    for (int i = 0; i < 16; ++i) {
      for (int j = 0; j < 9; ++j) {
        int index = 30 * (3 + j) + i + 7;
        _constraints.push_back(std::make_shared<ConstraintAttach>(ConstraintAttach(index, _x[index])));
        _w[index] = 0.f;
      }
    }

    // 3. stretch

    for (auto& edge : connections) {
      tUint i = *edge.begin();
      tUint j = *(++edge.begin());
      
      if (_w[i] == 0 && _w[j] == 0) {
        continue;
      }

      tReal len = glm::length(_x[i] - _x[j]);
      _constraints.push_back(std::make_shared<ConstraintStretch>(ConstraintStretch(i, j, len, _kStretch, 0.9)));
    }

    // 4. bend

    for (auto& edge : connections) {
      if (tri_neighbors[edge].size() == 2) {

        // PBD bend:
        // base edge begin and end
        int i1 = *edge.begin();
        int i2 = *(++edge.begin());

        // points that belong to the same triangle as the edge
        int i3 = *tri_neighbors[edge].begin();
        int i4 = *(++tri_neighbors[edge].begin());

        if (_w[i1] == 0 && _w[i2] == 0 && _w[i3] == 0 && _w[i4] == 0) {
          continue;
        }

        const glm::vec3 p2 = _x[i2] - _x[i1];
        const glm::vec3 p3 = _x[i3] - _x[i1];
        const glm::vec3 p4 = _x[i4] - _x[i1];
        const glm::vec3 n1 = glm::normalize(glm::cross(p2, p3));
        const glm::vec3 n2 = glm::normalize(glm::cross(p2, p4));
        const tReal phi_0 = std::acos(glm::dot(n1, n2));

        _constraints.push_back(std::make_shared<ConstraintBend>(ConstraintBend(i1, i2, i3, i4, phi_0, _kBend, 0.05)));


        // simplified bend:

        // int i = *tri_neighbors[edge].begin();
        // int j = *(++tri_neighbors[edge].begin());
        // tReal len = glm::length(_x[i] - _x[j]);
        // _constraints.push_back(std::make_shared<ConstraintStretch>(ConstraintStretch(i, j, len, _kBend, _kDamp)));

      } else {
        // delete edges that belong to only one triangle or not good once
        tri_neighbors.erase(edge);
      }
    }
  }

  void updateMesh(Mesh &mesh)
  {
    mesh.vertexPositions() = _x;
    mesh.recomputePerVertexNormals();
  }

  void step(const tReal dt)
  {
    // main solver routine

    for (int i = 0; i < _vertex_number; ++i) {
      _v[i] += dt * _f[i] * _w[i];
      _x_next[i] = _x[i] + dt * _v[i];

      // colision constraints can be here
    }

    for (auto constraint : _constraints) {
      constraint->reset();
    }

    for (int i = 0; i < _Ns; ++i) {
      for (auto constraint : _constraints) {
        constraint->project(_x_next, _x, _w, dt);
      } 
    }

    for (auto& x : _x_next) {
      x[1] = glm::clamp(x[1], 0.0001f - 1.f, 1.f);
    }

    for (int i = 0; i < _vertex_number; ++i) {
      _v[i] = (_x_next[i] - _x[i]) / dt;
      _x[i] = _x_next[i];
    }

    ++_step;
    _sim_t += dt;
  }

private:
  std::vector<glm::vec3> _x;    // position
  std::vector<glm::vec3> _x_next;    // position
  std::vector<glm::vec3> _v;    // velocity
  std::vector<glm::vec3> _f;    // force
  std::vector<glm::uvec3> _idx; // indices
  std::vector<tReal> _w;        // mass inverse

  tUint _vertex_number;

  std::vector< std::shared_ptr<Constraint> > _constraints; // constraints

  // simulation parameters
  glm::vec3 _g;                 // gravity
  tUint _step;                  // step count
  tReal _sim_t;                 // simulation time

  // PBD solver parameters
  tUint _Ns;                       // solver iterations
  tReal _kStretch, _kBend, _kDamp; // stiffness coefficients
};

#endif  /* _PBDSOLVER_HPP_ */
