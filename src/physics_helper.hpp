#ifndef __PHYSICS_HELPER_H__
#define  __PHYSICS_HELPER_H__

extern int window_width;
extern int window_height;
extern int helicopter_width;
extern int helicopter_height;
extern int walls_number;
extern std::chrono::milliseconds particle_lifespan;

struct Particle{
	double x;
	double y;
	std::chrono::milliseconds lifespan;
	int alfa;
};

bool detect_collision_with_wall(SDL_Rect wall, double x, double y);
void detect_collision(double &x, double &y, SDL_Rect walls[], double &dx, double &dy, double &ddx, double &ddy, bool &game_active);
bool check_game_won(double x, double y);
void calculate_particles(std::vector<Particle> &all_particles, double dt, double x, double y);
void calculate_acceleration(double &ddx, double &ddy, double &fuel, double dt);
void calculate_position(double &x, double &y, double &dx, double &dy, double ddx, double ddy, double dt);
void update_position(SDL_Rect &helicopter, double x, double y);

#endif