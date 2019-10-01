#pragma once

/*
 * A BubbleLevel is a scene augmented with some additional information which
 *   is useful when playing Bubble 3D.
 */

#include "Scene.hpp"
#include "Mesh.hpp"
#include "Load.hpp"

struct BubbleLevel;

//List of all levels:
extern Load< std::list< BubbleLevel > > bubble_levels;

struct BubbleLevel : Scene {
	//Build from a scene:
	//  note: will throw on loading failure, or if certain critical objects don't appear
	BubbleLevel(std::string const &scene_file);

	//Copy constructor:
	//  used to copy a pristine, just-loaded level to a level that is being played
	//    (and thus might be changed)
	//  -- needs to be careful to fixup pointers.
	BubbleLevel(BubbleLevel const &);
	// copy constructor actually just uses this = operator:
	BubbleLevel &operator=(BubbleLevel const &);

	//Solid parts of level are tracked as MeshColliders:
	struct MeshCollider {
		MeshCollider(Scene::Transform *transform_, Mesh const &mesh_, MeshBuffer const &buffer_) : transform(transform_), mesh(&mesh_), buffer(&buffer_) { }
		Scene::Transform *transform;
		Mesh const *mesh;
		MeshBuffer const *buffer;
	};

  // Bubble target(s) tracked using this structure:
  struct Bubble {
    Bubble(BubbleLevel &lvl, glm::vec3 &pos, glm::vec3 &vel_, uint32_t mass_);
    std::list< Scene::Drawable >::iterator draw_it;
    Scene::Transform transform;
    glm::vec3 vel;
    uint32_t mass;
    // glm::vec3 rot_vel;
  };

  struct Bullet {
    Bullet(BubbleLevel &lvl, glm::vec3 &pos, glm::vec3 &vel_);
    std::list< Scene::Drawable >::iterator draw_it;
    Scene::Transform transform;
    glm::vec3 vel;
  };

	// Player camera tracked using this structure:
	struct PlayerCam {
    Scene::Camera *camera = nullptr;
    Scene::Transform *transform = nullptr;
		glm::vec3 vel = glm::vec3(0.0f, 0.0f, 0.0f);
    float view_azimuth = 0.0f;
    float view_elevation = 0.0f;
	};

  struct {
    glm::vec3 min = glm::vec3(-20.0f, -20.0f, 0.0f);
    glm::vec3 max = glm::vec3(20.0f, 20.0f, 15.0f);
  } arena_bounds;

	//Additional information for things in the level:
	std::list< Bubble > bubbles;
  std::list< Bullet > bullets;
	PlayerCam player;

};
