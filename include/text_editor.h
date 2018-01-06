#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include "texture.h"
#include "settings.h"
#include <string>
#include <vector>
#include <map>

#include <iostream>
#include <fstream>

class Text_Editor
{
 private:
 char filename[TEXT_LENGTH_MAX]={NULL};
 std::vector<std::string> lines;
 std::vector<Texture*> lines_images;
 SDL_Rect display_cursor_position_in_text,display_cursor_position_on_screen;
 SDL_Rect cursor_position_in_text,cursor_position_on_screen;
 SDL_Rect area_on_screen;
 bool quit=false;
 Timer cursor_flash_time;
 bool cursor_state,selection_started,selection_active;
 SDL_Rect selection_start_in_text,selection_start_on_screen;
 SDL_Rect selection_end_in_text,selection_end_on_screen;

 public:
 void Load(char *_filename);
 Texture *Create_Monospaced_TTF_Texture(std::string *_text);
 Texture *Create_Monospaced_TTF_Texture(char *_text);
 void Print_selection(Texture *_screen);
 void Print(Texture *_screen);
 void Write_Text(char *_text);
 void Delete_Character();
 void Delete_Selection();
 void Update_selection();
 void Update_display_cursor();
 void Handle_Events(SDL_Event *event,Texture *_screen);
 void Start(SDL_Rect *_area_on_screen,Texture *_screen);
};

void Load_Text_Editor_fonts_and_characters();
void Clear_Text_Editor_fonts_and_characters();

#endif //TEXT_EDITOR_H
