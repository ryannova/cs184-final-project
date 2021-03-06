#include <nanogui/nanogui.h>

#include "../clothMesh.h"
#include "../misc/sphere_drawing.h"
#include "sphere.h"

#include <glad/glad.h>

using namespace nanogui;
using namespace CGL;

#define G 6.67408e-11
double Sphere::sphere_factor = 0;
double Sphere::gravity_margin = 0;
double Sphere::radiusFactor = 0;

void Sphere::collide(PointMass &pm) {
  // TODO (Part 3): Handle collisions with spheres.
  // FIXME: Check if inside sphere val < r^2
  double x,y,z;
  x = pow(pm.position.x - origin.x, 2);
  y = pow(pm.position.y - origin.y, 2);
  z = pow(pm.position.z - origin.z, 2);
  double val = x + y + z;
  if (val <= radius2) {
    Vector3D dir, normDir, tanPoint, correct;
    dir = pm.position - origin;
    normDir = dir.unit();
    tanPoint = normDir * radius + origin;
    correct = tanPoint - pm.last_position;
    pm.position = pm.last_position + (1.f - friction) * correct;
  }
}

Vector3D Sphere::gravity(Sphere &other_sphere) {
  Vector3D dir = other_sphere.pm.position - pm.position;
  double r = dir.norm();
  dir.normalize();
  // Divide by r once to normalize dir, then twice more for the gravitation equation
  return dir * G * mass * other_sphere.mass / (r * r);
}

void Sphere::add_force(Vector3D force) {
  pm.forces += force;
}

void Sphere::verlet(double delta_t) {
  Vector3D new_pos = pm.position + velocity + (pm.forces / mass);

//    std::cout << "forces: " << pm.forces << "\n";
//    std::cout << "velocity: " << velocity << "\n";
//    std::cout << "mass: " << mass << "\n";
//    std::cout << "forces / mass: " << pm.forces / mass << "\n";
//    std::cout << "Accleration factor: " << (pm.forces / mass) / 2.f << "\n";
//
//    std::cout << "pos: " << pm.position << "\n";
//    std::cout << "new_pos: " << new_pos << "\n";
//    std::cout << "\n\n\n";

  //pm.last_position = pm.position;
  velocity = new_pos - pm.position;
  pm.position = new_pos;
  pm.forces = Vector3D();
}

void Sphere::render(GLShader &shader, bool is_paused) {
  // We decrease the radius here so flat triangles don't behave strangely
  // and intersect with the sphere when rendered
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, *texture);

//  m_sphere_mesh.draw_sphere(shader, pm.position / sphere_factor, radius / radiusFactor);
    m_sphere_mesh.draw_sphere(shader, pm.position / sphere_factor, log(radius));
  if (!is_paused) {
      if (track.size() > 2 && addTrack) {
          this->isTrackEnd(track.front(), (track.at(1) - track.at(0)).norm());
      }

      if (addTrack) {
          track.push_back(pm.position);
      }
  }
}
void Sphere::trail(GLShader &shader, std::vector<Vector3D> trail) {
    if (trail.size() >= 2) {
        int num_lines = track.size() * 2;
        MatrixXd positions(4, num_lines * 2);

        // Draw springs as lines

        int si = 0;
//        Vector3D prev;

        for (int i = 0; i < trail.size() - 1; i++) {
            Vector3D pa = trail[i] / sphere_factor;
            Vector3D pb = trail[i+1] / sphere_factor;
//            if (isnan(pa.x) || isnan(pa.y) || isnan(pa.z) ||
//                isnan(pb.x) || isnan(pb.y) || isnan(pb.z)) {
//                break;
//            }
//            prev = pa;

            positions.col(si) << pa.x, pa.y, pa.z, 1.0;
            positions.col(si + 1) << pb.x, pb.y, pb.z, 1.0;

            si += 2;
        }

        // shader.setUniform("u_color", nanogui::Color(1.0f, 1.0f, 1.0f, 1.0f), false);
        shader.uploadAttrib("in_position", positions, false);
        // Commented out: the wireframe shader does not have this attribute
        //shader.uploadAttrib("in_normal", normals);

        shader.drawArray(GL_LINES, 0, num_lines);
#ifdef LEAK_PATCH_ON
        shader.freeAttrib("in_position");
#endif
    }
}

std::vector<Vector3D> Sphere::getTrack() {
    return track;
}

void Sphere::isTrackEnd(Vector3D track_start, double distance) {
    if (abs((track_start - pm.position).norm()) < abs(distance)) {
        addTrack = false;
    }

}

bool Sphere::getTrackDone() {
    return !addTrack;
}

Vector3D Sphere::getInitOrigin() {
    return startOrigin;
}

Vector3D Sphere::getInitVelocity() {
    return velocity;
}

double Sphere::getRadius() {
    return radius;
}

long double Sphere::getMass() {
    return mass;
}

string Sphere::getTexFile() {
    return tex_file;
}

void Sphere::reset() {
    this->origin = startOrigin;
    this->velocity = startVelocity;
    track.clear();
    addTrack = true;
    pm.position = pm.start_position;
    pm.last_position = pm.start_position;
}