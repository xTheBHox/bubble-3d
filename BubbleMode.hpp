#pragma once

#include "Mode.hpp"
#include "BubbleLevel.hpp"
#include "DrawLines.hpp"

#include <memory>

struct BubbleMode : Mode {
	BubbleMode(BubbleLevel const &level);
	virtual ~BubbleMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//The (starting shape of the) level:
	BubbleLevel const &start;

	//The (active, being-played) level:
	void restart();
	BubbleLevel level;
	bool won = false;

	//Current control signals:
	struct {
		bool forward = false;
		bool backward = false;
		bool left = false;
		bool right = false;
    bool mouse_locked = true;
    bool mouse_down = false;
    bool pause = false;
    float mouse_sensitivity = 4.0f;
	} controls;

  struct {
    float cooldown = 0.5f;
    float cooldown_counter = 0.0f;
  } gun;

	//fly around for collsion debug:
	bool DEBUG_fly = false;
	bool DEBUG_show_geometry = false;
	bool DEBUG_show_collision = false;

	//some debug drawing done during update:
	std::unique_ptr< DrawLines > DEBUG_draw_lines;

  float gravity = -0.2f;

};
