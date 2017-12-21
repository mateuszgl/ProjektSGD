#include <SDL2/SDL.h>
#include <thread>
#include <vector>
#include "physics_helper.hpp"
#include "drawing_helper.hpp"

int main( ) { 

	double x = 75, y = window_height-helicopter_height;    // starting position
	double dx = 0, dy = 0;     // speed
	double ddx = 0, ddy = 0;   // acceleration 
	double dt = 1.0/120.0;     // time increase

	double starting_fuel = 3;
	double fuel = starting_fuel;

	auto window = init_window(window_width, window_height);
	auto renderer = init_renderer( window );
	SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);

	auto helicopter_texture = load_texture( renderer, "data/helicopter.bmp" );
	auto explosion_texture = load_texture( renderer, "data/explosion.bmp" );
	auto game_won_texture = load_texture( renderer, "data/game_won.bmp" );
	auto game_over_texture = load_texture( renderer, "data/game_lose.bmp" );

	SDL_Rect helicopter; 
	helicopter.w = helicopter_width;    
	helicopter.h = helicopter_height;   
	helicopter.x = x;
	helicopter.y = y;
	
	SDL_Rect explosion; 
	explosion.w = 72; 
	explosion.h = 60; 
	
	SDL_Rect game_status; 
	game_status.w = 200;
	game_status.h = 50;

	SDL_Rect fuel_gauge; 
	fuel_gauge.w = 10; 
	fuel_gauge.x = 0;
	fuel_gauge.y = 0;

	SDL_Rect wall; 
	wall.w = 20;
	wall.h = 200;
	wall.x = window_width/2 - wall.w/2;
	wall.y = window_height - wall.h;

	SDL_Rect wall2; 
	wall2.w = 20;
	wall2.h = 75;
	wall2.x = window_width/2 - wall.w/2;
	wall2.y = 0;

	SDL_Rect walls [] = {wall, wall2};

	Particle particle;
	particle.x = -10;
	particle.y = -10;
	particle.alfa = 255;
	particle.lifespan = particle_lifespan;

	std::vector<Particle> all_particles;
	all_particles.reserve(10000);

	for(int i = 0; i < 5;i++){
		all_particles.push_back(particle);
	}

	auto current_time = std::chrono::system_clock::now();
	auto new_time = current_time;

	for ( bool game_active = true ; game_active; ) {

		SDL_Event event;
		while ( SDL_PollEvent( &event ) ) {
			if ( event.type == SDL_QUIT ) game_active = false;
		}



		SDL_RenderClear( renderer.get() );
		draw_walls(renderer, walls);
		SDL_RenderCopy( renderer.get(), helicopter_texture.get(), NULL, &helicopter );
		draw_all_particles(all_particles, renderer);
		draw_fuel_gauge(renderer, fuel, starting_fuel, fuel_gauge);



		detect_collision(x, y, walls, dx, dy, ddx, ddy, game_active);
		if(!game_active){
			draw_explosion(renderer, explosion_texture, explosion, x, y);
			draw_game_status(renderer, game_over_texture, game_status);
		} else 
		if(check_game_won(x,y)){
			draw_game_status(renderer, game_won_texture, game_status);
			game_active = false;
		}



		calculate_particles(all_particles, dt, x, y);
		calculate_acceleration(ddx,ddy,fuel,dt);
		calculate_position(x, y, dx, dy, ddx, ddy, dt);
		update_position(helicopter, x, y);



		new_time = current_time + std::chrono::milliseconds((int)(1000.0*dt));
		std::this_thread::sleep_until (new_time);
		current_time = new_time;
		SDL_RenderPresent( renderer.get() );

	}

	std::this_thread::sleep_for (std::chrono::milliseconds(10000));

	return 0;
}