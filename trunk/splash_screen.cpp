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

#include "splash_screen.h"
#include "ogl.h"
#include "textures.h"
#include "audio.h"
#include "gui.h"
#include "course.h"
#include "tux.h"
#include "env.h"
#include "particles.h"
#include "credits.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "score.h"

void SplashKeys (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case 27: Winsys.Quit (); break;
		case 13: Winsys.SetMode (REGIST); break;
	}
}


void SplashInit (void) {  
	Winsys.ShowCursor (!param.ice_cursor);    
	init_ui_snow (); 
	Music.Play (param.menu_music, -1);
	g_game.loopdelay = 10;
}

void SplashLoop (double timestep ){
	Music.Update ();    
	check_gl_error();
    ClearRenderContext ();
    set_gl_options (GUI);
    SetupGuiDisplay ();


//	FT.SetFont ("normal");
	Tex.Draw (TEXLOGO, CENTER, 60, param.scale);
	FT.SetColor (colDYell);
	FT.AutoSizeN (6);
	int top = AutoYPosN (60);
	int dist = FT.AutoDistanceN (3);
	FT.DrawText (CENTER, top, "Loading resources,");
	FT.DrawText (CENTER, top+dist, "please wait ...");

    Winsys.SwapBuffers();
	Trans.LoadLanguages ();
	Trans.LoadTranslations (param.language);
	Course.MakeStandardPolyhedrons ();
	Sound.LoadSoundList ();
	LoadCreditList ();
	Char.LoadCharacterList ();
	Course.LoadObjectTypes (); 
	Course.LoadTerrainTypes ();
	Env.LoadEnvironmentList ();
	Course.LoadCourseList ();
	Score.LoadHighScore (); // after LoadCourseList !!!
	Events.LoadEventList ();
	Players.LoadAvatars (); // before LoadPlayers !!!
	Players.LoadPlayers ();

	Winsys.SetMode (REGIST);
} 

void SplashTerm () {
}

void splash_screen_register() {
	Winsys.SetModeFuncs (SPLASH, SplashInit, SplashLoop, SplashTerm,
	SplashKeys, NULL, NULL, NULL, NULL, NULL);
}