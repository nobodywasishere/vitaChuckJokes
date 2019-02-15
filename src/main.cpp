#define VITASDK

#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>


int main(int argc, char *argv[]) {

	SceCtrlData pad; //the control pad.
	vita2d_pgf *pgf; // the font, pgf, pvf, and font can be used.

	netInit(); // init net and then http
	httpInit();

	vita2d_init(); //init vita2d, must be done before other vita2d stuff.
	vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF)); // sets the color used when calling vita2d_clear_screen()

	//Checks if a directory exits and if not creates it.
	struct SceIoStat * dirStat = (SceIoStat*)malloc(sizeof(SceIoStat));
	if(sceIoGetstat("ux0:data/chuckJokes" , dirStat) < 0){
		sceIoMkdir("ux0:data/chuckJokes" , 0777);
	}

	//load texture, if !texture download from https://assets.chucknorris.host/img/avatar/chuck-norris.png and try again.
	vita2d_texture *icon = vita2d_load_PNG_file("ux0:data/chuckJokes/icon.png");
	if(!icon){
		curlDownloadFile("https://assets.chucknorris.host/img/avatar/chuck-norris.png",  "ux0:data/chuckJokes/icon.png");
		icon = vita2d_load_PNG_file("ux0:data/chuckJokes/icon.png");
	}

  // the default font to be used. can also load from file using other formats.
	pgf = vita2d_load_default_pgf();

	//set up the pad?
	memset(&pad, 0, sizeof(pad));
	bool next = false;

	//get a joke from chuck jokes website.
	std::string message_text = getJoke();

	//YES, infinit loop, whatever.
	while (1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);

		//if start, exit loop.
		if (pad.buttons & SCE_CTRL_START)
			break;

		//if cross get new joke, && !next stops things from repeating.
		if ((pad.buttons & SCE_CTRL_CROSS) && !next){
			next = true;
			message_text = "";
			message_text = getJoke();
		}

		//if not cross allow button press action
		else next = false;

		//start drawing and clear screen every frame.
		vita2d_start_drawing();
		vita2d_clear_screen();

		//things will crash if not loaded properly when drawn.
		if(icon){
			vita2d_draw_texture(icon, \
			960 / 2 - vita2d_texture_get_width(icon), \
			544 / 4 - vita2d_texture_get_height(icon));
		}

		//attempt at drawing string char by char within screen bounds.
		int xpos = 200;
		int ypos = 250;
		for(int i = 0; i < message_text.length();i++){
			vita2d_pgf_draw_textf(pgf,xpos,ypos,RGBA8(255,255,255,255),1.0f,"%c", message_text.at(i));
			xpos += vita2d_pgf_text_height(pgf, 1.0f,&message_text.at(i)) + 1;
			if(xpos > 760 && message_text[i] == ' '){
				xpos = 200;
				ypos += vita2d_pgf_text_height(pgf, 1.0f,&message_text.at(i)) + 5;
			}
		}//end attempt.

		//again, both of these at the end of every frame.
		vita2d_end_drawing();
		vita2d_swap_buffers();
	}

	//term both, not every frame, only once.
	httpTerm();
	netTerm();

	//finish vita2d stuff.
	vita2d_fini();
	vita2d_free_texture(icon);//this may not be correct, added last min. Still needed, just maybe corrected.
	vita2d_free_pgf(pgf);

	sceKernelExitProcess(0);
	return 0;
}
