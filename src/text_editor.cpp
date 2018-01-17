#include "text_editor.h"

TTF_Font *font;
const SDL_Color TEXT_COLOR={256,127,256};
std::map<char,Texture*> characters_images;

const char FONT_PATH[]={"fonts/basis33.ttf"};
const int FONT_SIZE=40;
const char CHARACTERS_FILE[]={"text editor/characters.tech"};
const int COLUMNS_MAX=100;
int CHARACTER_HEIGHT=0,CHARACTER_WEIGHT=0,DISTANCE_BETWEEN_CHARACTERS=0;
Texture *cursor_image=NULL,*background_image=NULL,*NULL_IMAGE=NULL;
Texture *selection_image=NULL;
const int CURSOR_FLASH_DURATION=200;
char TAB_STRING[]={"    "};

void Load_Text_Editor_fonts_and_characters()
{
 font=TTF_OpenFont(FONT_PATH,FONT_SIZE);
 FILE *in=fopen(CHARACTERS_FILE,"r");
 FILE *out=fopen("test/allchar.txt","w");
 while(!feof(in))
       {
        char ch;
        ch=fgetc(in);
        if(ch!='\n' && characters_images.count(ch)==0)
           {
            char text[2];
            text[0]=ch;
            text[1]=NULL;
            characters_images[ch]=Create_TTF_Texture(font,text,TEXT_COLOR);
            int w,h;
            w=characters_images[ch]->w;
            h=characters_images[ch]->h;
            fprintf(out,"%c w=%d h=%d\n",ch,w,h);
            CHARACTER_HEIGHT=std::max(CHARACTER_HEIGHT,characters_images[ch]->h);
           }
       }
 CHARACTER_WEIGHT=characters_images['a']->w;
 for(std::map<char,Texture*>::iterator i=characters_images.begin();i!=characters_images.end();i++)
     {
      if(i->second->w!=CHARACTER_WEIGHT || i->second->h!=CHARACTER_HEIGHT)
         {
          Texture *a=Create_Transparent_Texture(CHARACTER_WEIGHT,CHARACTER_HEIGHT);
          Apply_Stretched_Texture(0,0,CHARACTER_WEIGHT,CHARACTER_HEIGHT,i->second,a);
          Destroy_Texture(i->second);
          i->second=a;
          int w,h;
          w=i->second->w;
          h=i->second->h;
          char ch=i->first;
          fprintf(out,"%c w=%d h=%d\n",ch,w,h);
         }
     }
 fclose(out);
 NULL_IMAGE=new Texture;
 NULL_IMAGE->w=0;
 NULL_IMAGE->h=0;
 NULL_IMAGE->image=NULL;
 characters_images['\n']=NULL_IMAGE;
 Texture *aux=Load_Transparent_Texture("images/text editor/cursor.png");
 cursor_image=Create_Transparent_Texture(2,CHARACTER_HEIGHT);
 Apply_Stretched_Texture(0,0,2,CHARACTER_HEIGHT,aux,cursor_image);
 Destroy_Texture(aux);
 aux=Load_Texture("images/text editor/background.png");
 background_image=Create_Transparent_Texture(RESOLUTION_W,RESOLUTION_H);
 Apply_Stretched_Texture(0,0,RESOLUTION_W,RESOLUTION_H,aux,background_image);
 Destroy_Texture(aux);
 selection_image=Load_Transparent_Texture("images/text editor/selection.png");
 fclose(in);
}

void Clear_Text_Editor_fonts_and_characters()
{
 TTF_CloseFont(font);
 for(std::map<char,Texture*>::iterator i=characters_images.begin();i!=characters_images.end();i++)
     {
      Destroy_Texture(i->second);
     }
 characters_images.clear();
 Destroy_Texture(cursor_image);
 Destroy_Texture(background_image);
 Destroy_Texture(selection_image);
}

void Text_Editor::Clear()
{
 std::vector<std::string>().swap(lines);
 for(int i=0;i<lines_images.size();i++)
     Destroy_Texture(lines_images[i]);
 std::vector<Texture*>().swap(lines_images);
 cursor_flash_time.stop();
}

Texture *Text_Editor::Create_Monospaced_TTF_Texture(std::string *_text)
{
 Texture *aux=Create_Transparent_Texture((_text->size())*CHARACTER_WEIGHT,CHARACTER_HEIGHT);
 for(int i=0,x=0;i<_text->size();i++)
     {
      Apply_Texture(x,0,characters_images[(*_text)[i]],aux);
      x+=CHARACTER_WEIGHT;
     }
 return aux;
}

Texture *Text_Editor::Create_Monospaced_TTF_Texture(char *_text)
{
 std::string aux;
 aux=_text;
 return Create_Monospaced_TTF_Texture(&aux);
}

void Text_Editor::Load(char *_filename)
{
 strcpy(filename,_filename);
 std::ifstream in(filename);
 std::string buffer;
 Texture *aux;
 while(std::getline(in,buffer))
       {
        aux=Create_Monospaced_TTF_Texture(&buffer);
        lines_images.push_back(aux);
        aux=NULL;
        buffer+='\n';
        std::cout<<buffer;
        lines.push_back(buffer);
       }
 in.close();
 display_cursor_position_in_text.x=0;
 display_cursor_position_in_text.y=0;
 display_cursor_position_on_screen=display_cursor_position_in_text;
 cursor_position_in_text=cursor_position_on_screen=display_cursor_position_in_text;
 selection_start_in_text=selection_start_on_screen=selection_end_in_text=selection_end_on_screen=display_cursor_position_in_text;
 quit=false;
 cursor_flash_time.start();
 cursor_state=true;
 selection_started=false;
 selection_active=false;
}

void Text_Editor::Print_selection_line(int lin,int start,int end,int j,Texture *_screen)
{
 if((start==end && start!=-1) || end==0 || lines_images[lin]==NULL_IMAGE || lines[lin].size()<display_cursor_position_in_text.x)
    return;
 ///The whole line is selected
 if((start==-1 || start==0) && (end==-1 || end==lines[lin].size()-1 || end>=lines[lin].size()-1))
    {
     Apply_Stretched_Texture(area_on_screen.x,area_on_screen.y+j*CHARACTER_HEIGHT,
                             std::min(lines_images[lin]->w-display_cursor_position_on_screen.x,area_on_screen.w),std::min(lines_images[lin]->h,area_on_screen.h),
                             selection_image,_screen);
     return;
    }
 ///from beginning
 if(end==-1)
    {
     int w=std::min(lines_images[lin]->w,display_cursor_position_in_text.x*CHARACTER_WEIGHT+area_on_screen.w)-start*CHARACTER_WEIGHT;
     int x=area_on_screen.x+std::max(start-display_cursor_position_in_text.x,0)*CHARACTER_WEIGHT;
     Apply_Stretched_Texture(x,area_on_screen.y+j*CHARACTER_HEIGHT,
                             w,std::min(lines_images[lin]->h,area_on_screen.h),
                             selection_image,_screen);
     return;
    }
 if(start==-1)
    {
     int w=std::min((end-display_cursor_position_in_text.x)*CHARACTER_WEIGHT,area_on_screen.w);
     Apply_Stretched_Texture(area_on_screen.x,area_on_screen.y+j*CHARACTER_HEIGHT,
                             w,std::min(lines_images[lin]->h,area_on_screen.h),
                             selection_image,_screen);
     return;
    }
 if(start!=-1 && end!=-1)
    {
     int x=area_on_screen.x+std::max(0,(start-display_cursor_position_in_text.x)*CHARACTER_WEIGHT);
     int w=area_on_screen.w-x+area_on_screen.x-std::max(0,(display_cursor_position_in_text.x-end)*CHARACTER_WEIGHT+area_on_screen.w);
     Apply_Stretched_Texture(x,area_on_screen.y+j*CHARACTER_HEIGHT,
                             w,std::min(lines_images[lin]->h,area_on_screen.h),
                             selection_image,_screen);
    }
}

void Text_Editor::Print_selection(Texture *_screen)
{
 if(selection_start_in_text.x==selection_end_in_text.x && selection_start_in_text.y==selection_end_in_text.y)
    return;
 if(selection_start_in_text.y==selection_end_in_text.y)
    {
     if(display_cursor_position_in_text.x>=selection_end_in_text.x || display_cursor_position_in_text.x+area_on_screen.w/CHARACTER_WEIGHT<selection_start_in_text.x)
        return;
     Print_selection_line(selection_start_in_text.y,selection_start_in_text.x,selection_end_in_text.x,selection_start_in_text.y-display_cursor_position_in_text.y,_screen);
     return;
    }
 Print_selection_line(selection_start_in_text.y,selection_start_in_text.x,-1,selection_start_in_text.y-display_cursor_position_in_text.y,_screen);
 for(int i=selection_start_in_text.y+1,j=selection_start_in_text.y-display_cursor_position_in_text.y+1  ;i<selection_end_in_text.y;i++,j++)
     {
      Print_selection_line(i,-1,-1,j,_screen);
     }
 Print_selection_line(selection_end_in_text.y,-1,selection_end_in_text.x,selection_end_in_text.y-display_cursor_position_in_text.y,_screen);
}

void Text_Editor::Print(Texture *_screen)
{
 Apply_Texture(area_on_screen.x,area_on_screen.y,area_on_screen.w,area_on_screen.h,background_image,_screen);
 for(int i=display_cursor_position_in_text.y,j=0;i<lines.size() && (j+1)*CHARACTER_HEIGHT<=area_on_screen.h;i++,j++)
     {
      if(lines_images[i]!=NULL_IMAGE)
         Apply_Texture(display_cursor_position_on_screen.x,0,area_on_screen.x,j*CHARACTER_HEIGHT+area_on_screen.y,std::min(lines_images[i]->w-display_cursor_position_on_screen.x,area_on_screen.w),std::min(lines_images[i]->h,area_on_screen.h),lines_images[i],_screen);
     }
 if(selection_active)
    Print_selection(_screen);
 if(cursor_state)
    {
     Apply_Texture(cursor_position_on_screen.x+area_on_screen.x-display_cursor_position_on_screen.x,cursor_position_on_screen.y+area_on_screen.y-display_cursor_position_on_screen.y,cursor_image,_screen);
    }
 if(cursor_flash_time.get_ticks()>=CURSOR_FLASH_DURATION)
    {
     cursor_flash_time.start();
     cursor_state=!cursor_state;
    }
}

void Text_Editor::Write_Text(char *_text)
{
 int len=strlen(_text);
 Texture *aux=Create_Transparent_Texture(lines_images[cursor_position_in_text.y]->w+len*CHARACTER_WEIGHT,CHARACTER_HEIGHT);
 Apply_Texture(0,0,cursor_position_in_text.x*CHARACTER_WEIGHT,CHARACTER_HEIGHT,lines_images[cursor_position_in_text.y],aux);
 Texture *new_text=Create_Monospaced_TTF_Texture(_text);
 Apply_Texture(cursor_position_in_text.x*CHARACTER_WEIGHT,0,new_text,aux);
 Apply_Texture(cursor_position_in_text.x*CHARACTER_WEIGHT,0,cursor_position_in_text.x*CHARACTER_WEIGHT+new_text->w,0,(lines[cursor_position_in_text.y].size()-cursor_position_in_text.x-1)*CHARACTER_WEIGHT,CHARACTER_HEIGHT,lines_images[cursor_position_in_text.y],aux);
 Destroy_Texture(new_text);
 Destroy_Texture(lines_images[cursor_position_in_text.y]);
 lines_images[cursor_position_in_text.y]=aux;
 lines[cursor_position_in_text.y].insert(std::max(0,cursor_position_in_text.x),_text);
 cursor_position_in_text.x+=len;
 cursor_position_on_screen.x+=len*CHARACTER_WEIGHT;
}

void Text_Editor::Delete_Character()
{
 if(cursor_position_in_text.x==0)
    {
     if(cursor_position_in_text.y>0)
        {
         int size,w;
         size=lines[cursor_position_in_text.y-1].size();
         w=lines_images[cursor_position_in_text.y-1]->w;
         if(lines_images[cursor_position_in_text.y]!=NULL_IMAGE && lines_images[cursor_position_in_text.y-1]!=NULL_IMAGE)
            {
             Texture *aux;
             aux=Create_Transparent_Texture(lines_images[cursor_position_in_text.y]->w+lines_images[cursor_position_in_text.y-1]->w,CHARACTER_HEIGHT);
             Apply_Texture(0,0,lines_images[cursor_position_in_text.y-1],aux);
             Apply_Texture(lines_images[cursor_position_in_text.y-1]->w+1,0,lines_images[cursor_position_in_text.y],aux);
             Destroy_Texture(lines_images[cursor_position_in_text.y-1]);
             Destroy_Texture(lines_images[cursor_position_in_text.y]);
             lines_images[cursor_position_in_text.y-1]=aux;
            }
         else
            {
             if(lines_images[cursor_position_in_text.y]!=NULL_IMAGE)
                lines_images[cursor_position_in_text.y-1]=lines_images[cursor_position_in_text.y];
            }
         lines_images.erase(lines_images.begin()+cursor_position_in_text.y);
         lines[cursor_position_in_text.y-1].erase(lines[cursor_position_in_text.y-1].begin()+lines[cursor_position_in_text.y-1].size()-1);
         lines[cursor_position_in_text.y-1]+=lines[cursor_position_in_text.y];
         lines.erase(lines.begin()+cursor_position_in_text.y);
         cursor_position_in_text.x=size-1;
         cursor_position_on_screen.x=w;
         cursor_position_in_text.y--;
         cursor_position_on_screen.y-=CHARACTER_HEIGHT+DISTANCE_BETWEEN_CHARACTERS;
        }
    }
 else
    {
     lines[cursor_position_in_text.y].erase(lines[cursor_position_in_text.y].begin()+cursor_position_in_text.x-1);
     Texture *aux=NULL_IMAGE;
     int w=lines_images[cursor_position_in_text.y]->w,size=lines[cursor_position_in_text.y].size();
     if(lines[cursor_position_in_text.y].size()>1)
        {
         aux=Create_Transparent_Texture(lines_images[cursor_position_in_text.y]->w-CHARACTER_WEIGHT,CHARACTER_HEIGHT);
        }
     if(cursor_position_in_text.x!=1)
        {
         Apply_Texture(0,0,(cursor_position_in_text.x-1)*(CHARACTER_WEIGHT),CHARACTER_HEIGHT,lines_images[cursor_position_in_text.y],aux);
        }
     if(cursor_position_in_text.x<lines[cursor_position_in_text.y].size())
        {
         Apply_Texture((cursor_position_in_text.x)*CHARACTER_WEIGHT,0,(cursor_position_in_text.x-1)*CHARACTER_WEIGHT,0,(lines[cursor_position_in_text.y].size()-cursor_position_in_text.x)*CHARACTER_WEIGHT,CHARACTER_HEIGHT,lines_images[cursor_position_in_text.y],aux);
        }
     Destroy_Texture(lines_images[cursor_position_in_text.y]);
     lines_images[cursor_position_in_text.y]=aux;
     cursor_position_on_screen.x-=CHARACTER_WEIGHT+DISTANCE_BETWEEN_CHARACTERS;
     cursor_position_in_text.x--;
    }
}

void Text_Editor::Delete_Selection()
{
 cursor_position_in_text=selection_end_in_text;
 cursor_position_on_screen=selection_end_on_screen;
 int number_of_characters;
 if(selection_start_in_text.y==selection_end_in_text.y)
    number_of_characters=selection_end_in_text.x-selection_start_in_text.x;
 else
    {
     number_of_characters=selection_end_in_text.x+lines[selection_start_in_text.y].size()-selection_start_in_text.x;
     for(int i=selection_start_in_text.y+1;i<selection_end_in_text.y;i++)
         {
          number_of_characters+=lines[i].size();
         }
    }
 for(int i=0;i<number_of_characters;i++)
     {
      Delete_Character();
     }
 selection_active=false;
 selection_started=false;
}

bool Is_Bigger(SDL_Rect a,SDL_Rect b)
{
 if(a.y>b.y)
    {
     return true;
    }
 else
    {
     if(a.y==b.y)
        if(a.x>b.x)
           {
            return true;
           }
    }
 return false;
}

void Text_Editor::Update_selection(SDL_Rect _initial,SDL_Rect _final_in_text,SDL_Rect _final_on_screen)
{
 if(selection_started)
    {
     if(_initial.x==selection_start_in_text.x && _initial.y==selection_start_in_text.y)
        {
         selection_start_in_text=_final_in_text;
         selection_start_on_screen=_final_on_screen;
        }
     else
        {
         selection_end_in_text=_final_in_text;
         selection_end_on_screen=_final_on_screen;
        }
     if(Is_Bigger(selection_start_in_text,selection_end_in_text))
        {
         std::swap(selection_start_in_text,selection_end_in_text);
         std::swap(selection_start_on_screen,selection_end_on_screen);
        }
    }
 else
    {
     selection_active=false;
    }
}

void Text_Editor::Update_display_cursor()
{
 if(cursor_position_on_screen.x-display_cursor_position_on_screen.x>area_on_screen.w)
    {
     int dif=(cursor_position_on_screen.x-display_cursor_position_on_screen.x-area_on_screen.w)/CHARACTER_WEIGHT+1;
     display_cursor_position_in_text.x+=dif;
     display_cursor_position_on_screen.x+=dif*CHARACTER_WEIGHT;
    }
 if(cursor_position_on_screen.x<display_cursor_position_on_screen.x)
    {
     display_cursor_position_in_text.x=std::max(cursor_position_in_text.x-(area_on_screen.w/CHARACTER_WEIGHT)/2,0);
     display_cursor_position_on_screen.x=std::max(cursor_position_in_text.x*CHARACTER_WEIGHT-area_on_screen.w/2,0);
    }
 if(cursor_position_on_screen.y-display_cursor_position_on_screen.y>area_on_screen.h-CHARACTER_HEIGHT)
    {
     display_cursor_position_in_text.y++;
     display_cursor_position_on_screen.y+=CHARACTER_HEIGHT;
    }
 if(cursor_position_on_screen.y<display_cursor_position_on_screen.y)
    {
     display_cursor_position_in_text.y=std::max(cursor_position_in_text.y-area_on_screen.h/2,0);
     display_cursor_position_on_screen.y=std::max(cursor_position_in_text.y-area_on_screen.h/2,0)*CHARACTER_HEIGHT;
    }
}

void Text_Editor::Handle_Events(SDL_Event *event,Texture *_screen)
{
 if(event->key.type==SDL_KEYDOWN)
    {
     if(event->key.keysym.scancode==SDL_SCANCODE_ESCAPE)
        quit=true;
     if(event->key.keysym.scancode==SDL_SCANCODE_BACKSPACE)
        {
         if(selection_active)
            Delete_Selection();
         else
            Delete_Character();
        }
     bool cursor_moved=false;
     SDL_Rect last_cursor_position_in_text=cursor_position_in_text;
     if(event->key.keysym.scancode==SDL_SCANCODE_LEFT)
        {
         if(cursor_position_in_text.x==0)
            {
             if(cursor_position_in_text.y>0)
                {
                 cursor_position_in_text.y--;
                 cursor_position_on_screen.y-=CHARACTER_HEIGHT;
                 cursor_position_in_text.x=lines[cursor_position_in_text.y].size()-1;
                 cursor_position_on_screen.x=lines_images[cursor_position_in_text.y]->w;
                 cursor_moved=true;
                }
            }
         else
            {
             cursor_position_on_screen.x-=CHARACTER_WEIGHT+DISTANCE_BETWEEN_CHARACTERS;
             cursor_position_in_text.x--;
             cursor_moved=true;
            }
        }
     if(event->key.keysym.scancode==SDL_SCANCODE_RIGHT)
        {
         if(cursor_position_in_text.x==lines[cursor_position_in_text.y].size()-1)
            {
             if(cursor_position_in_text.y<lines.size()-1)
                {
                 cursor_position_in_text.y++;
                 cursor_position_on_screen.y+=CHARACTER_HEIGHT;
                 cursor_position_in_text.x=0;
                 cursor_position_on_screen.x=0;
                 cursor_moved=true;
                }
            }
         else
            {
             cursor_position_on_screen.x+=CHARACTER_WEIGHT+DISTANCE_BETWEEN_CHARACTERS;
             cursor_position_in_text.x++;
             cursor_moved=true;
            }
        }
     if(event->key.keysym.scancode==SDL_SCANCODE_UP)
        {
         if(cursor_position_in_text.y>0)
            {
             cursor_position_in_text.y--;
             cursor_position_on_screen.y-=CHARACTER_HEIGHT;
             if(cursor_position_in_text.x>lines[cursor_position_in_text.y].size()-1)
                {
                 cursor_position_in_text.x=lines[cursor_position_in_text.y].size()-1;
                 cursor_position_on_screen.x=lines_images[cursor_position_in_text.y]->w;
                }
             cursor_moved=true;
            }
        }
     if(event->key.keysym.scancode==SDL_SCANCODE_DOWN)
        {
         if(cursor_position_in_text.y<lines.size()-1)
            {
             cursor_position_in_text.y++;
             cursor_position_on_screen.y+=CHARACTER_HEIGHT;
             if(cursor_position_in_text.x>lines[cursor_position_in_text.y].size()-1)
                {
                 cursor_position_in_text.x=lines[cursor_position_in_text.y].size()-1;
                 cursor_position_on_screen.x=lines_images[cursor_position_in_text.y]->w;
                }
             cursor_moved=true;
            }
        }
     if(cursor_moved)
        Update_selection(last_cursor_position_in_text,cursor_position_in_text,cursor_position_on_screen);
     if(event->key.keysym.scancode==SDL_SCANCODE_TAB)
        {
         Write_Text(TAB_STRING);
        }
     if(event->key.keysym.scancode==SDL_SCANCODE_LSHIFT || event->key.keysym.scancode==SDL_SCANCODE_RSHIFT)
        {
         if(!selection_active)
            {
             selection_start_in_text=selection_end_in_text=cursor_position_in_text;
             selection_start_on_screen=selection_end_on_screen=cursor_position_on_screen;
            }
         selection_started=true;
         selection_active=true;
        }
     if(event->key.keysym.scancode==SDL_SCANCODE_RETURN || event->key.keysym.scancode==SDL_SCANCODE_KP_ENTER)
        {
         std::string new_line;
         new_line.append(lines[cursor_position_in_text.y].begin()+cursor_position_in_text.x,lines[cursor_position_in_text.y].end());
         Texture *aux;
         if(new_line.size()-1!=0)
            {
             aux=Create_Transparent_Texture((new_line.size()-1)*CHARACTER_WEIGHT,CHARACTER_HEIGHT);
             Apply_Texture(cursor_position_in_text.x*CHARACTER_WEIGHT,0,0,0,(new_line.size()-1)*CHARACTER_WEIGHT,CHARACTER_HEIGHT,lines_images[cursor_position_in_text.y],aux);
            }
         else
            {
             aux=NULL_IMAGE;
            }
         lines.insert(lines.begin()+cursor_position_in_text.y+1,new_line);
         lines_images.insert(lines_images.begin()+cursor_position_in_text.y+1,aux);
         if(lines[cursor_position_in_text.y].size()-new_line.size()!=0)
            {
             aux=Create_Transparent_Texture((lines[cursor_position_in_text.y].size()-new_line.size())*CHARACTER_WEIGHT,CHARACTER_HEIGHT);
             Apply_Texture(0,0,lines_images[cursor_position_in_text.y],aux);
            }
         else
            {
             aux=NULL_IMAGE;
            }
         Destroy_Texture(lines_images[cursor_position_in_text.y]);
         lines_images[cursor_position_in_text.y]=aux;
         lines[cursor_position_in_text.y].assign(lines[cursor_position_in_text.y],0,cursor_position_in_text.x);
         lines[cursor_position_in_text.y]+='\n';
         cursor_position_in_text.y++;
         cursor_position_in_text.x=0;
         cursor_position_on_screen.y+=CHARACTER_HEIGHT;
         cursor_position_on_screen.x=0;
        }
    }
 else
    {
     if(event->type==SDL_KEYUP)
        {
         if(event->key.keysym.scancode==SDL_SCANCODE_LSHIFT || event->key.keysym.scancode==SDL_SCANCODE_RSHIFT)
            {
             selection_started=false;
            }
        }
     if(event->type==SDL_TEXTINPUT)
        {
         Write_Text(event->text.text);
         selection_active=false;
         selection_started=false;
        }
    }
 Update_display_cursor();
}

void Text_Editor::Start(SDL_Rect *_area_on_screen,Texture *_screen)
{
 area_on_screen=*_area_on_screen;
 SDL_Event event;
 SDL_StartTextInput();
 while(!quit)
       {
        while(SDL_PollEvent(&event))
           Handle_Events(&event,_screen);
        Print(_screen);
        Flip_Buffers(_screen);
        SDL_Delay(100);
       }
 SDL_StopTextInput();
 Save_file();
}

void Text_Editor::Save_file(char *_filename)
{
 char path[TEXT_LENGTH_MAX]={NULL};
 strcat(path,"text editor/files/saved/");
 strcat(path,_filename==NULL?filename:_filename);
 FILE *out=fopen(path,"w");
 for(int i=0;i<lines.size();i++)
     {
      fprintf(out,"%s",lines[i].c_str());
     }
 fclose(out);
}
