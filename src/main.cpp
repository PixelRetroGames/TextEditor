#include "settings.h"
#include "text_editor.h"

#include <cstdio>
#include <ctime>
#include <cstdlib>

Texture *screen;

int main( int argc, char *args[] )
{
 ///Random
 srand((unsigned int)(time(NULL)));
 ///Random
 if(SDL_Init(SDL_INIT_EVERYTHING)<0)
    {
     FILE *log_file=fopen("err/logs.txt","w");
     fprintf(log_file,"SDL_Init() failed : %s ",SDL_GetError());
     fclose(log_file);
     char message[TEXT_LENGTH_MAX];
     strcpy(message,"SDL_Init() failed : ");
     strcat(message,SDL_GetError());
     SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"SDL failure",message,NULL);
     exit(-1);
    }
 if(TTF_Init()==-1)
    {
     FILE *log_file=fopen("err/logs.txt","w");
     fprintf(log_file,"TTF_Init() failed : %s ",TTF_GetError());
     fclose(log_file);
     char message[TEXT_LENGTH_MAX];
     strcpy(message,"TTF_Init() failed : ");
     strcat(message,SDL_GetError());
     SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"SDL_TTF module failure",message,NULL);
     exit(-2);
    }
 Load_Settings();
 Open_Window_and_Renderer(RESOLUTION_W,RESOLUTION_H,DISPLAY_MODE);
 last_frame=Create_Transparent_Texture(RESOLUTION_W,RESOLUTION_H);
 screen=new Texture;
 screen->w=RESOLUTION_W;
 screen->h=RESOLUTION_H;
 screen->image=NULL;
 SCREEN=screen;
 if(screen==NULL)
    {
     FILE *log_file=fopen("err/logs.txt","w");
     fprintf(log_file,"Open_Window_and_Renderer failed : %s ",SDL_GetError());
     fclose(log_file);
     char message[TEXT_LENGTH_MAX];
     strcpy(message,"Open_Window_and_Renderer failed : ");
     strcat(message,SDL_GetError());
     SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"SDL video module failure",message,NULL);
     exit(-1);
    }
 Flip_Buffers(screen);
 //static_screen=screen;

 Text_Editor test;
 Load_Text_Editor_fonts_and_characters();
 test.Load("test1.txt");
 SDL_Rect display_area={5,5,RESOLUTION_W/2,RESOLUTION_H/2};
 test.Start(&display_area,screen);
 return 0;
}
