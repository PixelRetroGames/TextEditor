#include "settings.h"

int DISPLAY_RESOLUTION_W,DISPLAY_RESOLUTION_H;

const int NUMBER_OF_AVAILABLE_RESOLUTIONS=4;
const int available_RESOLUTION_W[NUMBER_OF_AVAILABLE_RESOLUTIONS]={1366,1440,1600,1920};
const int available_RESOLUTION_H[NUMBER_OF_AVAILABLE_RESOLUTIONS]={ 768, 900, 900, 1080};
const int NUMBER_OF_AVAILABLE_VOLUMES=5,available_volumes[5]={0,25,50,75,100};
char *available_volumes_names[5]={"Mute","25%","50%","75%","100%"};

const SDL_Color settings_color={255,255,255};

int RESOLUTION_W=1366,RESOLUTION_H=768,RESOLUTION=2;
int DISPLAY_MODE=0;
int VOLUME,VOL=2;
bool POWER_SAVER=false,AUTO_ATTACK=false;

void Load_Settings()
{
 FILE *where=fopen("settings/settings.set","r");
 if(where==NULL)
    {
     //Save_Settings();
     return;
    }
 int dm=0;
 fscanf(where,"%d %d %d %d %d ",&RESOLUTION,&dm,&VOL,&POWER_SAVER,&AUTO_ATTACK);
 RESOLUTION_W=available_RESOLUTION_W[RESOLUTION];
 RESOLUTION_H=available_RESOLUTION_H[RESOLUTION];
 DISPLAY_MODE=dm==0?SDL_WINDOW_FULLSCREEN:0;
 //Get_Display_Resolution();
 //Validate_RESOLUTION();
 VOLUME=available_volumes[VOL];
 fclose(where);
}
