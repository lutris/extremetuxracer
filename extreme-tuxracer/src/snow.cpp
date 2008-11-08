/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "snow.h"
#include "gl_util.h"
#include "course_render.h"
#include "render_util.h"
#include "game_config.h"
#include "player.h"
#include "textures.h"
#include "loop.h"
#include "ppgltk/alg/defs.h"
#include "part_sys.h"

static double XRANGE = 18;
static double YRANGE = 26;
static double ZRANGE = 27;

static double NEAR_XRANGE = 12;
static double NEAR_YRANGE = 16;
static double NEAR_ZRANGE = 12;

//default snowtype : light (decorative snow)

static double speed = 8;
static double minSize = 0.15; 
static double maxSize = 0.45;
static int MAXPART = 64;
static int MAXNEAR = 30;


static double xdrift = 1;
static double ydrift = 1;
static double zdrift = 1;
static double ZWindFactor = 0.0;
static double XWindFactor = 0.0;

#define MAXTYPES 8
static SnowType snowtypes[MAXTYPES];
static bool istyperegistered[MAXTYPES] = {false,false,false,false,false,false,false,false};

static pp::Vec3d lastPos;

typedef struct {
    pp::Vec3d pt;
    double size;
    pp::Vec3d vel;
    int index_tex;
    pp::Vec2d tex_min;
    pp::Vec2d tex_max;
} TParticle;

typedef struct {
	double left;
	double right;
	double bottom;
	double top;
	double front;
	double back;
} TArea;  // the cuboid for the snow flakes

TParticle* PartArr;
static TArea area;
TParticle* NearArr;
static TArea neararea;
static GLuint snow_tex[4];


static double xrand (double min, double max) {
		return (double)rand() / RAND_MAX * (max - min) + min; }
        
void UpdateArea(pp::Vec3d pos) {
    area.left = pos.x - XRANGE / 2;
	area.right = area.left + XRANGE;
	area.back = pos.z + ZRANGE/3;
	area.front = area.back - ZRANGE;
	area.top = pos.y + 0.5*YRANGE;
	area.bottom = area.top - YRANGE;

// some remarks about the neararea:
//============================

// If the size of the flakes is very low they won't appear dense enough
// If the flakes are sized very large they seem to be snowballs in the near area
// This problem can be solved by using two areas, one for the far and ond for the 
// near with smaller flakes

//if the Yarea doesnt go below the player's Z, then the player can see the snow ahead of him just stopping in midair
//if the Zarea doesnt go behind the player, the player can see that there is no snow behind, when he turns around

	neararea.left = pos.x - NEAR_XRANGE / 2;
	neararea.right = neararea.left + NEAR_XRANGE;
	neararea.back = pos.z + NEAR_ZRANGE/3;
	neararea.front = neararea.back - NEAR_ZRANGE;
	neararea.top = pos.y + 0.5*NEAR_YRANGE;
	neararea.bottom = neararea.top - NEAR_YRANGE;
    
    //std::cout << "[snow.cpp] Player position as passed to UpdateArea() : " << pos.x << " " << pos.y << " " << pos.z << "\n";
    //std::cout << "[snow.cpp] Updated snow area : " << neararea.left << " " << neararea.back << " " <<neararea.top  << "\n";
}

void RegisterSnowType (int index, SnowType type) {
    if((index < MAXTYPES) && (index > -1)) {
        snowtypes[index] = type;
        istyperegistered[index] = true;
    } else {
        std::cerr << "RegisterSnowType, index : " << index << " [ERROR] Index must be in range [0 .. " << MAXTYPES << "]\n";
    }
}

void SetSnowType (int index) {
    if((index < MAXTYPES) && (index > -1)) {
        if(istyperegistered[index]){
            speed = snowtypes[index].speed;
            minSize = snowtypes[index].minSize;
            maxSize = snowtypes[index].maxSize;
            MAXPART = snowtypes[index].MAXPART;
            MAXNEAR = snowtypes[index].MAXNEAR;
        } else {
            std::cerr << "SetSnowType, index : " << index << " [ERROR] No type has been registered with this index !\n";
        }
    } else {
        std::cerr << "RegisterSnowType, index : " << index << " [ERROR] Index must be in range [0 .. " << MAXTYPES << "]\n";
    }
}


static void MakeSnowParticle (int i) {

    double tmp;
    int type;
	PartArr[i].pt.x = xrand (area.left, area.right);  
	PartArr[i].pt.y = xrand (area.bottom, area.top);
	PartArr[i].pt.z = xrand(area.back, area.front);

    PartArr[i].size = xrand (minSize, maxSize);
	PartArr[i].vel.x = 0;
	PartArr[i].vel.y = -PartArr[i].size * speed;
	PartArr[i].vel.z = 0;	
    
    tmp = xrand(0.0,1.0) * (4.0 - EPS);
	PartArr[i].index_tex = (int)tmp;
    
    /*
    tmp = xrand(0.0,1.0) * (4.0 - EPS);
	type = (int)tmp;
	if (type == 0) {
		PartArr[i].tex_min = pp::Vec2d(0.0, 0.0);
		PartArr[i].tex_max = pp::Vec2d(0.5, 0.5);
    } else if (type == 1) {
		PartArr[i].tex_min = pp::Vec2d(0.5, 0.0);
		PartArr[i].tex_max = pp::Vec2d(1.0, 0.5);
    } else if (type == 2) {
		PartArr[i].tex_min = pp::Vec2d(0.5, 0.5);
		PartArr[i].tex_max = pp::Vec2d(1.0, 1.0);
    } else {
		PartArr[i].tex_min = pp::Vec2d(0.0, 0.5);
		PartArr[i].tex_max = pp::Vec2d(0.5, 1.0);
    }
    */
    PartArr[i].tex_min = pp::Vec2d(0.0, 0.0);
    PartArr[i].tex_max = pp::Vec2d(1.0, 1.0);
    
    //std::cout << "[snow.cpp] Particle, index " << i << " created at " << PartArr[i].pt.x << " " << PartArr[i].pt.y << " " << PartArr[i].pt.z << "\n";
}

static void MakeNearParticle (int i) {

    double tmp;
    int type;
	NearArr[i].pt.x = xrand (neararea.left, neararea.right);  
	NearArr[i].pt.y = xrand (neararea.bottom, neararea.top);
	NearArr[i].pt.z = xrand(neararea.back, neararea.front);

    NearArr[i].size = xrand (minSize / 6, maxSize / 6);
	NearArr[i].vel.x = 0;
	NearArr[i].vel.y = -NearArr[i].size * speed;
	NearArr[i].vel.z = 0;	
    
    tmp = xrand(0.0,1.0) * (4.0 - EPS);
	NearArr[i].index_tex = (int)tmp;
    
	/*tmp = xrand(0.0,1.0) * (4.0 - EPS);
	type = (int)tmp;
	if (type == 0) {
		NearArr[i].tex_min = pp::Vec2d(0.0, 0.0);
		NearArr[i].tex_max = pp::Vec2d(0.5, 0.5);
    } else if (type == 1) {
		NearArr[i].tex_min = pp::Vec2d(0.5, 0.0);
		NearArr[i].tex_max = pp::Vec2d(1.0, 0.5);
    } else if (type == 2) {
		NearArr[i].tex_min = pp::Vec2d(0.5, 0.5);
		NearArr[i].tex_max = pp::Vec2d(1.0, 1.0);
    } else {
		NearArr[i].tex_min = pp::Vec2d(0.0, 0.5);
		NearArr[i].tex_max = pp::Vec2d(0.5, 1.0);
    }*/
    NearArr[i].tex_min = pp::Vec2d(0.0, 0.0);
	NearArr[i].tex_max = pp::Vec2d(1.0, 1.0);
    
    //std::cout << "[snow.cpp] Near particle, index " << i << " created at " << NearArr[i].pt.x << " " << NearArr[i].pt.y << " " << NearArr[i].pt.z << "\n";
}

void init_snow( pp::Vec3d playerPos )
{
    //tried to call this function in course_load::course_load()
    //init_snow(players[0].pos);
    //init_snow(players[0].view.pos);
    //but both pos and view.pos werent initialized
    //now this function is called in racing::racing()
    if ( !get_texture_binding("c_snow_flake0", &(snow_tex[0]) ) ) {
        std::cerr << "Can't load snow texture : c_snow_flake0 ! Ay Ay Ay !\n";
    }
    if ( !get_texture_binding("c_snow_flake1", &(snow_tex[1]) ) ) {
        std::cerr << "Can't load snow texture : c_snow_flake1 ! Ay Ay Ay !\n";
    }
    if ( !get_texture_binding("c_snow_flake2", &(snow_tex[2]) ) ) {
        std::cerr << "Can't load snow texture : c_snow_flake2 ! Ay Ay Ay !\n";
    }
    if ( !get_texture_binding("c_snow_flake3", &(snow_tex[3]) ) ) {
        std::cerr << "Can't load snow texture : c_snow_flake3 ! Ay Ay Ay !\n";
    }
    
    UpdateArea(playerPos);
    NearArr = (TParticle*)malloc(sizeof(TParticle)*MAXNEAR);
    PartArr = (TParticle*)malloc(sizeof(TParticle)*MAXPART);
    int i;
    for(i = 0; i<MAXPART;i++) MakeSnowParticle(i);
    for(i = 0; i<MAXNEAR;i++) MakeNearParticle(i);
    
    lastPos = playerPos;
    
    /*
    UpdateArea(eyepoint);
    
    int i;
    for(i=0;i<MAXPART;i++) {
        MakeSnowParticle(i);
    }
    for(i=0;i<MAXNEAR;i++) {
        MakeNearParticle(i);
    }
    */
}

void reset_snow() {
    free(NearArr);
    free(PartArr);
}

void update_snow( double time_step, bool windy, pp::Vec3d playerPos )
{
    UpdateArea(playerPos);
    
    double xdiff = playerPos.x - lastPos.x;
    double ydiff = playerPos.y - lastPos.y;
	double zdiff = playerPos.z - lastPos.z;
    double xNearCoeff = XWindFactor * time_step;//We disable the xdrift for near particles because it is too obvious when the player turns
    double xcoeff = xNearCoeff + (xdiff * xdrift);
	double ycoeff = (ydiff * ydrift) + (ZWindFactor * 0.025);	
	double zcoeff = (ZWindFactor * time_step) + (zdiff * zdrift);
    
    int i;
    
    for(i = 0; i<MAXPART;i++) {
        //PartArr[i].pt.x += (eyepoint.x - PartArr[0].pt.x);  
    	//PartArr[i].pt.y += (eyepoint.y - PartArr[0].pt.y) + 1;
    	//PartArr[i].pt.z += (eyepoint.z - PartArr[0].pt.z);
        
        //PartArr[i].pt.x += (eyepoint.x - PartArr[0].pt.x);  
    	//PartArr[i].pt.y += (eyepoint.y - PartArr[0].pt.y) + 1;
    	//PartArr[i].pt.y += (eyepoint.y - PartArr[0].pt.y);
        //PartArr[i].pt.z += PartArr[i].vel.z;
        //PartArr[i].pt.z += (eyepoint.z - PartArr[0].pt.z) - 1;
        
        
        if (PartArr[i].pt.x < area.left) {
			PartArr[i].pt.x += XRANGE;
		} else if (PartArr[i].pt.x > area.right) {
			PartArr[i].pt.x -= XRANGE;
		} else if (PartArr[i].pt.y < area.bottom) {
			PartArr[i].pt.y += YRANGE;	
		} else if (PartArr[i].pt.y > area.top) {
			PartArr[i].pt.y -= YRANGE;			
		} else if (PartArr[i].pt.z < area.front) {
			PartArr[i].pt.z += ZRANGE;		
		} else if (PartArr[i].pt.z > area.back) {
			PartArr[i].pt.z -= ZRANGE;		
		}
        
        PartArr[i].pt.x += xcoeff;
		PartArr[i].pt.y += PartArr[i].vel.y * time_step + ycoeff;
		PartArr[i].pt.z += zcoeff;
    }
    for(i = 0; i<MAXNEAR;i++) {
        //NearArr[i].pt.x += (eyepoint.x - PartArr[0].pt.x);  
        //NearArr[i].pt.y += (eyepoint.y - PartArr[0].pt.y) + 1;
        //NearArr[i].pt.z += (eyepoint.z - PartArr[0].pt.z);
        //NearArr[i].pt.x += (eyepoint.x - PartArr[0].pt.x);
        //NearArr[i].pt.x += NearArr[i].vel.x;
        //NearArr[i].pt.y += (eyepoint.y - PartArr[0].pt.y) + 1;
        //NearArr[i].pt.y += (eyepoint.y - PartArr[0].pt.y);
        //NearArr[i].pt.z += (eyepoint.z - PartArr[0].pt.z) - 1;
        
        
        if (NearArr[i].pt.x < neararea.left) {
            NearArr[i].pt.x += NEAR_XRANGE;
		} else if (NearArr[i].pt.x > neararea.right) {
			NearArr[i].pt.x -= NEAR_XRANGE;
		} else if (NearArr[i].pt.y < neararea.bottom) {
			NearArr[i].pt.y += NEAR_YRANGE;	
		} else if (NearArr[i].pt.y > neararea.top) {
			NearArr[i].pt.y -= NEAR_YRANGE;
		} else if (NearArr[i].pt.z < neararea.front) {
			NearArr[i].pt.z += NEAR_ZRANGE;		
		} else if (NearArr[i].pt.z > neararea.back) {
			NearArr[i].pt.z -= NEAR_ZRANGE;		
		}
        
        NearArr[i].pt.x += xNearCoeff;//We disable the xdrift for near particles because it is too obvious when the player turns
		NearArr[i].pt.y += NearArr[i].vel.y * time_step + ycoeff;
		NearArr[i].pt.z += zcoeff;
    }
    
    lastPos = playerPos;
    
    
    /*
    UpdateArea(eyepoint);
    
    int i;
    for(i=0;i<MAXPART;i++) {
        MakeSnowParticle(i);
    }
    for(i=0;i<MAXNEAR;i++) {
        MakeNearParticle(i);
    }
    */
} 

void draw_snow( pp::Vec3d eyepoint )
{
    int i;
    double size;

    set_gl_options (PARTICLES);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glColor4f( particleColor[0], particleColor[1], particleColor[2],particleColor[3] );
    
    //color unspecified : shows snow
    //glColor4f( 1.0, 1.0, 1.0,1.0); shows snow
    //glColor4f( 0.0, 0.0, 0.0,0.0); doesnt show any snow
    
    //its the alpha vaolue in the color... if the alpha is null (total transparence), the textured color is also totally transparent => invisible
    
    //the snow only shows up near the start flag... why ?
    //because the y axis grows upwards. And in udateArea(), you had top = pos.y - YRANGE...and bottom = pos.y Also, in make*Particle(), you have part.y = -xrand(bottom,top)
    //  so it doesnt show too much on the strt line, for some reason...
    
    //a totally grey snowflake :-)
    //glColor4f(0.0,0.0,0.0,0.5);
    
    //glColor4f(0.0,1.0,0.0,0.5);
    
    //hardly visible with this color ! too... white :-)
    //glColor4f(1.0,1.0,1.0,0.5);
    
    
    
    for(i=0;i<MAXPART;i++) {
    
        /*
        if(i==0) {
            glColor4f(0.0,1.0,0.0,1.0);
        }*/
    
        size = PartArr[i].size;
        glBindTexture(GL_TEXTURE_2D, snow_tex[PartArr[i].index_tex]);
        draw_sprite( eyepoint, PartArr[i].pt, size, PartArr[i].tex_min, PartArr[i].tex_max );
        /*
        glPushMatrix();
        glTranslatef(PartArr[i].pt.x,PartArr[i].pt.y,PartArr[i].pt.z);
        glBegin( GL_QUADS );
        glTexCoord2f(PartArr[i].tex_min.x,PartArr[i].tex_min.y);
        glVertex3f(0,0,0);
        glTexCoord2f(PartArr[i].tex_min.x,PartArr[i].tex_max.y);
        glVertex3f(0,size,0);
        glTexCoord2f(PartArr[i].tex_max.x,PartArr[i].tex_max.y);
        glVertex3f(size,size,0);
        glTexCoord2f(PartArr[i].tex_max.x,PartArr[i].tex_min.y);
        glVertex3f(size,0,0);
        glEnd();
        glPopMatrix();
        */
        
        /*if(i==0) {
            glColor4f(0.0,0.0,0.0,0.5);
        }*/
    }
    
    for(i=0;i<MAXNEAR;i++) {
        size = NearArr[i].size;
        glBindTexture(GL_TEXTURE_2D, snow_tex[NearArr[i].index_tex]);
        /*
        glPushMatrix();
        glTranslatef(NearArr[i].pt.x,NearArr[i].pt.y,NearArr[i].pt.z);
        
        glBegin( GL_QUADS );
        glTexCoord2f(NearArr[i].tex_min.x,NearArr[i].tex_min.y);
        glVertex3f(0,0,0);
        glTexCoord2f(NearArr[i].tex_min.x,NearArr[i].tex_max.y);
        glVertex3f(0,size,0);
        glTexCoord2f(NearArr[i].tex_max.x,NearArr[i].tex_max.y);
        glVertex3f(size,size,0);
        glTexCoord2f(NearArr[i].tex_max.x,NearArr[i].tex_min.y);
        glVertex3f(size,0,0);
        glEnd();
        
        glPopMatrix();
        */
        draw_sprite( eyepoint, NearArr[i].pt, size, NearArr[i].tex_min, NearArr[i].tex_max );
    }
    
    

}

void draw_sprite( pp::Vec3d eyepoint, pp::Vec3d spriteLoc, double spriteSize, pp::Vec2d tex_min, pp::Vec2d tex_max ) {
    pp::Vec3d normal = eyepoint - spriteLoc;
    normal.normalize();
    if(normal.y == 1) return; // ||normal||==1 so if normal.y == 1 then normal == (0,1,0) and then we dont want to draw a "flat" sprite (contained in the (x,z) plane)
    glPushMatrix();
        glTranslatef( spriteLoc.x, spriteLoc.y, spriteLoc.z );
        glNormal3f(normal.x,normal.y,normal.z);//for lightning purposes... dont know if this is relevant here, so i put it just in case
        normal.y = 0; //the sprite has to be drawn using a normal vector contained in the (x,z) plane in order to appear as a billboard
        normal.normalize();
        glBegin( GL_QUADS );
	    {
            double spriteRadius = spriteSize/2; //we assume the sprite to occupy the space of a cube in 3d; so the radius is equal to size/2 here
            double spriteHeight = spriteSize;//we assume the sprite to occupy the space of a cube in 3d; so the height is equal to the size here
    		glTexCoord2f( tex_min.x, tex_min.y );
    		glVertex3f( -spriteRadius*normal.z, 0.0, spriteRadius*normal.x );
    		glTexCoord2f( tex_max.x, tex_min.y );
    		glVertex3f( spriteRadius*normal.z, 0.0, -spriteRadius*normal.x );
    		glTexCoord2f( tex_max.x, tex_max.y );
    		glVertex3f( spriteRadius*normal.z, spriteHeight, -spriteRadius*normal.x );
    		glTexCoord2f( tex_min.x, tex_max.y );
    		glVertex3f( -spriteRadius*normal.z, spriteHeight, spriteRadius*normal.x );
	    }
	    glEnd();
    glPopMatrix();
}


//Use for debugging purposes
// !!! Doesn't work with most of the snow textures. Works with snow.png as the texture for snow flakes (probably because it has no transparent pixels)
void draw_cuboid_areas() {
    glLineWidth(3.0);
    
    glPushMatrix();
    
    glColor4f(1.0,0.0,0.0,1.0);
    
    glBegin(GL_LINES);
    glVertex3f(neararea.left,neararea.bottom,neararea.front);
    glVertex3f(neararea.left,neararea.top,neararea.front);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(neararea.left,neararea.top,neararea.front);
    glVertex3f(neararea.right,neararea.top,neararea.front);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(neararea.right,neararea.top,neararea.front);
    glVertex3f(neararea.right,neararea.bottom,neararea.front);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(neararea.right,neararea.bottom,neararea.front);
    glVertex3f(neararea.left,neararea.bottom,neararea.front);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(neararea.left,neararea.bottom,neararea.back);
    glVertex3f(neararea.left,neararea.top,neararea.back);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(neararea.left,neararea.top,neararea.back);
    glVertex3f(neararea.right,neararea.top,neararea.back);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(neararea.right,neararea.top,neararea.back);
    glVertex3f(neararea.right,neararea.bottom,neararea.back);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(neararea.right,neararea.bottom,neararea.back);
    glVertex3f(neararea.left,neararea.bottom,neararea.back);
    glEnd();
    
    glPopMatrix();
    
    glPushMatrix();
    
    glColor4f(0.0,1.0,0.0,1.0);
    
    glBegin(GL_LINES);
    glVertex3f(area.left,area.bottom,area.front);
    glVertex3f(area.left,area.top,area.front);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(area.left,area.top,area.front);
    glVertex3f(area.right,area.top,area.front);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(area.right,area.top,area.front);
    glVertex3f(area.right,area.bottom,area.front);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(area.right,area.bottom,area.front);
    glVertex3f(area.left,area.bottom,area.front);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(area.left,area.bottom,area.back);
    glVertex3f(area.left,area.top,area.back);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(area.left,area.top,area.back);
    glVertex3f(area.right,area.top,area.back);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(area.right,area.top,area.back);
    glVertex3f(area.right,area.bottom,area.back);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(area.right,area.bottom,area.back);
    glVertex3f(area.left,area.bottom,area.back);
    glEnd();
    
    glPopMatrix();
}