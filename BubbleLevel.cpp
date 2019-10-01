#include "BubbleLevel.hpp"
#include "data_path.hpp"
#include "LitColorTextureProgram.hpp"

#include <unordered_set>
#include <unordered_map>
#include <iostream>

//used for lookup later:
Mesh const *mesh_Bullet = nullptr;
Mesh const *mesh_Bubble = nullptr;
Mesh const *mesh_Arena = nullptr;

GLuint bubble_meshes_for_lit_color_texture_program = 0;

//Load the meshes used in Bubble 3D levels:
Load< MeshBuffer > bubble_meshes(LoadTagDefault, []() -> MeshBuffer * {
	MeshBuffer *ret = new MeshBuffer(data_path("bubble-parts.pnct"));

	//Build vertex array object for the program we're using to shade these meshes:
	bubble_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);

	//key objects:
	mesh_Bullet = &ret->lookup("Bullet");
	mesh_Bubble = &ret->lookup("Bubble");
	mesh_Arena = &ret->lookup("Arena");

	return ret;
});

Load< std::list< BubbleLevel > > bubble_levels(LoadTagLate, []() -> std::list< BubbleLevel > * {
	std::list< BubbleLevel > *ret = new std::list< BubbleLevel >();
	ret->emplace_back(data_path("bubble-level-1.scene"));
	return ret;
});


//-------- BubbleLevel ---------

BubbleLevel::BubbleLevel(std::string const &scene_file) {
  auto load_fn = [this](Scene &, Transform *transform, std::string const &mesh_name){
    Mesh const *mesh = &bubble_meshes->lookup(mesh_name);

    drawables.emplace_back(transform);
    Drawable::Pipeline &pipeline = drawables.back().pipeline;

    //set up drawable to draw mesh from buffer:
    pipeline = lit_color_texture_program_pipeline;
    pipeline.vao = bubble_meshes_for_lit_color_texture_program;
    pipeline.type = mesh->type;
    pipeline.start = mesh->start;
    pipeline.count = mesh->count;

  };
	//Load scene (using Scene::load function), building proper associations as needed:
	load(scene_file, load_fn);

	//Create player camera:
  player = PlayerCam();
	transforms.emplace_back();
  player.transform = &transforms.back();
	cameras.emplace_back(&transforms.back());
	player.camera = &cameras.back();
	player.camera->transform = &transforms.back();

	player.camera->fovy = 60.0f / 180.0f * 3.1415926f;
	player.camera->near = 0.05f;
  player.camera->transform->position.z = 2.0f;

}

BubbleLevel::BubbleLevel(BubbleLevel const &other) {
	*this = other;
}
BubbleLevel &BubbleLevel::operator=(BubbleLevel const &other) {
	//copy other's transforms, and remember the mapping between them and the copies:
	std::unordered_map< Transform const *, Transform * > transform_to_transform;
	//null transform maps to itself:
	transform_to_transform.insert(std::make_pair(nullptr, nullptr));

	//Copy transforms and store mapping:
	for (auto const &t : other.transforms) {
		transforms.emplace_back();
		transforms.back().name = t.name;
		transforms.back().position = t.position;
		transforms.back().rotation = t.rotation;
		transforms.back().scale = t.scale;
		transforms.back().parent = t.parent; //will update later

		//store mapping between transforms old and new:
		auto ret = transform_to_transform.insert(std::make_pair(&t, &transforms.back()));
		assert(ret.second);
	}

	//update transform parents:
	for (auto &t : transforms) {
		t.parent = transform_to_transform.at(t.parent);
	}

	//copy other's drawables, updating transform pointers:
	drawables = other.drawables;
	for (auto &d : drawables) {
		d.transform = transform_to_transform.at(d.transform);
	}

	//copy other's cameras, updating transform pointers:
	for (auto const &c : other.cameras) {
		cameras.emplace_back(c);
		cameras.back().transform = transform_to_transform.at(c.transform);

		//update camera pointer when that camera is copied:
		if (&c == other.player.camera) {
      player.camera = &cameras.back();
      player.transform = cameras.back().transform;
    }
	}

	//copy other's lamps, updating transform pointers:
	lamps = other.lamps;
	for (auto &l : lamps) {
		l.transform = transform_to_transform.at(l.transform);
	}

  /* Don't copy bullets
	bullets = other.bullets;
  */

	//player = other.player;
	//player.transform = transform_to_transform.at(player.transform);

	return *this;
}

BubbleLevel::Bullet::Bullet(BubbleLevel &lvl, glm::vec3 &pos, glm::vec3 &vel_) {
  transform.position = pos;
  vel = vel_;

  lvl.drawables.emplace_front(&transform);
  draw_it = lvl.drawables.begin();
  Drawable::Pipeline &pipeline = lvl.drawables.front().pipeline;

  //set up drawable to draw mesh from buffer:
  pipeline = lit_color_texture_program_pipeline;
  pipeline.vao = bubble_meshes_for_lit_color_texture_program;
  pipeline.type = mesh_Bullet->type;
  pipeline.start = mesh_Bullet->start;
  pipeline.count = mesh_Bullet->count;

}

BubbleLevel::Bubble::Bubble(BubbleLevel &lvl, glm::vec3 &pos, glm::vec3 &vel_, uint32_t mass_) {
  vel = vel_;
  mass = mass_;
  transform.position = pos;
  transform.scale = glm::vec3(0.5f) * (float) mass;

  lvl.drawables.emplace_front(&transform);
  draw_it = lvl.drawables.begin();
  Drawable::Pipeline &pipeline = lvl.drawables.front().pipeline;

  //std::cout << &transform << std::endl;
  //std::cout << draw_it->transform << std::endl;
  //std::cout << drawables->front().transform << std::endl;

  //set up drawable to draw mesh from buffer:
  pipeline = lit_color_texture_program_pipeline;
  pipeline.vao = bubble_meshes_for_lit_color_texture_program;
  pipeline.type = mesh_Bubble->type;
  pipeline.start = mesh_Bubble->start;
  pipeline.count = mesh_Bubble->count;

}
