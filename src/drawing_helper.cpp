#include <SDL2/SDL.h>
#include <thread>
#include <vector>
#include "drawing_helper.hpp"
#include "physics_helper.hpp"


auto errthrow = []( const std::string &e ) {
	std::string errstr = e + " : " + SDL_GetError();
	SDL_Quit();
	throw std::runtime_error( errstr );
};

std::shared_ptr<SDL_Window> init_window( int width , int height ) {
	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) errthrow ( "SDL_Init" );

	SDL_Window *win = SDL_CreateWindow( "Rakieta liga",
										SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
										width, height, SDL_WINDOW_SHOWN );
	if ( win == nullptr ) errthrow ( "SDL_CreateWindow" );
	std::shared_ptr<SDL_Window> window ( win, []( SDL_Window * ptr ) {
		SDL_DestroyWindow( ptr );
	} );
	return window;
}

std::shared_ptr<SDL_Renderer> init_renderer( std::shared_ptr<SDL_Window> window ) {
	SDL_Renderer *ren = SDL_CreateRenderer( window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if ( ren == nullptr ) errthrow ( "SDL_CreateRenderer" );
	std::shared_ptr<SDL_Renderer> renderer ( ren, []( SDL_Renderer * ptr ) {
		SDL_DestroyRenderer( ptr );
	} );
	return renderer;
}

std::shared_ptr<SDL_Texture> load_texture( const std::shared_ptr<SDL_Renderer> renderer, const std::string fname ) {
	SDL_Surface *bmp = SDL_LoadBMP( fname.c_str() );
	if ( bmp == nullptr ) errthrow ( "SDL_LoadBMP" );
	std::shared_ptr<SDL_Surface> bitmap ( bmp, []( SDL_Surface * ptr ) {
		SDL_FreeSurface( ptr );
	} );

	SDL_Texture *tex = SDL_CreateTextureFromSurface( renderer.get(), bitmap.get() );
	if ( tex == nullptr ) errthrow ( "SDL_CreateTextureFromSurface" );
	std::shared_ptr<SDL_Texture> texture ( tex, []( SDL_Texture * ptr ) {
		SDL_DestroyTexture( ptr );
	} );
	return texture;
}

void draw_walls(std::shared_ptr<SDL_Renderer> renderer, SDL_Rect walls[]){

	SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);

	for( int i = 0; i < walls_number; i++ ){	
		SDL_RenderFillRect( renderer.get(), &walls[i]);
	}
	
	SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);

}

void draw_explosion(std::shared_ptr<SDL_Renderer> renderer, std::shared_ptr<SDL_Texture> texture, SDL_Rect rect, double x, double y){
	rect.x = x + helicopter_width/2 - 36;
	rect.y = y + helicopter_height/2 - 30;
	SDL_RenderCopy( renderer.get(), texture.get(), NULL, &rect );
}

void draw_game_status(std::shared_ptr<SDL_Renderer> renderer, std::shared_ptr<SDL_Texture> texture, SDL_Rect rect){
	rect.x = window_height/2 - rect.h/2;
	rect.y = window_width/2 - rect.w/2;
	SDL_RenderCopy( renderer.get(), texture.get(), NULL, &rect );
}

void draw_fuel_gauge(std::shared_ptr<SDL_Renderer> renderer, double fuel, double starting_fuel, SDL_Rect rect){

	int fuel_percentage = (fuel/starting_fuel)*100;

	switch(fuel_percentage){
		case 76 ... 100 : {
			SDL_SetRenderDrawColor(renderer.get(), 0, 255, 0, 255);    		
			break;
			}

		case 51 ... 75: {
			SDL_SetRenderDrawColor(renderer.get(), 255, 255, 0, 255);    	
			break;
			}

		case 25 ... 50: {
			SDL_SetRenderDrawColor(renderer.get(), 255, 128, 0, 255);    	
			break;
			}

		case 0 ... 24: {
			SDL_SetRenderDrawColor(renderer.get(), 255, 0, 0, 255);    	
			break;
			}
	}

	rect.y = window_height - window_height * fuel_percentage / 100;
	rect.h = window_height - rect.y;

	SDL_RenderFillRect( renderer.get(), &rect);
	
	SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);

}

void draw_particle(Particle part, std::shared_ptr<SDL_Renderer> renderer)
{
	SDL_SetRenderDrawColor(renderer.get(), 180, 180, 180, part.alfa);

	for (int w = 1; w < 4; w++) {
		for (int h = 1; h < 4; h++) {
			SDL_RenderDrawPoint(renderer.get(), part.x+ w, part.y + h);
		}
	}
	SDL_RenderDrawPoint(renderer.get(), part.x + 2, part.y);
	SDL_RenderDrawPoint(renderer.get(), part.x, part.y + 2);
	SDL_RenderDrawPoint(renderer.get(), part.x + 2, part.y + 4);
	SDL_RenderDrawPoint(renderer.get(), part.x + 4, part.y + 2);
	
	SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);

}

void draw_all_particles(std::vector<Particle> particles, std::shared_ptr<SDL_Renderer> renderer){
	for (unsigned int i = 0; i < particles.capacity(); i++){
		draw_particle(particles.at(i), renderer);
	}

}