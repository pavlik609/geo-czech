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
bool changed_layers = false;

void CheckCollisionGeos(){
    for(auto geo : geos_highlighted){
        bool colorequal = ColorIsEqual(GetImageColor(highlighted, mpos.x, mpos.y), geo.second);
        if(colorequal && !ui_hovering && !changed_layers){
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
                    changed_layers = true;
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

Camera2D cam = {};


void DrawMapMove(float x,float y, float width, float height){
    DrawRectangleRec({x,y,width,height},WHITE);
    DrawRectangleLinesEx({x,y,width,height},2,GRAY);

    float x_off = cam_x/(wind_w)*width;
    float y_off = cam_y/(wind_h)*height;

    float small_w = width*1/zoom;
    float small_h = height*1/zoom;

    DrawRectangleRec({x-x_off,y-y_off,width*1/zoom,height*1/zoom},BLUE);
    DrawRectangleLinesEx({x-x_off,y-y_off,width*1/zoom,height*1/zoom},2,SKYBLUE);

    DrawTexturePro(mapa,{0,0,(float)mapa.width,(float)mapa.height},{x,y,width,height},{0,0},0,{255,255,255,100});
    
    bool aabb = CheckCollisionRecs({mpos.x-1,mpos.y-1,3,3},{x,y,width,height});

    if(aabb && IsMouseButtonDown(0)){
        float dx = mpos.x-x-small_w/2.0f;
        float dy = mpos.y-y-small_h/2.0f;
        dx = fmax(0,dx);
        dx = fmin(width-small_w,dx);
        dy = fmax(0,dy);
        dy = fmin(height-small_h,dy);
        cam_x = -((dx)/width)*(wind_w);
        cam_y = -((dy)/height)*(wind_h);
    }
}

float topbar_height = 80;

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
    // BeginMode2D(cam);
    ClearBackground(BGCOL);
    int xdiff = wind_w - mapa.width;
    int ydiff = wind_h - mapa.height;
    float scalex = (float)(mapa.width+xdiff)/mapa.width; 
    float scaley = (float)(mapa.height+ydiff)/mapa.height;
    scale = fmin(scalex,scaley);
    mappos_x = (wind_w-scale*mapa.width)/2.0f;
    mappos_y = (wind_h-scale*mapa.height)/2.0f;
    DrawTexturePro(mapa, {0,0,(float)mapa.width,(float)mapa.height}, {(mappos_x+cam_x)*zoom,(cam_y+mappos_y)*zoom,scale*mapa.width*zoom,scale*mapa.height*zoom}, {0,0}, 0, WHITE);
    
    ui_hovering = false;
    ui_hovering = CheckCollisionRecs({mpos.x-1,mpos.y-1,3,3},{0,0,(float)wind_w,topbar_height});
    
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
        changed_layers = false;
        percentage = fmax(0, 100 * (float(geos_clicked.size()) / geo_clickedcount_total));
        std::string percent_str = "Skóre: "+ std::to_string(percentage).substr(0,5) + "%";
        // EndMode2D();
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
        // EndMode2D();
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
        DrawTextEx(fnt, "Změny:\n17.10.25 - v1.1.1\n- Přibližování\n16.10.25 - v1.1\n- Přidáno 63\n15.10.25 - v1.0\n- Iniciální verze", {boxx+boxw/2.0f+10,boxy+30}, 22, 0, GRAY);
    }
    DrawRectangleRec({0,0,(float)wind_w,topbar_height}, {200,190,198,255});
    DrawRectangleRec({0,0,(float)wind_w,topbar_height-2}, {215,204,213,255});
    bool zoomslider_result = GuiSlider({250,10,200,30}, "Přiblížení: 1", "5", &zoom, 1, 5);
    DrawMapMove(475,2,150, 70); 
    if(zoomslider_result){
        cam_x = -fmin(wind_w-(wind_w*1/zoom),-cam_x);
        cam_y = -fmin(wind_h-(wind_h*1/zoom),-cam_y);
    }
    // DrawFPS(0,0);
    // Texture tex = LoadTextureFromImage(highlighted);
    // DrawTexture(tex,0,0,WHITE);
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
