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

float scale = 0;
float mappos_x = 0;
float mappos_y = 0;

int wind_w = 800;
int wind_h = 800;

Vector2 mpos = {};

Image highlighted = GenImageColor(wind_w, wind_h, BLANK);

Color colorid = BLACK;

struct geo{
    Image img;
    Texture2D tex;
    Vector2 offset;
    int idx = 0;
    std::string name;
    std::string level;
};

std::vector<std::pair<geo*,Color>> geos_highlighted = {};

std::vector<geo> geos = {};


std::unordered_map<std::string,std::vector<int>> geos_unclicked = {
    {"Soustava",{}},
    {"Podsoustava",{}},
    {"Jednotka",{}}
};

std::vector<std::pair<int,geo*>> geos_clicked = {};

geo* current_geo = nullptr;

std::vector<std::string> strsplit(std::string s, char delim){
    int latest = 0;
    int i = 1;
    std::vector<std::string> retval = {};
    for(char c : s){
        if(c==delim){
            retval.push_back(s.substr(latest,i-1));
            latest+= i;
            i=0;
        }
        i++;
    }
    retval.push_back(s.substr(latest,i));
    return retval;
}


void DrawGeo(geo* geo,Color c){
    Rectangle destination = {
        mappos_x+(geo->offset.x*scale),
        mappos_y+(geo->offset.y*scale),
        scale*geo->tex.width,
        scale*geo->tex.height
    };
    DrawTexturePro(geo->tex, {0,0,(float)geo->tex.width,(float)geo->tex.height}, destination, {0,0}, 0, c);
    if(CheckCollisionRecs({mpos.x-1,mpos.y-1,3,3},destination) && (geo->level == current_geo->level) && std::find(geos_unclicked[geo->level].begin(), geos_unclicked[geo->level].end(), geo->idx) != geos_unclicked[geo->level].end()){
        ImageDraw(&highlighted, geo->img, {0,0,(float)wind_w,(float)wind_h}, destination, colorid);
        geos_highlighted.push_back({geo,colorid});
        colorid.r+=1;
    }
}

void DrawGeoNoCollision(geo* geo,Color c){
    Rectangle destination = {
        mappos_x+(geo->offset.x*scale),
        mappos_y+(geo->offset.y*scale),
        scale*geo->tex.width,
        scale*geo->tex.height
    };
    DrawTexturePro(geo->tex, {0,0,(float)geo->tex.width,(float)geo->tex.height}, destination, {0,0}, 0, c);
}

int geo_clickedcount = 0;
int geo_clickedcount_total = 0;
float percentage = -1;
bool showendwindow = false;


void GetNextGeo(){
    // NOTE: this is a quick and dirty GOTO, could just be while loop and a vector that decreases every attempt
    // technically a TODO: fix
pre_roll:
    int nextgeolevel = GetRandomValue(0, 2);
    std::string tag = "";
    switch(nextgeolevel){
        case 0:
            tag = "Jednotka";
            break;
        case 1:
            tag = "Podsoustava";
            break;
        case 2:
            tag = "Soustava";
            break;
    }
    if(geos_unclicked[tag].size() == 0) { goto pre_roll; }
    int nextgeoidx = GetRandomValue(0, geos_unclicked[tag].size()-1);
    current_geo = &geos[geos_unclicked[tag][nextgeoidx]];
}

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
                        int nextgeoidx = GetRandomValue(0, geos_unclicked.size()-1);
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

bool loaded = false;
void LoadGeos(const std::string path){
    geo_clickedcount_total = 0;
    geo_clickedcount = 0;
    std::fstream geos_load(path);
    std::string line = "";
    while(getline(geos_load,line)){
        if(line.substr(0,2) == "--") { continue; }
        if(line == "") { continue; }
        std::vector<std::string> args = strsplit(line, '|');
        geos.push_back(geo{});
        geos.back().img = LoadImage(args[0].c_str());
        geos.back().tex = LoadTextureFromImage(geos.back().img);

        std::vector<std::string> offset = strsplit(args[2], '^');
        geos.back().offset = {std::stof(offset[0]),std::stof(offset[1])};
        geos.back().name = args[1];
        geos.back().level = args[3];
        geos.back().idx = geos.size()-1;
        geo* ptr = &geos[geos.size()-1];
        geos_unclicked[args[3]].push_back(geos.size()-1);
    } 
    GetNextGeo();


    geos_load.close();
    loaded = true;
}

int main (int argc, char *argv[]) { 
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(wind_w, wind_h, "Geomorfologické členění Česka");
    Font fnt = LoadFontEx("LiberationMono-Regular.ttf",32,NULL,999);
    SetRandomSeed(time(NULL));

    LoadGeos("geos/34.txt");

    Color BGCOL = {230,218,228,255};

    Texture2D mapa = LoadTexture("img/bg.png");  


    while(!WindowShouldClose()){
        wind_w = GetScreenWidth();
        wind_h = GetScreenHeight();
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
        float scalex = wind_w < mapa.width ? (float)(mapa.width+xdiff)/mapa.width : 1; 
        float scaley = wind_h < mapa.height ? (float)(mapa.height+ydiff)/mapa.height : 1;
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
                GuiWindowBox({boxx,boxy,boxw,boxh}, "Konec");
                DrawTextEx(fnt,percent_str.c_str(),{boxx+20,boxy+50},32,0,BLACK);
            }
            DrawTextEx(fnt,("Další: "+ current_geo->name).c_str(),{5,(float)wind_h-(35+2)},32,0,BLACK);
            DrawTextEx(fnt,("Úroveń: "+ current_geo->level).c_str(),{5,(float)wind_h-(35+2)*2},32,0,BLACK);
            DrawTextEx(fnt,percent_str.c_str(),{5,(float)wind_h-(35+2)*3},32,0,BLACK);
        }

        EndDrawing();
    }
    return 0;
}
