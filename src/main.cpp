#include <SDL2/SDL.h>
#include <iostream>
#include <thread>

const int window_width = 640;
const int window_height = 480;
const int rocket_width = 32*2;
const int rocket_height = 64*2;

auto errthrow = []( const std::string &e ) {
	std::string errstr = e + " : " + SDL_GetError();
	SDL_Quit();
	throw std::runtime_error( errstr );
};

std::shared_ptr<SDL_Window> init_window( int width , int height ) {
	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) errthrow ( "SDL_Init" );

	SDL_Window *win = SDL_CreateWindow( "Witaj w Swiecie",
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


int main( ) { // int argc, char **argv ) {

	double x = 75, y = 352;   // starting position
	double dx = 0, dy = 0;  // speed
	double ddx = 0, ddy = 0; // acceleration 
	double dt = 1.0/120.0; // time increase

	double starting_fuel = 5;
	double fuel = starting_fuel;

	auto window = init_window(window_width, window_height);
	auto renderer = init_renderer( window );
	auto game_texture = load_texture( renderer, "data/rocket.bmp" );
	auto game_over_texture = load_texture( renderer, "data/explosion.bmp" );

	// rectangle to display texture
	SDL_Rect texr; 
	texr.w = rocket_width; //texture width
	texr.h = rocket_height	; //texture height
	
	SDL_Rect explosion; 
	explosion.w = 72; //texture width
	explosion.h = 60; //texture height

	//game loop
	for ( bool game_active = true ; game_active; ) {
		SDL_Event event;
		// someone closed window event
		while ( SDL_PollEvent( &event ) ) {
			if ( event.type == SDL_QUIT ) game_active = false;
		}

		// collision with wall
		if ((x > window_width-rocket_width) || (x < 0) || (y < 0)){
			game_active = false;
		}
		
		// landing
		if(y > window_height-rocket_height) {
			y = window_height-rocket_height;
			
			// landing failed
			if(dy > 60){
				std::cout << "......" <<"\n";
				game_active = false;
			}

			dx = 0;
			dy = 0;
			ddx = 0;
			ddy = 0; 
		}
		
		//update rocket position
		texr.x = x;
		texr.y = y;

		//draw rocket in new position
		SDL_RenderClear( renderer.get() );
		SDL_RenderCopy( renderer.get(), game_texture.get(), NULL, &texr );

		// landing failed
		if(!game_active){
			explosion.x = x-5;
			explosion.y = y+40;
			SDL_RenderCopy( renderer.get(), game_over_texture.get(), NULL, &explosion );
		}

		SDL_RenderPresent( renderer.get() );
		
		// reset acceleration
		ddx = 0;
		ddy = 100;
	
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		// every acceleration uses fuel
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
		
		// "wind drag"
		dx *= 0.999; 

		dx += ddx*dt;
		dy += ddy*dt;
		x += dx*dt;
		y += dy*dt;

		std::cout << (fuel/starting_fuel)*100 << "%" <<"\n";
		std::cout << "dy:" << dy <<"\n";

		std::this_thread::sleep_for (std::chrono::milliseconds((int)(1000.0*dt)));
	}

	std::this_thread::sleep_for (std::chrono::milliseconds(10000));

	return 0;
}
