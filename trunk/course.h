/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifndef COURSE_H
#define COURSE_H

#include "bh.h"
#include <vector>

#define FLOATVAL(i) (*(GLfloat*)(vnc_array+idx+(i)*sizeof(GLfloat)))
#define BYTEVAL(i) (*(GLubyte*)(vnc_array+idx+8*sizeof(GLfloat) +\
    i*sizeof(GLubyte)))
#define STRIDE_GL_ARRAY (8 * sizeof(GLfloat) + 4 * sizeof(GLubyte))
#define ELEV(x,y) (elevation[(x) + nx*(y)] )
#define NORM_INTERPOL 0.05
#define XCD(x) ((double)(x) / (nx-1.0) * width)
#define ZCD(y) (-(double)(y) / (ny-1.0) * length)
#define NMLPOINT(x,y) MakeVector (XCD(x), ELEV(x,y), ZCD(y) )


#define MAX_COURSES 64
#define MAX_TERR_TYPES 64
#define MAX_OBJECT_TYPES 128
#define MAX_DESCRIPTION_LINES 8

struct TCourse {
	string name;
	string dir;
	string author;
	string desc[MAX_DESCRIPTION_LINES];
	size_t num_lines;
	GLuint preview;
	double width;
	double length;
	double play_width;
	double play_length;
	double angle;
	double scale;
	double startx;
	double starty;
	int env;
	size_t music_theme;
 	bool use_keyframe;
	double finish_brake;
};

struct TTerrType {
	string texture;
	string sound;
	GLuint texid;
	TColor3 col;

	double friction;
	double depth;
	int vol_type;
	int particles;
	int trackmarks;
	int starttex;
	int tracktex;
	int stoptex;
	bool shiny;
};

struct TObjectType {
	string		name;
	string		texture;
	GLuint		texid;
	bool		collidable;
    double		above_ground;
    int			collectable;
    bool		drawable;
    bool		reset_point;
    bool		use_normal;
    TVector3	normal;
    int			num_items;
    int			poly;
};

class CCourse {
private:
	size_t		curr_course;
	string		CourseIndex;
	string		ObjectIndex;
	string		PolyIndex;
	string		CourseDir;
	bool		SaveItemsFlag;

	int			nx;
	int			ny;
	double		width;
	double		length;
	double		play_width;
	double		play_length;
	double		course_angle;
	double		course_descent;
	double		elev_scale;
	TVector2	start_pt;
	int			base_height_value;
	int			env;

	void		FreeTerrainTextures ();
	void		FreeObjectTextures ();
	void		CalcNormals ();
	void		MakeCourseNormals ();
	bool		LoadElevMap ();
	void		LoadItemList ();
	bool		LoadObjectMap ();
	bool		LoadTerrainMap ();
	int			GetTerrain (unsigned char pixel[]) const;

	void		MirrorCourseData ();
public:
	CCourse ();

	vector<TCourse>		CourseList;
	vector<TTerrType>	TerrList;
	vector<TObjectType>	ObjTypes;
	vector<TCollidable>	CollArr;
	vector<TItem>		NocollArr;
	vector<TPolyhedron>	PolyArr;

	char		*terrain;
	double		*elevation;
	TVector3	*nmls;
	GLubyte		*vnc_array;

	void ResetCourse ();
 	size_t GetCourseIdx (const string& dir) const;
	bool LoadCourseList ();
	void FreeCourseList ();
	bool LoadCourse (size_t idx);
	bool LoadTerrainTypes ();
	bool LoadObjectTypes ();
	void MakeStandardPolyhedrons ();
	void GetGLArrays (GLubyte **vnc_array) const;
	void FillGlArrays();

	void GetDimensions (double *w, double *l) const;
	void GetPlayDimensions (double *pw, double *pl) const;
	void GetDivisions (int *nx, int *ny) const;
	double GetCourseAngle () const;
	double GetCourseDescent () const;
	double GetBaseHeight (double distance) const;
	double GetMaxHeight (double distance) const;
	int GetEnv () const;
	const TVector2& GetStartPoint () const;
	void SetStartPoint (const TVector2& p);
	const TPolyhedron& GetPoly (int type) const;
	void MirrorCourse ();

	void GetIndicesForPoint (double x, double z, int *x0, int *y0, int *x1, int *y1) const;
	void FindBarycentricCoords (double x, double z,
		TIndex2 *idx0, TIndex2 *idx1, TIndex2 *idx2, double *u, double *v) const;
	double GetMinYCoord() const;
	TVector3 FindCourseNormal (double x, double z) const;
	double FindYCoord (double x, double z) const;
	void GetSurfaceType (double x, double z, double weights[]) const;
	size_t GetTerrainIdx (double x, double z, double level) const;
	TPlane GetLocalCoursePlane (TVector3 pt) const;
};

extern CCourse Course;

#endif
