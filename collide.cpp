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
	//-----------------------------

	//First task: fill in this function

	return true;

	//-----------------------------
}



//helper: ray vs sphere:
bool collide_ray_vs_sphere(
	glm::vec3 const &ray_start, glm::vec3 const &ray_direction,
	glm::vec3 const &sphere_center, float sphere_radius,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_normal) {

	//if ray is travelling away from sphere, don't intersect:
	if (glm::dot(ray_start - sphere_center, ray_direction) >= 0.0f) return false;

	//when is (ray_start + t * ray_direction - sphere_center)^2 <= sphere_radius^2 ?

	//Solve a quadratic equation to find out:
	float a = glm::dot(ray_direction, ray_direction);
	float b = 2.0f * glm::dot(ray_start - sphere_center, ray_direction);
	float c = glm::dot(ray_start - sphere_center, ray_start - sphere_center) - sphere_radius*sphere_radius;

	//this is the part of the quadratic formula under the radical:
	float d = b*b - 4.0f * a * c;
	if (d < 0.0f) return false;
	d = std::sqrt(d);

	//intersects between t0 and t1:
	float t0 = (-b - d) / (2.0f * a);
	float t1 = (-b + d) / (2.0f * a);

	if (t1 < 0.0f || t0 > 1.0f) return false;
	if (collision_t && t0 >= *collision_t) return false;

	if (t0 <= 0.0f) {
		//collides (or was already colliding) at start:
		if (collision_t) *collision_t = 0.0f;
		if (collision_at) *collision_at = ray_start;
		if (collision_normal) *collision_normal = careful_normalize(ray_start - sphere_center);
		return true;
	} else {
		//collides somewhat after start:
		if (collision_t) *collision_t = t0;
		if (collision_at) *collision_at = ray_start + t0 * ray_direction;
		if (collision_normal) *collision_normal = careful_normalize((ray_start + t0 * ray_direction) - sphere_center);
		return true;
	}
}

bool collide_swept_sphere_vs_triangle(
	glm::vec3 const &sphere_from, glm::vec3 const &sphere_to, float sphere_radius,
	glm::vec3 const &triangle_a, glm::vec3 const &triangle_b, glm::vec3 const &triangle_c,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out
) {
	//-----------------------------

	//Second task: fill in this function

	return false;

	//-----------------------------
}
