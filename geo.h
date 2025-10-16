#pragma once
#include <raylib/raylib.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>

struct geo{
    Image img;
    Texture2D tex;
    Vector2 offset;
    int idx = 0;
    std::string name;
    std::string level;
};

static std::vector<geo> geos = {};

static std::vector<std::pair<int,geo*>> geos_clicked = {};

static int wind_w = 800;
static int wind_h = 800;
static int geo_clickedcount = 0;
static int geo_clickedcount_total = 0;

static float scale = 0;
static float mappos_x = 0;
static float mappos_y = 0;

static Vector2 mpos = {};

static Image highlighted = GenImageColor(1920,1080, BLANK);

static geo* current_geo = nullptr;

static Color colorid = BLACK;

static std::unordered_map<std::string,std::vector<int>> geos_unclicked = {
    {"Soustava",{}},
    {"Podsoustava",{}},
    {"Jednotka",{}}
};

static std::vector<std::pair<geo*,Color>> geos_highlighted = {};



static void DrawGeo(geo* geo,Color c){
    Rectangle destination = {
        mappos_x+(geo->offset.x*scale),
        mappos_y+(geo->offset.y*scale),
        scale*geo->tex.width,
        scale*geo->tex.height
    };
    DrawTexturePro(geo->tex, {0,0,(float)geo->tex.width,(float)geo->tex.height}, destination, {0,0}, 0, c);
    bool aabb = CheckCollisionRecs({mpos.x-1,mpos.y-1,3,3},destination);
    if(aabb && (geo->level == current_geo->level) && std::find(geos_unclicked[geo->level].begin(), geos_unclicked[geo->level].end(), geo->idx) != geos_unclicked[geo->level].end()){
        ImageDraw(&highlighted, geo->img, {0,0,(float)wind_w,(float)wind_h}, destination, colorid);
        geos_highlighted.push_back({geo,colorid});
        colorid.r+=1;
    }
}

static void DrawGeoNoCollision(geo* geo,Color c){
    Rectangle destination = {
        mappos_x+(geo->offset.x*scale),
        mappos_y+(geo->offset.y*scale),
        scale*geo->tex.width,
        scale*geo->tex.height
    };
    DrawTexturePro(geo->tex, {0,0,(float)geo->tex.width,(float)geo->tex.height}, destination, {0,0}, 0, c);
}

static std::vector<std::string> strsplit(std::string s, char delim){
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

static void GetNextGeo(){
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

static bool loaded = false;
static void LoadGeos(const std::string path){
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
        // geo* ptr = &geos[geos.size()-1];
        geos_unclicked[args[3]].push_back(geos.size()-1);
    } 
    GetNextGeo();


    geos_load.close();
    loaded = true;
}

static void UnloadGeos(){
    for(auto geo : geos){
        UnloadImage(geo.img);
        UnloadTexture(geo.tex);
    }
    geos.clear();
    geos_clicked.clear();
    
    geos_unclicked = {
        {"Soustava",{}},
        {"Podsoustava",{}},
        {"Jednotka",{}}
    };

    current_geo = nullptr;
}
