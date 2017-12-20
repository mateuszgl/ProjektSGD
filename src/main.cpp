#include <SDL2/SDL.h>
#include <iostream>
#include <thread>
#include <vector>

const int window_width = 640;
const int window_height = 480;
const int helicopter_width = 128*0.75;
const int helicopter_height = 64*0.75;
const int walls_number = 2;
const auto particle_lifespan = std::chrono::milliseconds(100);

struct Particle{
	double x;
	double y;
	std::chrono::milliseconds lifespan;
	int alfa;
};

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
		for( unsigned int i = 0; i < walls_number; i++ ){	
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
	for(int i = 0; i< 5;i++){
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