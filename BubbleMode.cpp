#include "BubbleMode.hpp"
#include "DrawLines.hpp"
#include "LitColorTextureProgram.hpp"
#include "Mesh.hpp"
#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "data_path.hpp"
#include "Sound.hpp"
#include "collide.hpp"
#include "gl_errors.hpp"

//for glm::pow(quaternion, float):
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <iostream>
#include <random>
#include <ctime>

Load< SpriteAtlas > trade_font_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	return new SpriteAtlas(data_path("trade-font"));
});

BubbleMode::BubbleMode(BubbleLevel const &level_) : start(level_), level(level_) {
	restart();
}

BubbleMode::~BubbleMode() {
}

bool BubbleMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
  if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
    controls.pause = !controls.pause;
    if (controls.pause) SDL_SetRelativeMouseMode(SDL_FALSE);
    else SDL_SetRelativeMouseMode(SDL_TRUE);
    return true;
  }
  if (controls.pause) return false;

  if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_BACKSPACE) {
		restart();
		return true;
	} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_RETURN) {
		return true;
	} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_1) {
		DEBUG_show_geometry = !DEBUG_show_geometry;
		return true;
	} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_2) {
		DEBUG_show_collision = !DEBUG_show_collision;
  } else if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_A) {
			controls.left = (evt.type == SDL_KEYDOWN);
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_D) {
			controls.right = (evt.type == SDL_KEYDOWN);
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
			controls.forward = (evt.type == SDL_KEYDOWN);
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
			controls.backward = (evt.type == SDL_KEYDOWN);
		} else return false;
	} else if (evt.type == SDL_MOUSEMOTION) {

		//based on trackball camera control from ShowMeshesMode:
		//figure out the motion (as a fraction of a normalized [-a,a]x[-1,1] window):
		glm::vec2 delta;
		delta.x = evt.motion.xrel / float(window_size.x) * 2.0f;
		delta.x *= float(window_size.y) / float(window_size.x);
		delta.y = evt.motion.yrel / float(window_size.y) * -2.0f;

    delta *= controls.mouse_sensitivity;

		level.player.view_azimuth -= delta.x;
		level.player.view_elevation -= delta.y;

    // Normalize to [-pi, pi)
		level.player.view_azimuth /= 2.0f * 3.1415926f;
		level.player.view_azimuth -= std::round(level.player.view_azimuth);
		level.player.view_azimuth *= 2.0f * 3.1415926f;

    // Clamp to [-89deg, 89deg]
		level.player.view_elevation = std::max(-89.0f / 180.0f * 3.1415926f, level.player.view_elevation);
		level.player.view_elevation = std::min( 89.0f / 180.0f * 3.1415926f, level.player.view_elevation);

	} else if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP) {
    if (evt.button.button == SDL_BUTTON_LEFT) {
      controls.mouse_down = (evt.type == SDL_MOUSEBUTTONDOWN);
    }
  }
  else return false;

	return true;
}

void BubbleMode::update(float elapsed) {

  if (controls.pause) return;

  float player_ca = std::cos(level.player.view_azimuth);
  float player_sa = std::sin(level.player.view_azimuth);

  glm::mat3 player_frame = glm::mat3_cast(
    glm::angleAxis(level.player.view_azimuth, glm::vec3(0.0f, 0.0f, 1.0f)) *
    glm::angleAxis(-level.player.view_elevation + 0.5f * 3.1415926f, glm::vec3(1.0f, 0.0f, 0.0f))
  );
  // Shooting
  if (gun.cooldown_counter > 0.0f) {
    gun.cooldown_counter -= elapsed;
  }
  if (controls.mouse_down && gun.cooldown_counter <= 0.0f) {
    level.bullets.emplace_back(
      level,
      level.player.transform->position,
      player_frame * glm::vec3(0.0f, 0.0f, -1.0f)
    );
    gun.cooldown_counter += gun.cooldown;
  }

  // 1. Update player velocity
  {
    glm::vec3 player_dvel = glm::vec3(0.0f);
    if (controls.left) player_dvel.x -= 1.0f;
    if (controls.right) player_dvel.x += 1.0f;
    if (controls.backward) player_dvel.y -= 1.0f;
    if (controls.forward) player_dvel.y += 1.0f;

    if (player_dvel != glm::vec3(0.0f)) {
      player_dvel = glm::normalize(player_dvel);
      player_dvel =
        glm::vec3(player_ca, player_sa, 0.0f) * player_dvel.x +
        glm::vec3(-player_sa, player_ca, 0.0f) * player_dvel.y;
      level.player.vel += 0.03f * player_dvel;
    }

    level.player.vel *= std::pow(0.5f, elapsed / 0.05f); // friction

  }

  // 2. Update bullet velocity

  // 3. Update bubble velocity
  for (BubbleLevel::Bubble &b : level.bubbles) {
    b.vel.z += gravity * elapsed;
  }
  // 4. Update bubble-wall collisions
  for (BubbleLevel::Bubble &b : level.bubbles) {
    glm::vec3 pos_new = b.transform.position + b.vel;
    glm::vec3 bounds_min = level.arena_bounds.min + b.transform.scale * glm::vec3(1.0f);
    glm::vec3 bounds_max = level.arena_bounds.max - b.transform.scale * glm::vec3(1.0f);
    if (pos_new.x < bounds_min.x || pos_new.x > bounds_max.x) {
      b.vel.x = -0.98f * b.vel.x;
    }
    if (pos_new.y < bounds_min.y || pos_new.y > bounds_max.y) {
      b.vel.y = -0.98f * b.vel.y;
    }
    if (pos_new.z < bounds_min.z || pos_new.z > bounds_max.z) {
      b.vel.z = -0.98f * b.vel.z;
    }
  }

  // 5. Update bubble-bubble collisions

  // 6. Update bullet-bubble collisions
  {
    auto bl_it = level.bullets.begin();
    while (bl_it != level.bullets.end()) {
      bool collided = false;
      auto b_it = level.bubbles.begin();
      while (b_it != level.bubbles.end()) {
        if (!collide_AABB_vs_AABB(
          bl_it->transform.position - bl_it->transform.scale * 0.4f,
          bl_it->transform.position + bl_it->transform.scale * 0.4f,
          b_it->transform.position - b_it->transform.scale * 1.0f,
          b_it->transform.position + b_it->transform.scale * 1.0f
        )) {
          b_it++;
          continue;
        }

        float t;
        glm::vec3 at;
        glm::vec3 out;

        if (collide_swept_sphere_vs_swept_sphere(
          b_it->transform.position, b_it->transform.position + b_it->vel, 1.0f,
          bl_it->transform.position, bl_it->transform.position + bl_it->vel, 0.4f,
          &t, &at, &out
        )) {
          collided = true;
          if (b_it->mass > 1) {
            uint32_t scale = b_it->mass - 1;
            glm::vec3 r = glm::vec3(1.0f, 1.0f, 0.0f) * (float) scale;
            glm::vec3 flat_r = out;
            flat_r.z = 0.0f;
            flat_r = glm::normalize(flat_r) * r;
            glm::vec3 offset = glm::vec3(-flat_r.y, flat_r.x, 0.0f);
            level.bubbles.emplace_back(level, b_it->transform.position + offset, glm::mix(offset, out, 0.1f) * 0.1f, scale);
            level.bubbles.emplace_back(level, b_it->transform.position - offset, glm::mix(-offset, out, 0.1f) * 0.1f, scale);
          }

          level.drawables.erase(b_it->draw_it);
          level.bubbles.erase(b_it);
          break;
        } else b_it++;
      }
      if (collided) {
        level.drawables.erase(bl_it->draw_it);
        auto temp_it = bl_it;
        bl_it++;
        level.bullets.erase(temp_it);
        continue;
      }
      else bl_it++;
    }

  }


  // 7. Update bubble-player collisions

  // 8. Update player position

  level.player.transform->position += level.player.vel;

  // 9. Update player-wall collisions

  level.player.transform->position.x = glm::min(
    glm::max(
      level.player.transform->position.x,
      level.arena_bounds.min.x
    ), level.arena_bounds.max.x
  );
  level.player.transform->position.y = glm::min(
    glm::max(
      level.player.transform->position.y,
      level.arena_bounds.min.y
    ), level.arena_bounds.max.y
  );
  //std::cout << level.player.transform->position.x << std::endl;
  {
    level.player.transform->rotation =
      glm::angleAxis(
        level.player.view_azimuth,
        glm::vec3(0.0f, 0.0f, 1.0f)
      ) *
      glm::angleAxis(
        -level.player.view_elevation + 0.5f * 3.1415926f,
        glm::vec3(1.0f, 0.0f, 0.0f)
      );
  }

  // 10. Update bubble positions

  for (BubbleLevel::Bubble &b : level.bubbles) {
    b.transform.position += b.vel;
  }

  // 11. Update bullet positions, bullet-wall collisions

  auto bl_it = level.bullets.begin();
  while (bl_it != level.bullets.end()) {
    bl_it->transform.position += bl_it->vel;
    if (bl_it->transform.position.x < level.arena_bounds.min.x ||
      bl_it->transform.position.x > level.arena_bounds.max.x ||
      bl_it->transform.position.y < level.arena_bounds.min.y ||
      bl_it->transform.position.y > level.arena_bounds.max.y
    ) {
      level.drawables.erase(bl_it->draw_it);
      auto temp_it = bl_it;
      bl_it++;
      level.bullets.erase(temp_it);
      continue;
    }
    bl_it++;
  }


}

void BubbleMode::draw(glm::uvec2 const &drawable_size) {
	//--- actual drawing ---
	glClearColor(0.45f, 0.45f, 0.50f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	level.player.camera->aspect = drawable_size.x / float(drawable_size.y);
	level.draw(*level.player.camera);

	{ //help text overlay:
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		DrawSprites draw(*trade_font_atlas, glm::vec2(0,0), glm::vec2(640, 400), drawable_size, DrawSprites::AlignPixelPerfect);

		{
			std::string help_text = "wasd:move, mouse:camera, esc: pause";
			glm::vec2 min, max;
			draw.get_text_extents(help_text, glm::vec2(0.0f, 0.0f), 1.0f, &min, &max);
			float x = std::round(320.0f - (0.5f * (max.x + min.x)));
			draw.draw_text(help_text, glm::vec2(x, 1.0f), 1.0f, glm::u8vec4(0x00,0x00,0x00,0xff));
			draw.draw_text(help_text, glm::vec2(x, 2.0f), 1.0f, glm::u8vec4(0xff,0xff,0xff,0xff));
		}

		if (level.bubbles.empty()) {
			std::string text = "Finished! bksp: reset";
			glm::vec2 min, max;
			draw.get_text_extents(text, glm::vec2(0.0f, 0.0f), 2.0f, &min, &max);
			float x = std::round(320.0f - (0.5f * (max.x + min.x)));
			draw.draw_text(text, glm::vec2(x, 200.0f), 2.0f, glm::u8vec4(0x00,0x00,0x00,0xff));
			draw.draw_text(text, glm::vec2(x, 201.0f), 2.0f, glm::u8vec4(0xff,0xff,0xff,0xff));
		}

    if (controls.pause) {
  			std::string text = "paused";
  			glm::vec2 min, max;
  			draw.get_text_extents(text, glm::vec2(0.0f, 0.0f), 2.0f, &min, &max);
  			float x = std::round(320.0f - (0.5f * (max.x + min.x)));
  			draw.draw_text(text, glm::vec2(x, 300.0f), 2.0f, glm::u8vec4(0x00,0x00,0x00,0xff));
  			draw.draw_text(text, glm::vec2(x, 301.0f), 2.0f, glm::u8vec4(0xff,0xff,0xff,0xff));
    }

	}

	if (DEBUG_draw_lines) { //DEBUG drawing:
		//adjust world-to-clip matrix to current camera:
		DEBUG_draw_lines->world_to_clip =
      level.player.camera->make_projection() *
      level.player.camera->transform->make_world_to_local();
		//delete object (draws in destructor):
		DEBUG_draw_lines.reset();
	}


	GL_ERRORS();
}

void BubbleMode::restart() {
	level = start;
	won = false;

  std::mt19937 mt;
  mt.seed((int) time(NULL));
  std::uniform_int_distribution<int> dist_count(1, 4);
  std::uniform_real_distribution<float> dist_xy(-10.0f, 10.0f);
  std::uniform_real_distribution<float> dist_z(6.0f, 10.0f);
  std::uniform_real_distribution<float> dist_vxy(0.05f, 0.2f);
  int count = dist_count(mt);
  for (int i = 0; i < count; i++) {
    level.bubbles.emplace_back(
      level,
      glm::vec3(dist_xy(mt), dist_xy(mt), dist_z(mt)),
      glm::vec3(dist_vxy(mt), dist_vxy(mt), 0.0f),
      3
    );
  }

}
