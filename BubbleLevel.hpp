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
    Scene::Transform *transform = nullptr;
    glm::vec3 vel = glm::vec3(0.0f, 0.0f, 0.0f);
    uint32_t mass_log = 4;
    float r;
    // glm::vec3 rot_vel;
  };

  struct Bullet {
    Scene::Transform *transform = nullptr;
    glm::vec3 vel;
    float r;
  };

	// Player camera tracked using this structure:
	struct PlayerCam {
    Scene::Camera *camera = nullptr;
    Scene::Transform *transform = nullptr;
		glm::vec3 vel = glm::vec3(0.0f, 0.0f, 0.0f);
    float view_azimuth;
    float view_elevation;
	};

	//Additional information for things in the level:
	std::vector< MeshCollider > mesh_colliders;
	std::vector< Bubble > bubbles;
  std::vector< Bullet > bullets;
	PlayerCam player;

};
