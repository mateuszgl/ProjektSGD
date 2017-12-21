#ifndef __DRAWING_HELPER_H__
#define  __DRAWING_HELPER_H__

#include "physics_helper.hpp"

std::shared_ptr<SDL_Window> init_window( int width , int height );
std::shared_ptr<SDL_Renderer> init_renderer( std::shared_ptr<SDL_Window> window );
std::shared_ptr<SDL_Texture> load_texture( const std::shared_ptr<SDL_Renderer> renderer, const std::string fname );
void draw_walls(std::shared_ptr<SDL_Renderer> renderer, SDL_Rect walls[]);
void draw_explosion(std::shared_ptr<SDL_Renderer> renderer, std::shared_ptr<SDL_Texture> texture, SDL_Rect rect, double x, double y);
void draw_game_status(std::shared_ptr<SDL_Renderer> renderer, std::shared_ptr<SDL_Texture> texture, SDL_Rect rect);
void draw_fuel_gauge(std::shared_ptr<SDL_Renderer> renderer, double fuel, double starting_fuel, SDL_Rect rect);
void draw_particle(Particle part, std::shared_ptr<SDL_Renderer> renderer);
void draw_all_particles(std::vector<Particle> particles, std::shared_ptr<SDL_Renderer> renderer);

#endif