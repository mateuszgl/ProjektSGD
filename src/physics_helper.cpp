#include <SDL2/SDL.h>
#include <thread>
#include <vector>
#include "physics_helper.hpp"

int window_width = 640;
int window_height = 480;
int helicopter_width = 128*0.75;
int helicopter_height = 64*0.75;
int walls_number = 2;
std::chrono::milliseconds particle_lifespan = std::chrono::milliseconds(100);

bool detect_collision_with_wall(SDL_Rect wall, double x, double y){
	return ((x+helicopter_width > wall.x + 5) 
		&& (x < wall.x + wall.w) 
		&& (y + helicopter_height > wall.y + 5)
		&& (y < wall.y + wall.h));
}

void detect_collision(double &x, double &y, SDL_Rect walls[], double &dx, double &dy, double &ddx, double &ddy, bool &game_active){
		
	if ((x > window_width-helicopter_width) || (x < 10) || (y < 0)){
		game_active = false;
	} else 

	if(y > window_height-helicopter_height) {
		y = window_height-helicopter_height;
	
		if(dy > 60){
			game_active = false;
		}
		dx = 0;
		dy = 0;
		ddx = 0;
		ddy = 0; 

	} else 
	{
		for( int i = 0; i < walls_number; i++ ){	
			if(detect_collision_with_wall(walls[i], x, y)){
				game_active = false;
			}
		}
	}
}

bool check_game_won(double x, double y){
	return ((x > window_width/2) && (y == window_height-helicopter_height));
}

void calculate_particles(std::vector<Particle> &all_particles, double dt, double x, double y){
	all_particles.at(0).lifespan -= std::chrono::milliseconds((int)(1000.0*dt));
			
	if(std::chrono::duration_cast<std::chrono::milliseconds>(all_particles.at(0).lifespan).count() < 0 ){
		all_particles.erase(all_particles.begin());
			
		Particle particle;
		particle.x = x+25;
		particle.y = y+30;
		particle.lifespan = particle_lifespan;
		all_particles.push_back(particle);

		all_particles.at(0).alfa = 55;
		all_particles.at(1).alfa = 105;
		all_particles.at(2).alfa = 155;
		all_particles.at(3).alfa = 205;
		all_particles.at(4).alfa = 255;
	}

}
void calculate_acceleration(double &ddx, double &ddy, double &fuel, double dt){
	
	ddx = 0;
	ddy = 100;
	
	const Uint8 *state = SDL_GetKeyboardState(NULL);

	if (state[SDL_SCANCODE_UP] && fuel > 0) {
		ddy = -150;
		fuel -= dt;
	}
	if (state[SDL_SCANCODE_LEFT] && ddy < 0) {
		ddx = -75;
	}
	if (state[SDL_SCANCODE_RIGHT] && ddy < 0) {
		ddx = 75;
	}
}

void calculate_position(double &x, double &y, double &dx, double &dy, double ddx, double ddy, double dt){

	x += dx*dt + 0.5*ddx*dt*dt;
	y += dy*dt + 0.5*ddy*dt*dt;
	dx *= 0.999; 
	dx += ddx*dt;
	dy += ddy*dt;
	
}

void update_position(SDL_Rect &helicopter, double x, double y){
	helicopter.x = x;
	helicopter.y = y;
}