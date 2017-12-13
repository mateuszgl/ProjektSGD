#include <SDL2/SDL.h>
#include <iostream>
#include <thread>

const int window_width = 640;
const int window_height = 480;
const int rocket_width = 32*2;
const int rocket_height = 64*2;
const int walls_number = 2;

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

	for( unsigned int i = 0; i < walls_number; i++ ){	
		SDL_RenderFillRect( renderer.get(), &walls[i]);
	}
	
	SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);

}

void draw_explosion(std::shared_ptr<SDL_Renderer> renderer, std::shared_ptr<SDL_Texture> texture, SDL_Rect rect, double x, double y){
	rect.x = x-5;
	rect.y = y+40;
	SDL_RenderCopy( renderer.get(), texture.get(), NULL, &rect );
}

bool detect_collision_with_wall(SDL_Rect wall, double x, double y){
	return ((x+rocket_width > wall.x) 
	&& (x < wall.x + wall.w) 
	&& (y + rocket_height > wall.y)
	&& (y < wall.y + wall.h));
}

void detect_collision(double &x, double &y, SDL_Rect walls[], double &dx, double &dy, double &ddx, double &ddy, bool &game_active){
		
	// collision with window
	if ((x > window_width-rocket_width) || (x < 0) || (y < 0)){
		game_active = false;
	} else 
	// landing
	if(y > window_height-rocket_height) {
		y = window_height-rocket_height;
		
		// landing failed
		if(dy > 60){
			game_active = false;
		}
		dx = 0;
		dy = 0;
		ddx = 0;
		ddy = 0; 

	} else 
	//collision with wall
	{
		for( unsigned int i = 0; i < walls_number; i++ ){	
			if(detect_collision_with_wall(walls[i], x, y)){
				game_active = false;
			}
		}
	}
}



int main( ) { // int argc, char **argv ) {

	double x = 75, y = 352;    // starting position
	double dx = 0, dy = 0;     // speed
	double ddx = 0, ddy = 0;   // acceleration 
	double dt = 1.0/120.0;     // time increase

	double starting_fuel = 5;
	double fuel = starting_fuel;

	auto window = init_window(window_width, window_height);
	auto renderer = init_renderer( window );
	// load textures
	auto game_texture = load_texture( renderer, "data/rocket.bmp" );
	auto game_over_texture = load_texture( renderer, "data/explosion.bmp" );

	// rectangle to display texture
	SDL_Rect texr; 
	texr.w = rocket_width;    //texture width
	texr.h = rocket_height;   //texture height
	
	SDL_Rect explosion; 
	explosion.w = 72; //texture width
	explosion.h = 60; //texture height

	// create some walls to hit
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

	//game loop
	for ( bool game_active = true ; game_active; ) {
		SDL_Event event;
		// someone closed window event
		while ( SDL_PollEvent( &event ) ) {
			if ( event.type == SDL_QUIT ) game_active = false;
		}

		// detect collision with various objects
		detect_collision(x, y, walls, dx, dy, ddx, ddy, game_active);

		//update rocket position
		texr.x = x;
		texr.y = y;

		//draw rocket in new position
		SDL_RenderClear( renderer.get() );

		// draw some walls
		draw_walls(renderer, walls);

		SDL_RenderCopy( renderer.get(), game_texture.get(), NULL, &texr );

		// landing failed
		if(!game_active){
			draw_explosion(renderer, game_over_texture, explosion, x, y);
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
