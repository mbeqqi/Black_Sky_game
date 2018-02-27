#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>
#include <SDL_image.h>
#include <cmath>
#include <cstring>
#include <wchar.h>
#include "Vector2.h"
#include "sdlglutils.h"

#define SW 800
#define SH 600
#define INI_EXPLOSION 8000

#define MIN(x,y) ((x>=y)?y:x)
#define MAX(x,y) ((x>=y)?x:y)
#define SGN(x) ((x>0)?1:-1)
//Game Vars
#define GRAVITY 0.04
#define XSPEED 0.1
#define YSPEED 0.1
#define XBREAK 0.1
#define YBREAK 0.1
#define TIME_BREAK_COEFF 0.05
#define JUMPSPEED 0.7
#define MAX_LASERS 64
#define MAX_BOXES 64
#define MAX_LEVELS 64
//OGL
#define WCARA 11
#define HCARA 13

//Math

#define RAD_TO_DEG 57.295779f
#define PI 3.14159265f
#define DEG_TO_RAD 0.01745329f
#define GOLDEN_RATIO 1.618034f

struct Laser;
struct Box;
struct Level;

int font;

bool collide(Laser* l,Box* b,float* contact=0);
void draw_fill_quad(float x,float y,float w,float h,int r=255,int g=255,int b=255);
void draw_quad(float x,float y,float w,float h,int r=255,int g=255,int b=255,int size=1);
void draw_line(float x1,float y1,float x2,float y2,int width=1,int r=255,int g=255,int b=255);
void draw_cara(char c,int ofx,int ofy);
void draw_text(char* str,int x,int y,int r,int g,int b,int scale=1);
bool SAT(Vector2* c1Vert,Vector2* c2Vert,Vector2 ax);
float lerp(float a,float b,float t);
void load_high_scores();
void save_high_scores();
void load_user_levels();
void load_level_file(char* path);




struct Sprite
{
	Sprite()
	{
	}

	Sprite(int tex_id,int w,int h)
	{
		texture=tex_id;
		clip_region[0]=Vector2(0,0);

		width=w;
		height=h;

		clip_region[1]=Vector2(width,height);
	}

	void set_texture(int tex_id,int w,int h)
	{
		texture=tex_id;

		width=w;
		height=h;

		clip_region[0]=Vector2(0,0);

		clip_region[1]=Vector2(width,height);
	}

	void set_clip_region(float x,float y,float w,float h)
	{
		clip_region[0]=Vector2(x,y);
		clip_region[1]=Vector2(w,h);
	}

	void draw(float x,float y,float w,float h)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,texture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();
		//glScalef(1/width,1/height,1);

		glBegin(GL_QUADS);

		glTexCoord2f(clip_region[0].x/width,clip_region[0].y/height);
		glVertex2f(x,y);

		glTexCoord2f((clip_region[0].x+clip_region[1].x)/width,clip_region[0].y/height);
		glVertex2f(x+w,y);

		glTexCoord2f((clip_region[0].x+clip_region[1].x)/width,(clip_region[0].y+clip_region[1].y)/height);
		glVertex2f(x+w,y+h);

		glTexCoord2f(clip_region[0].x/width,(clip_region[0].y+clip_region[1].y)/height);
		glVertex2f(x,y+h);

		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	Vector2 clip_region[2];
	int width;
	int height;
	int texture;
};

struct Input
{
	Input()
	{
		left=up=right=down=lctrl=r=space=0;
		enter=escape=0;
		left_hold_time=right_hold_time=up_hold_time=down_hold_time=0;
	}

	void update(SDL_Event& ev)
	{
		if(ev.type==SDL_KEYDOWN)
		{
			if(ev.key.keysym.sym==SDLK_LEFT)
				left=1;
			else if(ev.key.keysym.sym==SDLK_RIGHT)
				right=1;
			else if(ev.key.keysym.sym==SDLK_UP)
				up=1;
			else if(ev.key.keysym.sym==SDLK_DOWN)
				down=1;
			else if(ev.key.keysym.sym==SDLK_LCTRL)
				lctrl=1;
			else if(ev.key.keysym.sym==SDLK_r)
				r=1;
			else if(ev.key.keysym.sym==SDLK_SPACE)
				space=1;
			else if(ev.key.keysym.sym==SDLK_RETURN)
				enter=1;
			else if(ev.key.keysym.sym==SDLK_ESCAPE)
				escape=1;
		}

		if(ev.type==SDL_KEYUP)
		{
			if(ev.key.keysym.sym==SDLK_LEFT)
			{
				left_hold_time=0;
				left=0;
			}
			else if(ev.key.keysym.sym==SDLK_RIGHT)
			{
				right_hold_time=0;
				right=0;
			}
			else if(ev.key.keysym.sym==SDLK_UP)
			{
				up_hold_time=0;
				up=0;
			}
			else if(ev.key.keysym.sym==SDLK_DOWN)
			{
				down_hold_time=0;
				down=0;
			}
			else if(ev.key.keysym.sym==SDLK_LCTRL)
				lctrl=0;
			else if(ev.key.keysym.sym==SDLK_r)
				r=0;
			else if(ev.key.keysym.sym==SDLK_SPACE)
				space=0;
			else if(ev.key.keysym.sym==SDLK_RETURN)
				enter=0;
			else if(ev.key.keysym.sym==SDLK_ESCAPE)
				escape=0;
		}
	}

	void time_update(float dt)
	{
		left_hold_time+=dt;
		right_hold_time+=dt;
		up_hold_time+=dt;
		down_hold_time+=dt;
	}

	int left,right,up,down;
	int lctrl,r;
	int space;
	int enter,escape;

	float left_hold_time;
	float right_hold_time;
	float up_hold_time;
	float down_hold_time;
};

struct Box_Desc
{
	Vector2 position;
	Vector2 dimension;
	Vector2 center;//in box's coordinate sys!
	float theta;

	Vector2 velocity;
	float angular_velocity;
	float scale_speed;
	float scale_max;
	float scale_min;
};

int box_tex_test;//@TOMP

//@REFACT:DO a collision_box class..this is not a box!!
struct Box
{

	Box()
	{
		scale=1;
		theta=0;
		angular_velocity=0;
		scale_max=1;
		scale_min=1;
	}

	Box(float x,float y,float w,float h,float a=0)
	{
		position=Vector2(x,y);
		dimension=Vector2(w,h);
		theta=a;
		scale_speed=0;
		scale=1;
		angular_velocity=0;
		scale_max=1;
		scale_min=1;
	}

	Box(Box_Desc bd)
	{
		position=bd.position;
		dimension=bd.dimension;
		center=bd.center;

		theta=bd.theta;
		scale=1;

		scale_speed=bd.scale_speed;
		velocity=bd.velocity;
		angular_velocity=bd.angular_velocity;
		scale_max=bd.scale_max;
		scale_min=bd.scale_min;
	}

	void init(float x,float y,float w,float h,float a=0)
	{
		position=Vector2(x,y);dimension=Vector2(w,h);
		theta=a;
		scale=1;
		scale_speed=0;
		angular_velocity=0;
		scale_max=1;
		scale_min=1;
	}

	void init(Box_Desc bd)
	{
		position=bd.position;
		dimension=bd.dimension;
		center=bd.center;

		theta=bd.theta;
		scale=1;
		scale_max=bd.scale_max;
		scale_min=bd.scale_min;

		scale_speed=bd.scale_speed;
		velocity=bd.velocity;
		angular_velocity=bd.angular_velocity;
	}

	bool collide(Box& box2)
	{
		Vector2 a[4];

		a[0]=Vector2(1,0).rotate(theta*DEG_TO_RAD);
		a[1]=Vector2(0,1).rotate(theta*DEG_TO_RAD);
		a[2]=Vector2(1,0).rotate(box2.theta*DEG_TO_RAD);
		a[3]=Vector2(0,1).rotate(box2.theta*DEG_TO_RAD);

		Vector2 vert1[4],vert2[4];

		Vector2 rotpos1=position.rotation(position+center,theta*DEG_TO_RAD);
		Vector2 rotpos2=box2.position.rotation(box2.position+box2.center,box2.theta*DEG_TO_RAD);

		vert1[0]=rotpos1;vert1[1]=rotpos1+dimension.x*a[0];
		vert1[2]=rotpos1+dimension.x*a[0]+dimension.y*a[1];vert1[3]=rotpos1+dimension.y*a[1];

		vert2[0]=rotpos2;vert2[1]=rotpos2+box2.dimension.x*a[2];
		vert2[2]=rotpos2+box2.dimension.x*a[2]+box2.dimension.y*a[3];vert2[3]=rotpos2+box2.dimension.y*a[3];

		for(int i=0;i<4;i++)
		{	
			a[i]=a[i].normal();
			a[i].normalize();
			if(SAT(vert1,vert2,a[i]))
				return false;
		}
		return true;
	}


	void update()
	{
		position+=velocity;
		theta+=angular_velocity;
		scale+=scale_speed;

		if(scale>scale_max || scale<scale_min)
			scale_speed*=-1;

		if(theta>360)
			theta-=360;
		if(theta<0)
			theta+=360;
	}

	void draw(int r=255,int g=255,int b=255)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(position.x+center.x,position.y+center.y,0);
		glRotatef(theta,0,0,1);
		//glScalef(scale,scale,1);
		draw_fill_quad(-center.x,-center.y,dimension.x,dimension.y,r,g,b);
		sprite.set_texture(box_tex_test,50,50);
		//sprite.draw(-center.x,-center.y,dimension.x,dimension.y);
		glPopMatrix();
	}

	Vector2 position;
	Vector2 dimension;
	Vector2 center;//rotation

	float scale;//scalabe boxes
	float theta;

	Vector2 velocity;
	float angular_velocity;
	float scale_speed;
	float scale_max;
	float scale_min;

	Sprite sprite;
};

struct Player
{
	enum{ALIVE,DEAD,DYING};//player state

	Player()
	{
		state=ALIVE;
	}

	Player(float x,float y,float w,float h,float a=0):box(Box(x,y,w,h,a))
	{
		position=Vector2(x,y);dimension=Vector2(w,h);
		state=ALIVE;
	}

	void init(float x,float y,float w,float h,float a=0)
	{
		position=Vector2(x,y);dimension=Vector2(w,h);
		state=ALIVE;
		box.init(x,y,w,h,a);
	}

	void draw()
	{
		sprite.draw(position.x,position.y,dimension.x,dimension.y);
	}

	Vector2 position;
	Vector2 velocity;
	Vector2 dimension;
	Box box;
	Sprite sprite;

	int state;
};

struct Laser_Desc
{
	Laser_Desc()
	{
		angular_velocity=0.0f;
		firing=false;
	}

	void set_colors(int fr,int fg,int fb,int tr,int tg,int tb)
	{
		fred=fr;tred=tr;
		fgreen=fg;tgreen=tg;
		fblue=fb;tblue=tb;
	}

	Vector2 position;
	Vector2 direction;

	float angular_velocity;
	float fire_interval;
	float hold_interval;

	int fred,fgreen,fblue,tred,tgreen,tblue;

	bool firing;
};

struct Laser
{
	Laser()
	{
	}

	Laser(Laser_Desc lp)
	{
		position=lp.position;
		direction=lp.direction;
		direction.normalize();
		end_point=position+10000*direction;
		fire_timer=0;
		hold_timer=0;
		fire_interval=lp.fire_interval;
		hold_interval=lp.hold_interval;
		firing=lp.firing;
		angular_velocity=lp.angular_velocity;
		theta=0.0f;
		theta_min=-1;
		theta_max=361;
		set_colors(lp.fred,lp.fgreen,lp.fgreen,lp.tred,lp.tgreen,lp.tgreen);
	}

	Laser(float x,float y,float dx,float dy,float fi,float hi)
	{
		position=Vector2(x,y);direction=Vector2(dx,dy);direction.normalize();
		end_point=position+10000*direction;
		fire_timer=0;
		hold_timer=0;
		fire_interval=fi;
		hold_interval=hi;
		firing=false;
		angular_velocity=0.0f;
		theta=0.0f;
		theta_min=-1;
		theta_max=361;
	}

	void init(float x,float y,float dx,float dy,float fi,float hi)
	{
		position=Vector2(x,y);direction=Vector2(dx,dy);direction.normalize();
		end_point=position+10000*direction;
		fire_timer=0;
		hold_timer=0;
		fire_interval=fi;
		hold_interval=hi;
		firing=false;
		angular_velocity=0.0f;
		theta=361;
		theta_min=-1;
		theta_max=361;
	}

	void init(Laser_Desc lp)
	{
		position=lp.position;
		direction=lp.direction;
		direction.normalize();
		end_point=position+10000*direction;
		fire_timer=0;
		hold_timer=0;
		fire_interval=lp.fire_interval;
		hold_interval=lp.hold_interval;
		firing=lp.firing;
		angular_velocity=lp.angular_velocity;
		theta=0.0f;
		theta_min=-1;
		theta_max=361;
		set_colors(lp.fred,lp.fgreen,lp.fblue,lp.tred,lp.tgreen,lp.tblue);
	}

	void set_colors(int fr,int fg,int fb,int tr,int tg,int tb)
	{
		fred=fr;tred=tr;
		fgreen=fg;tgreen=tg;
		fblue=fb;tblue=tb;
	}

	void set_angular_limits(float min,float max)
	{
		theta_min=min;
		theta_max=max;
		theta=min+1;
	}

	void visual_update(float dt)
	{

	}

	void update(float dt)
	{
		if(firing)
		{
			fire_timer+=dt;
			if(fire_timer>fire_interval)
			{
				firing=false;
				hold_timer=0;
			}
		}
		else
		{
			hold_timer+=dt;
			if(hold_timer>hold_interval)
			{
				firing=true;
				fire_timer=0;
			}
		}

		direction.rotate(angular_velocity*DEG_TO_RAD);
		theta+=angular_velocity;
		if(theta>360)
			theta-=360;
		if(theta<0)
			theta+=360;

	}

	void draw(int r=255,int g=255,int b=255)
	{
		if(firing)
			draw_line(position.x,position.y,end_point.x,end_point.y,3,
			lerp(fred,tred,fire_timer/fire_interval),lerp(fgreen,tgreen,fire_timer/fire_interval),lerp(fblue,tblue,fire_timer/fire_interval));

		//draw basement(luncher)
		draw_fill_quad(position.x-10,position.y-10,20,20,0,0,255);

	}

	Vector2 position;
	Vector2 direction;
	Vector2 end_point;

	float theta;
	float theta_min;
	float theta_max;

	float angular_velocity;
	float fire_timer;
	float fire_interval;
	float hold_timer;
	float hold_interval;

	int fred,fgreen,fblue,tred,tgreen,tblue;

	bool firing;
};

Player player;
Input input;

struct Level_Desc
{
	Box end;

	Vector2 limits;
	Vector2 player_start;

	float dying_time;
	float ending_time;

	char name[100];
};

struct Level
{
	enum{DEATH,PLAYING,FINISHED,ENDING};

	Level(Level_Desc ld)
	{
		num_boxes=0;
		num_lasers=0;

		end=ld.end;

		dying_time=ld.dying_time;
		ending_time=ld.ending_time;

		limits=ld.limits;
		player_start=ld.player_start;

		dying_timer=0;
		ending_timer=0;

		state=PLAYING;
		level_time=0;

		strcpy(name,ld.name);
	}

	void add_laser(Laser_Desc lp)
	{
		if(num_lasers>=MAX_LASERS)
			return;

		lasers[num_lasers].init(lp);
		num_lasers++;
	}

	void add_laser_star(Laser_Desc lp)
	{
		if(num_lasers>=MAX_LASERS-4)
			return;

		lasers[num_lasers++].init(lp);
		lp.direction.rotate(90*DEG_TO_RAD);
		lasers[num_lasers++].init(lp);
		lp.direction.rotate(90*DEG_TO_RAD);
		lasers[num_lasers++].init(lp);
		lp.direction.rotate(90*DEG_TO_RAD);
		lasers[num_lasers++].init(lp);
	}

	void add_box(float x,float y,float dx,float dy,float a=0,float av=0,float vx=0,float vy=0)
	{
		if(num_boxes>=MAX_LASERS)
			return;

		boxes[num_boxes].init(x,y,dx,dy,a);
		boxes[num_boxes].velocity=Vector2(vx,vy);
		boxes[num_boxes].angular_velocity=av;
		num_boxes++;
	}

	void add_box(Box_Desc bd)
	{
		if(num_boxes>=MAX_LASERS)
			return;

		boxes[num_boxes].init(bd);
		num_boxes++;
	}

	void update(float dt)
	{
		if(state==PLAYING)
			level_time+=dt;

		//player input
		if(input.left)
		{
			if(player.velocity.x<=0)
				player.velocity.x-=XSPEED;
			else if(player.velocity.x>0)
				player.velocity.x-=XBREAK+(input.left_hold_time/1000)*TIME_BREAK_COEFF;
		}
		if(input.right)
		{
			if(player.velocity.x>=0)
				player.velocity.x+=XSPEED;
			else if(player.velocity.x<0)
				player.velocity.x+=XBREAK+(input.right_hold_time/1000)*TIME_BREAK_COEFF;
		}
		if(input.up)
		{
			if(player.velocity.y<=0)
				player.velocity.y-=YSPEED;
			else if(player.velocity.y>0)
				player.velocity.y-=YBREAK+(input.up_hold_time/1000)*TIME_BREAK_COEFF;
		}
			
		if(input.down)
		{
			if(player.velocity.y>=0)
				player.velocity.y+=YSPEED;
			else if(player.velocity.y<0)
				player.velocity.y+=YBREAK+(input.down_hold_time/1000)*TIME_BREAK_COEFF;
		}


		if(state==PLAYING)
		{
			if(player.state==Player::ALIVE)
			{
				player.position+=player.velocity;
				player.box.position=player.position;

				for(int i=0;i<num_boxes;i++)
				{
					boxes[i].update();
					if(player.box.collide(boxes[i]))
					{
						player.state=Player::DYING;
						break;
					}
				}

				//OBS VS OBS Collision
				for(int i=0;i<num_boxes;i++)
				{
					for(int j=i+1;j<num_boxes;j++)
					{
						if(boxes[i].collide(boxes[j]))
						{
							boxes[i].velocity*=-1;
							boxes[j].velocity*=-1;
						}
					}
				}

				for(int i=0;i<num_lasers;i++)
				{
					lasers[i].update(dt);

					float contact=2;
					lasers[i].end_point.x=lasers[i].position.x+lasers[i].direction.x*MAX(limits.x,limits.y);
					lasers[i].end_point.y=lasers[i].position.y+lasers[i].direction.y*MAX(limits.x,limits.y);

					for(int j=0;j<num_boxes;j++)
					{
						float c;
						if(collide(&lasers[i],&boxes[j],&c))
						{
							if(c<contact)
								contact=c;
						}
					}

					lasers[i].end_point=lasers[i].position+(lasers[i].end_point-lasers[i].position)*contact;

					if(lasers[i].firing && lasers[i].fire_timer>dt && collide(&lasers[i],&player.box))//the player have to see the beam getting through him!
					{
						player.state=Player::DYING;
					}
				}

				//end box update
				end.update();

				if(player.box.collide(end))
				{
					state=ENDING;
				}

			}
			else if(player.state==Player::DYING)
			{
				dying_timer+=dt;
				if(dying_timer>dying_time)
				{
					player.state=Player::DEAD;
					state=DEATH;
					dying_timer=0;
				}
			}
		}
		else if(state==ENDING)
		{
			ending_timer+=dt;
			if(ending_timer>ending_time)
			{
				state=FINISHED;
			}
		}
	}


	void draw()
	{
		if(state==PLAYING)
		{
			float xscroll=MAX(0,MIN(player.position.x,limits.x-SW/2)-SW/2);
			float yscroll=MAX(0,MIN(player.position.y,limits.y-SH/2)-SH/2);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glTranslatef(-xscroll,-yscroll,0);

			//draw player
			//draw_fill_quad(player.position.x,player.position.y,player.dimension.x,player.dimension.y,0,255,0);
			player.draw();

			//draw obstacles
			for(int i=0;i<num_boxes;i++)
			{
				boxes[i].draw(255,0,0);
			}

			//draw lasers
			for(int i=0;i<num_lasers;i++)
				lasers[i].draw();

			//draw end
			end.draw();



			//draw_time
			char time[100];
			sprintf(time,"%d : %d",int(level_time/1000),((int)level_time%1000)/10);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			draw_text(time,SW-200,0,255,255,255,2);

			glPopMatrix();
		}
		else if(state==ENDING)
		{
			//draw end
			float xscroll=MAX(0,MIN(player.position.x,limits.x-SW/2)-SW/2);
			float yscroll=MAX(0,MIN(player.position.y,limits.y-SH/2)-SH/2);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glTranslatef(-xscroll,-yscroll,0);

			//draw player
			//draw_fill_quad(player.position.x,player.position.y,player.dimension.x,player.dimension.y,0,255,0);
			player.draw();

			//draw obstacles
			for(int i=0;i<num_boxes;i++)
			{
				boxes[i].draw(255,0,0);
			}

			for(int i=0;i<num_lasers;i++)
				lasers[i].draw();

			end.draw();


			glLoadIdentity();
			draw_fill_quad(0,0,lerp(0,SW+200,ending_timer/ending_time),lerp(0,SH+200,ending_timer/ending_time));

			glPopMatrix();

		}
	}

	Laser lasers[MAX_LASERS];
	Box boxes[MAX_BOXES];
	Box end;

	Vector2 limits;
	Vector2 player_start;

	int num_boxes;
	int num_lasers;

	float dying_timer;
	float dying_time;

	float level_time;

	float ending_time;
	float ending_timer;

	int state;

	char name[100];
};


int num_levels=0;
const int num_official_levels=7;
Level* levels[MAX_LEVELS];
int high_scores[MAX_LEVELS];

//level creation functions

Level* create_lvl1();
Level* create_lvl2();
Level* create_lvl3();
Level* create_lvl4();
Level* create_lvl5();
Level* create_lvl6();
Level* create_lvl7();
//Level* create_lvl8();
//Level* create_lvl9();
//Level* create_lvl10();
//Level* create_lvl11();
//Level* create_lvl12();
//Level* create_lvl13();
//Level* create_lvl14();
//Level* create_lvl15();

enum{MENU,LVL_PANEL,GAME,END_GAME};


int main( int argc, char* args[] )
{
	SDL_Init( SDL_INIT_VIDEO );
	SDL_SetVideoMode(SW, SH, 0, SDL_OPENGL);
	SDL_Event ev;

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D(0,SW,SH,0);

	int quit=0;float curr=0,dt;
	float acc=0;
	const float frame_time=1000/60;
	int game_state=LVL_PANEL;

	font=loadTexture("font.jpg");

	int  texw,texh;

	int player_texture=loadTexture("player.png",&texw,&texh);

	player.init(120,400,20,20,0);
	player.sprite.set_texture(player_texture,texw,texh);


	box_tex_test=loadTexture("mosaik_1.png");
	glBindTexture(GL_TEXTURE_2D,box_tex_test);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	

	//Score file creation and init!
	/*for(int i=0;i<num_levels;i++)
		high_scores[i]=5000;

	save_high_scores();*/


	levels[num_levels++]=create_lvl1();
	levels[num_levels++]=create_lvl2();
	levels[num_levels++]=create_lvl3();
	levels[num_levels++]=create_lvl4();
	levels[num_levels++]=create_lvl5();
	levels[num_levels++]=create_lvl6();
	levels[num_levels++]=create_lvl7();

	load_high_scores();

	load_user_levels();

	Level current_level=*levels[0];
	int current_level_id=0;
	int selected_level=0;
	int new_high_score=0;

	float lvl_selection_outline_size=1;
	float lvl_selection_outline_grow=0.5;

	player.position=current_level.player_start;

	int last_down=0,last_up=0,last_escape=0;
	while(!quit)
	{
		dt=SDL_GetTicks()-curr;
		curr=SDL_GetTicks();
		acc+=dt;

		while(SDL_PollEvent(&ev))
		{
				if(ev.type==SDL_QUIT)
					quit=1;

				input.update(ev);
		}

		while(acc>frame_time)//independant update
		{
			//time-related input stuff!
			input.time_update(frame_time);

			if(game_state==GAME)
			{
				if(input.escape && !last_escape)
				{
					//return to level selection
					game_state=LVL_PANEL;
					selected_level=0;
				}

				if(current_level.state==Level::DEATH)
				{
					//re_init the level
					if(input.r)
					{
						current_level=*levels[current_level_id];
						player.position=current_level.player_start;
						player.state=Player::ALIVE;
						player.velocity=Vector2(0,0);
					}
				}
				else if(current_level.state==Level::FINISHED)
				{
					if(input.space)
					{
						//move to next level
						current_level_id++;
						if(current_level_id==num_levels)
							game_state=END_GAME;
						else
						{
						current_level=*levels[current_level_id];
						player.position=current_level.player_start;
						player.state=Player::ALIVE;
						player.velocity=Vector2(0,0);
						new_high_score=0;
						}
					}
					//re_init the level
					if(input.r)
					{
						current_level=*levels[current_level_id];
						player.position=current_level.player_start;
						player.state=Player::ALIVE;
						player.velocity=Vector2(0,0);
						new_high_score=0;
					}
				}
				else
					current_level.update(frame_time);
			}
			else if(game_state==LVL_PANEL)
			{
				lvl_selection_outline_size+=lvl_selection_outline_grow;

				if(lvl_selection_outline_size<1 || lvl_selection_outline_size>=10)
					lvl_selection_outline_grow*=-1;

				if(input.down && !last_down)
				{
					selected_level++;
					if(selected_level==num_levels)
						selected_level=0;		
				}
				if(input.up && !last_up)
				{
					selected_level--;
					if(selected_level==-1)
						selected_level=num_levels-1;		
				}

				if(input.enter)
				{
					current_level=*levels[selected_level];
					current_level_id=selected_level;
					player.position=current_level.player_start;
					player.state=Player::ALIVE;
					player.velocity=Vector2(0,0);
					game_state=GAME;
				}

				if(input.escape && !last_escape)
				{
					//return to menu
					quit=1;
				}
			}

			last_down=input.down;
			last_up=input.up;
			last_escape=input.escape;

			acc-=frame_time;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		if(game_state==GAME)
		{
			if(current_level.state==Level::FINISHED)
			{
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
				draw_fill_quad(0,0,SW,SH);
				draw_text("LEVEL  COMPLETE!",SW/2-350,SH/2-200,0,255,0,4);
				char time[100];

				if(current_level.level_time<high_scores[current_level_id])
				{
					//new high score!
					high_scores[current_level_id]=current_level.level_time;
					save_high_scores();
					new_high_score=1;
				}

				if(new_high_score)
				{
					sprintf(time,"NEW BEST TIME:  %d : %d",int(current_level.level_time/1000),((int)current_level.level_time%1000)/10);
					draw_text(time,SW/2-350,SH/2,255,0,0,2.5);
				}
				else
				{
					sprintf(time,"Your time:   %d : %d",int(current_level.level_time/1000),((int)current_level.level_time%1000)/10);
					draw_text(time,SW/2-350,SH/2,255,0,0,2);
					sprintf(time,"Record time:   %d : %d",int(high_scores[current_level_id]/1000),((int)high_scores[current_level_id]%1000)/10);
					draw_text(time,SW/2-350,SH/2+100,255,0,0,2);
				}

				draw_text("Hit 'space' to continue",SW/2-350,SH/2+200,0,0,255,1);
				draw_text("Hit 'R' to retry",SW/2+100,SH/2+200,0,0,255,1);

				glPopMatrix();

			}
			else if(current_level.state==Level::DEATH)
			{
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
				draw_text("DEAD!",SW/2-250,SH/2-100,255,0,0,10);
				draw_text("Hit 'R' to retry",SW/2-100,SH/2+200,255,255,0,1);
				glPopMatrix();

			}
			else
				current_level.draw();
		}
		else if(game_state==LVL_PANEL)
		{
			float adx=((selected_level>=num_official_levels)?1:0)*140;
			float yscroll=MAX(0,MIN(adx+selected_level*140+200,num_levels*140-SH/2+400)-SH/2);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();


			glTranslatef(SW/2-200,-yscroll+200,0);

			char label[10];
			for(int i=0;i<num_levels;i++)
			{	
				if(i==num_official_levels)
				{
					draw_text("YOUR LEVELS",350-SW/2,i*140,255,255,255,4);
					glTranslatef(0,100,0);
				}

				if(i==selected_level)
				{
					int r=lerp(0,250,lvl_selection_outline_size/10);
					int g=lerp(0,250,lvl_selection_outline_size/10);
					int b=lerp(0,250,lvl_selection_outline_size/10);
					draw_quad(-5,i*140-5,440,120,r,g,b,lvl_selection_outline_size);
				}

				sprintf(label,"Level %d",i);

				draw_text(label,120,i*140,255,0,0,2);

				draw_text(levels[i]->name,5,i*140+55,0,255,0,2);
			}
		
			glLoadIdentity();

			//draw head
			draw_fill_quad(0,0,SW,100,0,0,0);
			draw_text("LEVEL SELECT",150,25,255,255,255,4);
			glPopMatrix();
		}

		glFlush();
		SDL_GL_SwapBuffers();
	}

	SDL_Quit();

	return 0;
}




Level* create_lvl1()
{
	Level_Desc ld;

	ld.end.init(1100,150,20,20,45);
	ld.end.angular_velocity=5;
	ld.end.center=0.5*ld.end.dimension;

	ld.limits=Vector2(1300,1000);
	ld.player_start=Vector2(250,800);
	ld.dying_time=500;
	ld.ending_time=500;
	strcpy(ld.name,"Nice to meet you");
	Level* lvl1=new Level(ld);


	lvl1->add_box(0,0,100,900);
	lvl1->add_box(100,0,1100,100);
	lvl1->add_box(1200,0,100,900);
	lvl1->add_box(300,300,700,400);
	lvl1->add_box(0,900,1300,200);

	return lvl1;
}

Level* create_lvl2()
{
	Level_Desc ld;

	ld.end.init(1300,250,20,20,45);
	ld.end.angular_velocity=5;
	ld.end.center=0.5*ld.end.dimension;

	ld.limits=Vector2(1500,1065);
	ld.player_start=Vector2(250,800);
	ld.dying_time=500;
	ld.ending_time=500;
	strcpy(ld.name,"Diagonal");

	Level* lvl1=new Level(ld);


	lvl1->add_box(0,0,500,665);
	lvl1->add_box(0,665,100,500);
	lvl1->add_box(100,965,400,100);

	lvl1->add_box(359-2*141-1,524-2*141-1,800,600,-45);
	lvl1->add_box(400,965,1000,900,-45);

	lvl1->add_box(1065,300,400,800);
	lvl1->add_box(924,0,665,150);
	lvl1->add_box(1365,0,150,800);


	return lvl1;
}

Level* create_lvl3()
{
	Level_Desc ld;

	ld.end.init(SW/2-250+350,SH/2-250+350,20,20,45);
	ld.end.angular_velocity=5;
	ld.end.center=0.5*ld.end.dimension;

	ld.limits=Vector2(700,700);
	ld.player_start=Vector2(SW/2-250+150,SH/2-250+150);
	ld.dying_time=500;
	ld.ending_time=500;
	strcpy(ld.name,"Light Amplification-SERS");

	Level* lvl1=new Level(ld);

	Laser_Desc lp;

	lp.position=Vector2(SW/2-250+101,SH/2-250+200);
	lp.direction=Vector2(1,0);
	lp.set_colors(255,255,255,255,0,0);
	lp.fire_interval=700;
	lp.hold_interval=700;

	lvl1->add_laser(lp);

	lp.position=Vector2(SW/2-250+399,SH/2-250+300);
	lp.direction=Vector2(-1,0);
	lp.set_colors(255,255,255,255,0,0);
	lp.fire_interval=800;
	lp.hold_interval=800;
	lp.firing=true;

	lvl1->add_laser(lp);

	lvl1->add_box(0,0,SW/2-250+700,SH/2-250+100);
	lvl1->add_box(0,0,SW/2-250+100,SH/2-250+500);
	lvl1->add_box(0,SH/2-250+400,2000,2000);
	lvl1->add_box(SW/2-250+400,0,2000,SH/2-250+500);


	return lvl1;
}

Level* create_lvl4()
{
	Level_Desc ld;

	ld.end.init(980,SH/2-200+250,20,20,45);
	ld.end.angular_velocity=5;
	ld.end.center=0.5*ld.end.dimension;

	ld.limits=Vector2(1150,400);
	ld.player_start=Vector2(150,SH/2-200+150);
	ld.dying_time=500;
	ld.ending_time=500;
	strcpy(ld.name,"Piston Movement");

	Level* lvl1=new Level(ld);

	Laser_Desc lp;

	lp.position=Vector2(1050-1,SH/2-200+200);
	lp.direction=Vector2(-1,0);
	lp.set_colors(255,255,255,0,255,0);
	lp.fire_interval=700;
	lp.hold_interval=0;

	lvl1->add_laser(lp);

	lvl1->add_box(0,0,1050,SH/2-200+100);
	lvl1->add_box(0,SH/2-200+300,1050,500);
	lvl1->add_box(0,0,100,SH/2-200+400);
	lvl1->add_box(1050,0,100,SH/2-200+400);

	lvl1->add_box(350,SH/2-200+300-50*GOLDEN_RATIO,50,50*GOLDEN_RATIO,0,0,0,-3.5);
	lvl1->add_box(500,SH/2-200+101,50,50*GOLDEN_RATIO,0,0,0,3.5);
	lvl1->add_box(650,SH/2-200+300-50*GOLDEN_RATIO,50,50*GOLDEN_RATIO,0,0,0,-3.5);
	lvl1->add_box(800,SH/2-200+101,50,50*GOLDEN_RATIO,0,0,0,3.5);

	return lvl1;
}

Level* create_lvl5()
{
	Level_Desc ld;

	ld.end.init(SW/2-250+350,SH/2-250+350,20,20,45);
	ld.end.angular_velocity=5;
	ld.end.center=0.5*ld.end.dimension;

	ld.limits=Vector2(700,700);
	ld.player_start=Vector2(SW/2-250+150,SH/2-250+150);
	ld.dying_time=500;
	ld.ending_time=500;
	strcpy(ld.name,"Flower");

	Level* lvl1=new Level(ld);

	Laser_Desc lp;

	lp.position=Vector2(SW/2,SH/2);
	lp.direction=Vector2(1,0);
	lp.set_colors(255,255,255,0,0,255);
	lp.fire_interval=700;
	lp.hold_interval=0;
	lp.angular_velocity=0.5;

	lvl1->add_laser_star(lp);

	lvl1->add_box(0,0,SW/2-250+700,SH/2-250+100);
	lvl1->add_box(0,0,SW/2-250+100,SH/2-250+500);
	lvl1->add_box(0,SH/2-250+400,2000,2000);
	lvl1->add_box(SW/2-250+400,0,2000,SH/2-250+500);


	return lvl1;
}

Level* create_lvl6()
{
	Level_Desc ld;

	ld.end.init(SW/2,950,20,20,45);
	ld.end.angular_velocity=5;
	ld.end.center=0.5*ld.end.dimension;

	ld.limits=Vector2(SW,1100);
	ld.player_start=Vector2(SW/2,150);
	ld.dying_time=500;
	ld.ending_time=500;
	strcpy(ld.name,"Double Flower");

	Level* lvl1=new Level(ld);

	Laser_Desc lp;

	lp.position=Vector2(SW/2,450);
	lp.direction=Vector2(1,0);
	lp.set_colors(255,255,255,0,0,255);
	lp.fire_interval=700;
	lp.hold_interval=0;
	lp.angular_velocity=1;

	lvl1->add_laser_star(lp);

	lp.position=Vector2(SW/2,725);
	lp.direction=Vector2(1,1);
	lp.angular_velocity=-1;
	lvl1->add_laser_star(lp);

	lvl1->add_box(SW/2-35*GOLDEN_RATIO,300,70*GOLDEN_RATIO,25);
	lvl1->add_box(SW/2-35*GOLDEN_RATIO,575,70*GOLDEN_RATIO,25);


	lvl1->add_box(0,0,SW,100);
	lvl1->add_box(0,100,SW/2-175,925);
	lvl1->add_box(SW/2-175+350,100,600,925);
	lvl1->add_box(0,975,SW,100);

	return lvl1;
}

Level* create_lvl7()
{
	Level_Desc ld;

	ld.end.init(900,SH/2,20,20,45);
	ld.end.angular_velocity=5;
	ld.end.center=0.5*ld.end.dimension;

	ld.limits=Vector2(1200,SH);
	ld.player_start=Vector2(250,SH/2);
	ld.dying_time=500;
	ld.ending_time=500;
	strcpy(ld.name,"Lava eruption !");

	Level* lvl1=new Level(ld);

	/*bd.position=Vector2(SW/2-100,SH/2);
	bd.dimension=Vector2(20,20);
	bd.center=Vector2(150,0);
	bd.angular_velocity=5;
	bd.velocity=Vector2(0,0);
	bd.theta=0;*/

	/*Laser_Desc lp;

	lp.position=Vector2(1000-1,SH/2-50);
	lp.direction=Vector2(-1,0);
	lp.set_colors(255,255,255,0,0,255);
	lp.fire_interval=700;
	lp.hold_interval=0;

	lvl1->add_laser(lp);

	lp.position=Vector2(1000-1,SH/2+50);
	lp.set_colors(255,255,255,255,255,0);
	lvl1->add_laser(lp);*/

	Box_Desc bd;
	bd.position=Vector2(350,SH/2-100);
	bd.dimension=Vector2(25,25);
	bd.center=Vector2(150,0);
	bd.angular_velocity=5;
	bd.velocity=Vector2(0,0);
	bd.theta=0;
	lvl1->add_box(bd);

	bd.position=Vector2(350,SH/2+100);
	bd.angular_velocity=-5;
	lvl1->add_box(bd);

	bd.position=Vector2(500,SH/2-100);
	bd.angular_velocity=5;
	lvl1->add_box(bd);

	bd.position=Vector2(500,SH/2+100);
	bd.angular_velocity=-5;
	lvl1->add_box(bd);

	lvl1->add_box(0,0,1000,SH/2-100);
	lvl1->add_box(0,0,200,SH);
	lvl1->add_box(0,SH/2+100,1000,SH);
	lvl1->add_box(1000,0,1000,SH);
	return lvl1;
}

bool collide(Laser* l,Box* b,float* contact)
{
	Vector2 a[3];


	a[0]=Vector2(1,0).rotate(b->theta*DEG_TO_RAD);
	a[1]=Vector2(0,1).rotate(b->theta*DEG_TO_RAD);
	a[2]=l->direction;

	Vector2 vert1[4],vert2[4];

	Vector2 rotpos1=b->position.rotation(b->position+b->center,b->theta*DEG_TO_RAD);

	vert1[0]=l->position;vert1[1]=l->end_point;
	vert1[2]=l->position;vert1[3]=l->end_point;

	vert2[0]=rotpos1;vert2[1]=rotpos1+b->dimension.x*a[0];
	vert2[2]=rotpos1+b->dimension.x*a[0]+b->dimension.y*a[1];vert2[3]=rotpos1+b->dimension.y*a[1];


	for(int i=0;i<3;i++)
	{	
		a[i]=a[i].normal();
		a[i].normalize();
		if(SAT(vert1,vert2,a[i]))
			return false;
	}

	if(contact)
	{
		float tnear=0;
		for(int j=3,i=0; i<4 ; j=i,i++)
		{
			Vector2 E0 = vert2[j];
			Vector2 E1 = vert2[i];
			Vector2 E  = E1 - E0;
			Vector2 En(E.y, -E.x);
			Vector2 D = E0 - l->position;

			float denom = D * En;
			float numer = (l->end_point-l->position) * En;

			if (fabs(numer) < 1.0E-8f)
				continue;

			float tclip = denom / numer;
			if (numer < 0.0f && tclip > tnear)
				tnear = tclip;

		}

		*contact=tnear;
	}

	return true;

}

void draw_fill_quad(float x,float y,float w,float h,int r,int g,int b)
{
	glBegin(GL_QUADS);

	glColor3ub(r,g,b);
	glVertex2f(x,y);
	glVertex2f(x+w,y);
	glVertex2f(x+w,y+h);
	glVertex2f(x,y+h);

	glEnd();

	glColor4f(1, 1, 1, 1);
}

void draw_quad(float x,float y,float w,float h,int r,int g,int b,int size)
{
	glLineWidth(size);
	glBegin(GL_LINE_LOOP);

	glColor3ub(r,g,b);
	glVertex2f(x,y);
	glVertex2f(x+w,y);
	glVertex2f(x+w,y+h);
	glVertex2f(x,y+h);

	glEnd();

	glColor4f(1, 1, 1, 1);
}

void draw_line(float x1,float y1,float x2,float y2,int width,int r,int g,int b)
{
	glLineWidth(width);
	glBegin(GL_LINES);

	glColor3ub(r,g,b);
	glVertex2f(x1,y1);
	glVertex2f(x2,y2);

	glEnd();

	glColor4f(1, 1, 1, 1);
}

void draw_cara(char c,int ofx,int ofy)
{
	int cx=(c%16)*WCARA,cy=(c/16)*HCARA;

	//printf("%f  %d   %f\n",(float)cx/256,(c),(float)cy/256);
	//the font texture is 256/256
	glTexCoord2f((float)cx/256,(float)cy/256);
	glVertex2f(ofx,ofy);

	glTexCoord2f((float)(cx+WCARA)/256,(float)cy/256);
	glVertex2f(ofx+WCARA,ofy);

	glTexCoord2f((float)(cx+WCARA)/256,(float)(cy+HCARA)/256);
	glVertex2f(ofx+WCARA,ofy+HCARA);

	glTexCoord2f((float)cx/256,(float)(cy+HCARA)/256);
	glVertex2f(ofx,ofy+HCARA);
}

void draw_text(char* str,int x,int y,int r,int g,int b,int scale)
{
	int xi=x;//initial x
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(scale,scale,scale);

	x/=scale;
	y/=scale;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,font);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBegin(GL_QUADS);
	glColor3ub(r,g,b);
	for(int i=0;str[i];i++)
	{
		if(str[i]=='\n')
		{
			y+=HCARA;
			x=xi;
		}
		else
		{
			draw_cara(str[i],x,y);
			x+=WCARA;
		}

	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

bool SAT(Vector2* c1Vert,Vector2* c2Vert,Vector2 ax)
{
	float min1=c1Vert[0].dot(ax),max1=min1;
	float min2=c2Vert[0].dot(ax),max2=min2;


	for(int i=1;i<4;i++)
	{
		float proj1=c1Vert[i].dot(ax),proj2=c2Vert[i].dot(ax);
		if(proj1<min1)min1=proj1;
		else if(proj1>max1)max1=proj1;
		if(proj2<min2)min2=proj2;
		else if(proj2>max2)max2=proj2;
	}
	if(max1<min2 || max2<min1)
		return true;

	return false;
}

float lerp(float a,float b,float t)
{
	return a+t*(b-a);
}

void load_high_scores()
{
	FILE* file=fopen("times.sav","rb");
	fread(high_scores,sizeof(int),num_levels,file);
}

void save_high_scores()
{
	FILE* file=fopen("times.sav","wb");
	fwrite(high_scores,sizeof(int),num_levels,file);
}


void load_user_levels()
{
	WIN32_FIND_DATA ffd;
   LARGE_INTEGER filesize;
   size_t length_of_arg;
   HANDLE hFind = INVALID_HANDLE_VALUE;
   DWORD dwError=0;

   hFind = FindFirstFile(L"user_levels\\*", &ffd);

   if (hFind==INVALID_HANDLE_VALUE) 
      return;

   do
   {
      
      if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
		  //load level in file ffd.cFileName
		  char path[100];
		  char mbstr[100];
		  wcstombs(mbstr,ffd.cFileName,sizeof(mbstr));

		  strcpy(path,"user_levels\\");
		  strcat(path,mbstr);
		  load_level_file(path);
      }
   }
   while (FindNextFile(hFind, &ffd) != 0);
 
   FindClose(hFind);
}

void load_level_file(char* path)
{
	FILE* file=fopen(path,"rt");

	char line[400];
	fgets(line,400,file);

	if(strcmp(line,"BEGIN\n"))
		return;

	char prop_name[50];
	char ps[50];
	float pi1,pi2;

	Level_Desc ld;

	fscanf(file,"%s %s\n",prop_name,ps);
	if(strcmp(prop_name,"level_name"))
		return;
	strcpy(ld.name,ps);

	fscanf(file,"%s %f %f\n",prop_name,&pi1,&pi2);
	if(strcmp(prop_name,"player_start"))
		return;
	ld.player_start=Vector2(pi1,pi2);

	fscanf(file,"%s %f %f\n",prop_name,&pi1,&pi2);
	if(strcmp(prop_name,"level_limits"))
		return;
	ld.limits=Vector2(pi1,pi2);

	fscanf(file,"%s %f %f\n",prop_name,&pi1,&pi2);
	if(strcmp(prop_name,"level_end_position"))
		return;

	ld.end.init(pi1,pi2,20,20,45);
	ld.end.angular_velocity=5;
	ld.end.center=0.5*ld.end.dimension;

	ld.dying_time=500;
	ld.ending_time=500;

	Level* lvl=new Level(ld);

	while(fscanf(file,"%s",prop_name) && strcmp(prop_name,"END"))
	{
		//its a one line comment
				if(prop_name[0]==prop_name[1] && prop_name[0]=='/')
					fgets(line,400,file);
		if(!strcmp(prop_name,"BOX_BEGIN"))
		{
			Box_Desc bd;
			bd.angular_velocity=0;
			bd.theta=0;

			while(fscanf(file,"%s",prop_name) && strcmp(prop_name,"BOX_END"))
			{
				//its a one line comment
				if(prop_name[0]==prop_name[1] && prop_name[0]=='/')
					fgets(line,400,file);
				if(!strcmp(prop_name,"position"))
				{
					fscanf(file,"%f %f",&pi1,&pi2);
					bd.position=Vector2(pi1,pi2);
				}
				else if(!strcmp(prop_name,"dimension"))
				{
					fscanf(file,"%f %f",&pi1,&pi2);
					bd.dimension=Vector2(pi1,pi2);
				}
				else if(!strcmp(prop_name,"center"))
				{
					fscanf(file,"%f %f",&pi1,&pi2);
					bd.center=Vector2(pi1,pi2);
				}
				else if(!strcmp(prop_name,"angle"))
				{
					fscanf(file,"%f",&pi1);
					bd.theta=pi1;
				}
				else if(!strcmp(prop_name,"velocity"))
				{
					fscanf(file,"%f %f",&pi1,&pi2);
					bd.velocity=Vector2(pi1,pi2);
				}
				else if(!strcmp(prop_name,"angular_velocity"))
				{
					fscanf(file,"%f",&pi1);
					bd.angular_velocity=pi1;
				}
			}
			lvl->add_box(bd);
		}
		else if(!strcmp(prop_name,"LASER_BEGIN"))
		{
			Laser_Desc lsd;
			lsd.firing=true;
			lsd.hold_interval=0;
			lsd.angular_velocity=0;

			while(fscanf(file,"%s\n",prop_name) && strcmp(prop_name,"LASER_END"))
			{
				//its a one line comment
				if(prop_name[0]==prop_name[1] && prop_name[0]=='/')
					fgets(line,400,file);

				if(!strcmp(prop_name,"position"))
				{
					fscanf(file,"%f %f",&pi1,&pi2);
					lsd.position=Vector2(pi1,pi2);
				}
				else if(!strcmp(prop_name,"direction"))
				{
					fscanf(file,"%f %f",&pi1,&pi2);
					lsd.direction=Vector2(pi1,pi2);
				}
				else if(!strcmp(prop_name,"fire_interval"))
				{
					fscanf(file,"%f",&pi1);
					lsd.fire_interval=pi1;
				}
				else if(!strcmp(prop_name,"hold_interval"))
				{
					fscanf(file,"%f",&pi1);
					lsd.hold_interval=pi1;
				}
				else if(!strcmp(prop_name,"angular_velocity"))
				{
					fscanf(file,"%f",&pi1);
					lsd.angular_velocity=pi1;
				}
				else if(!strcmp(prop_name,"colors"))
				{
					int a,b,c,d,e,f;
					fscanf(file,"%d  %d  %d  %d  %d  %d\n",&a,&b,&c,&d,&e,&f);
					lsd.fred=a;
					lsd.fgreen=b;
					lsd.fblue=c;
					lsd.tred=d;
					lsd.tgreen=e;
					lsd.tblue=f;
				}
				else if(!strcmp(prop_name,"initial_state"))
				{
					int a;
					fscanf(file,"%d",&a);
					lsd.firing=a;
					a=lsd.firing;
				}
			}

			lvl->add_laser(lsd);
		}
		else if(!strcmp(prop_name,"LASER_STAR_BEGIN"))
		{
			Laser_Desc lsd;
			lsd.firing=true;
			lsd.angular_velocity=0;
			lsd.hold_interval=0;

			while(fscanf(file,"%s\n",prop_name) && strcmp(prop_name,"LASER_STAR_END"))
			{
				//its a one line comment
				if(prop_name[0]==prop_name[1] && prop_name[0]=='/')
					fgets(line,400,file);

				if(!strcmp(prop_name,"position"))
				{
					fscanf(file,"%f %f",&pi1,&pi2);
					lsd.position=Vector2(pi1,pi2);
				}else if(!strcmp(prop_name,"direction"))
				{
					fscanf(file,"%f %f",&pi1,&pi2);
					lsd.direction=Vector2(pi1,pi2);
				}
				else if(!strcmp(prop_name,"fire_interval"))
				{
					fscanf(file,"%f",&pi1);
					lsd.fire_interval=pi1;
				}

				else if(!strcmp(prop_name,"hold_interval"))
				{
					fscanf(file,"%f",&pi1);
					lsd.hold_interval=pi1;
				}


				else if(!strcmp(prop_name,"angular_velocity"))
				{
					fscanf(file,"%f",&pi1);
					lsd.angular_velocity=pi1;
				}

				else if(!strcmp(prop_name,"colors"))
				{
					int a,b,c,d,e,f;
					fscanf(file,"%d  %d  %d  %d  %d  %d\n",&a,&b,&c,&d,&e,&f);
					lsd.fred=a;
					lsd.fgreen=b;
					lsd.fblue=c;
					lsd.tred=d;
					lsd.tgreen=e;
					lsd.tblue=f;
				}

				else if(!strcmp(prop_name,"initial_state"))
				{
					int a;
					fscanf(file,"%d",&a);
					lsd.firing=pi1;
				}

			}
			
			lvl->add_laser_star(lsd);
		}
	}

	levels[num_levels++]=lvl;
}