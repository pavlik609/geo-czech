#include <cmath>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <raylib/raylib.h>
#include <string>
#define RAYGUI_IMPLEMENTATION
#include <raylib/raygui.h>
#include <unordered_map>
#include <vector>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "geo.h"

float percentage = -1;
bool showendwindow = false;

void CheckCollisionGeos(){
    for(auto geo : geos_highlighted){
        if(ColorIsEqual(GetImageColor(highlighted, mpos.x, mpos.y), geo.second)){
            DrawGeo(geo.first,YELLOW);
            if(IsMouseButtonPressed(0)){
                if(geo.first->name == current_geo->name){
                    geos_clicked.push_back({geo_clickedcount,geo.first});
                    geos_unclicked[geo.first->level].erase(std::find(geos_unclicked[geo.first->level].begin(), geos_unclicked[geo.first->level].end(), geo.first->idx));
                    geo_clickedcount = 0;
                    geo_clickedcount_total++;
                    if(geos_unclicked[geo.first->level].size() == 0){
                        int layers_completed = 0;
                        for(auto a : geos_unclicked){
                            if(a.second.size() == 0) { layers_completed++; continue; }
                        }
                        if(layers_completed == geos_unclicked.size()){
                            showendwindow = true;
                        }else{
                            GetNextGeo();
                        }
                    }else{
                        GetNextGeo();
                    }
                }else{
                    geo_clickedcount++;
                    geo_clickedcount_total++;
                }
            }
        }
    }
}

Font fnt;
Color BGCOL = {230,218,228,255};
Texture2D mapa;

int dropdown_active = 0;
bool dropdown_edit = false;

void Update(void){
    wind_w = GetScreenWidth();
    wind_h = GetScreenHeight();
    SetWindowSize(wind_w, wind_h);
    mpos = GetMousePosition();
    if(IsWindowResized()){
        ImageResize(&highlighted,wind_w,wind_h);
    }
    colorid = BLACK;
    if(geos_highlighted.size() > 0) { geos_highlighted.clear(); }

    ImageClearBackground(&highlighted, BLANK);
    BeginDrawing();
    ClearBackground(BGCOL);
    int xdiff = wind_w - mapa.width;
    int ydiff = wind_h - mapa.height;
    float scalex = (float)(mapa.width+xdiff)/mapa.width; 
    float scaley = (float)(mapa.height+ydiff)/mapa.height;
    scale = fmin(scalex,scaley);
    mappos_x = (wind_w-scale*mapa.width)/2.0f;
    mappos_y = (wind_h-scale*mapa.height)/2.0f;
    DrawTexturePro(mapa, {0,0,(float)mapa.width,(float)mapa.height}, {mappos_x,mappos_y,scale*mapa.width,scale*mapa.height}, {0,0}, 0, WHITE);
    
    if(loaded){
        for(auto level : geos_unclicked){
            Color c = {180,220,220,80};
            if(level.first != current_geo->level){
                c.r -= 70;
                c.g -= 70;
                c.b -= 70;
            }
            for(auto geo : level.second){
                DrawGeo(&geos[geo],c);
            }
        }
        for(auto geo : geos_clicked){
            float factor = fmin(4,geo.first)/4.0f;
            Color c = ColorLerp(GREEN, MAROON, factor);

            DrawGeoNoCollision(geo.second,c);
        }
        for(auto geo : geos_unclicked[current_geo->level]){
            Color c = {170,210,210,255};
            DrawGeo(&geos[geo],c);
        }
        if(geo_clickedcount >= 4){
            DrawGeoNoCollision(current_geo,PURPLE);
        }
        CheckCollisionGeos();
        percentage = fmax(0, 100 * (float(geos_clicked.size()) / geo_clickedcount_total));
        std::string percent_str = "Skóre: "+ std::to_string(percentage).substr(0,5) + "%";
       if(showendwindow){
            float boxw = 400;
            float boxh = 200;
            float boxx = (wind_w-boxw)/2.0f;
            float boxy = (wind_h-boxh)/2.0f;
            GuiPanel({boxx,boxy,boxw,boxh}, "Konec");
            DrawTextEx(fnt,percent_str.c_str(),{boxx+10,boxy+40},32,0,BLACK);
            DrawTextEx(fnt, "Typ:", {boxx+200,boxy+90}, 32, 0, BLACK);
            GuiDropdownBox({boxx+275,boxy+90,100,30}, u8"34\n63", &dropdown_active, true);
            if(GuiButton({boxx+5,boxy+boxh-35,175,30}, "Start")){
                UnloadGeos();
                switch(dropdown_active){
                    case 0:
                        LoadGeos("geos/34.txt");
                        break;
                    case 1:
                        LoadGeos("geos/63.txt");
                        break;
                    case 2: 
                        LoadGeos("geos/all.txt");
                        break;
                }
                showendwindow = false;
            }
        }
        DrawTextEx(fnt,("Další: "+ current_geo->name).c_str(),{5,(float)wind_h-(35+2)},32,0,BLACK);
        DrawTextEx(fnt,("Úroveń: "+ current_geo->level).c_str(),{5,(float)wind_h-(35+2)*2},32,0,BLACK);
        DrawTextEx(fnt,percent_str.c_str(),{5,(float)wind_h-(35+2)*3},32,0,BLACK);
    }else{
        float boxw = 400;
        float boxh = 200;
        float boxx = (wind_w-boxw)/2.0f;
        float boxy = (wind_h-boxh)/2.0f;
        GuiPanel({boxx,boxy,boxw,boxh}, "");
        DrawLineEx({boxx+boxw/2.0f,boxy+27},{boxx+boxw/2.0f,boxy+boxh-5},2,LIGHTGRAY);
        DrawTextEx(fnt, "Typ:", {boxx+5,boxy+30}, 32, 0, GRAY);
        GuiDropdownBox({boxx+80,boxy+30,100,30}, u8"34\n63", &dropdown_active, true);
        if(GuiButton({boxx+5,boxy+boxh-35,175,30}, "Start")){
            switch(dropdown_active){
                case 0:
                    LoadGeos("geos/34.txt");
                    break;
                case 1:
                    LoadGeos("geos/63.txt");
                    break;
                case 2: 
                    LoadGeos("geos/all.txt");
                    break;
            }
        }
        DrawTextEx(fnt, "Změny:\n16.10.25 - v1.1\n- Přidáno 63\n15.10.25 - v1.0\n- Iniciální verze", {boxx+boxw/2.0f+10,boxy+30}, 22, 0, GRAY);
    }
    // DrawFPS(0,0);
    EndDrawing();
}

int main (int argc, char *argv[]) { 
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(wind_w, wind_h, "Geomorfologické členění Česka");
    fnt = LoadFontEx("LiberationMono-Regular.ttf",32,NULL,999);
    GuiSetFont(fnt);
    GuiSetStyle(DEFAULT,TEXT_SIZE,32);
    SetRandomSeed(time(NULL));

    SetTargetFPS(30);

    // LoadGeos("geos/34.txt");

    mapa = LoadTexture("img/bg.png");  

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(Update, 0, 1);
#else
    while(!WindowShouldClose()){
        Update();
    }
#endif
    return 0;
}
