#include "collide.hpp"

#include <initializer_list>
#include <algorithm>
#include <iostream>


//Check if two AABBs overlap:
// (useful for early-out code)
bool collide_AABB_vs_AABB(
	glm::vec3 const &a_min, glm::vec3 const &a_max,
	glm::vec3 const &b_min, glm::vec3 const &b_max
) {
	return !(
		a_min.x > b_max.x || b_min.x > a_max.x ||
		a_min.y > b_max.y || b_min.y > a_max.y ||
		a_min.z > b_max.z || b_min.z > a_max.z
	);
}

//helper: normalize but don't return NaN:
glm::vec3 careful_normalize(glm::vec3 const &in) {
	glm::vec3 out = glm::normalize(in);
	//if 'out' ended up as NaN (e.g., because in was a zero vector), reset it:
	if (!(out.x == out.x)) out = glm::vec3(1.0f, 0.0f, 0.0f);
	return out;
}

//helper: ray vs sphere:
bool collide_ray_vs_sphere(
	glm::vec3 const &ray_start, glm::vec3 const &ray_direction,
	glm::vec3 const &sphere_center, float sphere_radius,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out) {

	// if ray is travelling away from sphere, don't intersect:
	// if (glm::dot(ray_start - sphere_center, ray_direction) >= 0.0f) return false;

	float t = 1.0f;
	if (collision_t) {
		t = std::min(t, *collision_t);
		if (t <= 0.0f) return false;
	}

  glm::vec3 ray_start_to_sphere = ray_start - sphere_center;

	// Solve a quadratic equation to find out:
	float a = glm::dot(ray_direction, ray_direction);
	float b = 2.0f * glm::dot(ray_start_to_sphere, ray_direction);
	float c = glm::dot(ray_start_to_sphere, ray_start_to_sphere) - sphere_radius * sphere_radius;

	//this is the part of the quadratic formula under the radical:
	float d = b * b - 4.0f * a * c;
	if (d < 0.0f) return false;
	d = std::sqrt(d);

  //intersects between t0 and t1:
	float t0 = (-b - d) / (2.0f * a);
	float t1 = (-b + d) / (2.0f * a);

	if (t1 < 0.0f || t0 > t) return false;

  t0 = glm::max(t0, 0.0f);
  glm::vec3 at = ray_start + t0 * ray_direction;;

  if (collision_t) *collision_t = t0;
  if (collision_at) *collision_at = at;
  if (collision_out) *collision_out = careful_normalize(at - sphere_center);

  return true;

}

bool collide_swept_sphere_vs_point(
	glm::vec3 const &sphere_from, glm::vec3 const &sphere_to, float sphere_radius,
	glm::vec3 const &point,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out
) {
	bool collided = collide_ray_vs_sphere(sphere_from, sphere_to - sphere_from, point, sphere_radius, collision_t, nullptr, collision_out);
	if (collided && collision_at) *collision_at = point;
	return collided;
}

bool collide_ray_vs_cylinder(glm::vec3 ray_start, glm::vec3 ray_direction,
		glm::vec3 cylinder_a, glm::vec3 cylinder_b, float radius,
		float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out
) {

	glm::vec3 cy_axis = cylinder_b - cylinder_a;

	float cy_height = glm::dot(cy_axis, cy_axis);

	float t0 = 0.0;
	float t1 = 1.0;

	float dot_from = glm::dot(ray_start - cylinder_a, cy_axis);
	float dot_to = glm::dot(ray_start + ray_direction - cylinder_a, cy_axis);

	if (dot_from < 0.0f) {
		if (dot_to <= dot_from) return false;
		t0 = (0.0f - dot_from) / (dot_to - dot_from);
	}
	if (dot_from > cy_height) {
		if (dot_to >= dot_from) return false;
		t0 = (cy_height - dot_from) / (dot_to - dot_from);
	}
	if (dot_to < 0.0f) {
		if (dot_from <= dot_to) return false;
		t1 = (0.0f - dot_from) / (dot_to - dot_from);
	}
	if (dot_to > cy_height)	{
		if (dot_from >= dot_to) return false;
		t1 = (cy_height - dot_from) / (dot_to - dot_from);
	}

	glm::vec3 projected_pt = cylinder_a + dot_from * cy_axis / cy_height;
	glm::vec3 projected_dir = (dot_to - dot_from) * cy_axis / cy_height;

	float t = t1 - t0;

	if (collide_ray_vs_sphere(
		ray_start - projected_pt + t0 * (ray_direction - projected_dir),
		ray_direction - projected_dir, glm::vec3(0.0f), radius,
		&t, nullptr, nullptr
  )) {
    t += t0;
    glm::vec3 out = careful_normalize(ray_start - projected_pt + t * (ray_direction - projected_dir));
		if (collision_t) *collision_t = t;
		if (collision_out) *collision_out = out;
		if (collision_at) *collision_at = ray_start + t * ray_direction - radius * out;
		return true;
	}

	return false;
}

bool collide_swept_sphere_vs_swept_sphere(
	glm::vec3 const &sphere0_from, glm::vec3 const &sphere0_to, float sphere0_radius,
	glm::vec3 const &sphere1_from, glm::vec3 const &sphere1_to, float sphere1_radius,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out
) {
	glm::vec3 sphere1_dir = sphere1_to - sphere1_from;
	glm::vec3 dir_sum = sphere0_to - sphere0_from - sphere1_dir;
	float t;
	glm::vec3 at;
	bool collided = collide_ray_vs_sphere(sphere0_from, sphere0_from + dir_sum, sphere1_from, sphere0_radius + sphere1_radius, &t, &at, collision_out);
	if (collided) {
		if (collision_t) *collision_t = t;
		if (collision_at) {
			at += t * sphere1_dir;
			*collision_at = at;
		}
	}
	return collided;

}
bool collide_swept_sphere_vs_triangle(
	glm::vec3 const &sphere_from, glm::vec3 const &sphere_to, float sphere_radius,
	glm::vec3 const &triangle_a, glm::vec3 const &triangle_b, glm::vec3 const &triangle_c,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out
) {

	float t = 1.0f;
	if (collision_t) {
		t = std::min(t, *collision_t);
		if (t <= 0.0f) return false;
	}
	glm::vec3 perp = glm::cross(triangle_b-triangle_a, triangle_c-triangle_a);
	glm::vec3 norm = glm::normalize(perp);

	float dot_from  = glm::dot(norm, sphere_from-triangle_a);
	float dot_to = glm::dot(norm, sphere_to-triangle_a);

	float t0 = 1.0;
	float t1 = -1.0;
	//above triangle
	if (dot_from > 0.0f && dot_from > dot_to){
		t0 = (dot_from - sphere_radius) / (dot_from - dot_to);
		t1 = (dot_from + sphere_radius) / (dot_from - dot_to);
	} else if (dot_from < 0.0f && dot_from < dot_to){
		t0 = (dot_from + sphere_radius) / (dot_to - dot_from);
		t1 = (- dot_from + sphere_radius) / (dot_to - dot_from);
	}

	if (t1 < 0.0f || t0 > t) return false;

	float at_t = glm::max(0.0f, t0);
	glm::vec3 at = glm::mix(sphere_from, sphere_to, at_t);

	glm::vec3 triangle_pt = at + glm::dot(triangle_a - at, norm) * norm;

	if ((
    glm::dot(glm::cross(triangle_a-triangle_b,triangle_a-triangle_pt), norm) >= 0 &&
		glm::dot(glm::cross(triangle_c-triangle_a,triangle_c-triangle_pt), norm) >= 0 &&
		glm::dot(glm::cross(triangle_b-triangle_c,triangle_b-triangle_pt), norm) >= 0
  ) || (
    glm::dot(glm::cross(triangle_a-triangle_b,triangle_a-triangle_pt), norm) <=0 &&
		glm::dot(glm::cross(triangle_c-triangle_a,triangle_c-triangle_pt), norm) <=0 &&
		glm::dot(glm::cross(triangle_b-triangle_c,triangle_b-triangle_pt), norm) <=0)
  )	{
		if(collision_t) *collision_t = at_t;
		if(collision_at) *collision_at = triangle_pt;
		if(collision_out) *collision_out = careful_normalize(at - triangle_pt);
		return true;
	}

  bool collided = false;
  //edges
  if (collide_ray_vs_cylinder(
    sphere_from, sphere_to - sphere_from,
    triangle_a,triangle_b, sphere_radius,
    collision_t, collision_at, collision_out
  )) {
    collided = true;
  }
  if (collide_ray_vs_cylinder(
    sphere_from, sphere_to - sphere_from,
    triangle_b, triangle_c, sphere_radius,
    collision_t, collision_at, collision_out
  )) {
    collided = true;
  }
  if (collide_ray_vs_cylinder(
    sphere_from, sphere_to - sphere_from,
    triangle_c, triangle_a, sphere_radius,
    collision_t, collision_at, collision_out
  )) {
    collided = true;
  }

	//vertices
	if (collide_swept_sphere_vs_point(sphere_from, sphere_to, sphere_radius,
		triangle_a, collision_t, collision_at, collision_out)) {
		collided = true;
	}
	if (collide_swept_sphere_vs_point(sphere_from, sphere_to, sphere_radius,
		triangle_b, collision_t, collision_at, collision_out)) {
		collided = true;
	}
	if (collide_swept_sphere_vs_point(sphere_from, sphere_to, sphere_radius,
		triangle_c, collision_t, collision_at, collision_out)) {
		collided = true;
	}

	return collided;

	//-----------------------------
}
