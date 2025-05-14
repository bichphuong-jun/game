#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
using namespace std;
const int SCREEN_WIDTH=800;
const int SCREEN_HEIGHT=600;
const int PLAYER_SIZE=50;
enum GameState {
    MENU,
    PLAYING,
    DYING,
    GAME_OVER,
    INSTRUCTIONS,
    VICTORY
};
struct Player {
    float x, y;
    float velY=0;
    bool onGround=false;
};
bool checkCollision (SDL_Rect a, SDL_Rect b) {
    return SDL_HasIntersection(&a, &b);
}
struct Platform {
    SDL_Rect rect;
    bool isDisappearing=false;
    bool visible=true;
    Uint32 disappearTime=0;
    bool isMoving=false;
    int dx=0;
    int minX=0, maxX=0;
};
struct Enemy {
    SDL_Rect rect;
    int dx;
    int minX, maxX;
};
struct Level {
    vector<Platform> platforms;
    vector<SDL_Rect> traps;
    SDL_Rect coin;
    bool coinCollected=false;
    vector <Enemy> enemies;
};
void loadLevels(vector<Level>& levels) {
    Level level1;
    Platform p11={{0,550,800,50},false};
    Platform p12={{200,450,150,20},false};
    level1.platforms={p11,p12};
    level1.coin={250,410,32,32};
    level1.traps={{350,530,30,20}};
    Level level2;
    Platform p21={{0,550,800,50}, false};
    Platform p22={{100,400,200,20}, false};
    Platform p23={{400,300,200,20}, true};
    level2. platforms={p21,p22,p23};
    level2.coin={450,260,32,32};
    level2.traps={{300,380,30,20},{600,530,30,20}};
    levels.push_back(level1);
    levels.push_back(level2);
    Level level3;
    Platform p31={{0,550,800,50}, false};
    Platform p32={{100, 400, 150, 20}, false};
    Platform movingPlat={{300,300,150,20}, false, true, 0, true, 2, 300, 500};
    level3.platforms={p31, p32, movingPlat};
    level3.coin={550, 260,32,32};
    level3.traps={{250,530,30,20},{600,530,30,20}};
    levels.push_back(level3);
    Level level4;
    Platform p41={{0, 550, 800, 50}, false};
    Platform p42={{100, 400, 200,20}, false};
    level4.platforms ={p41,p42};
    level4.coin={650,360,32,32};
    level4.traps={{300,530,30,20}};
    Enemy enemy1={{300, 510, 40, 40}, 2,300,500};
    level4.enemies.push_back(enemy1);
    levels.push_back(level4);
}
void resetPlayer(Player& player) {
    player.x=100;
    player.y=100;
    player.velY=0;
}
void resetGame(Player& player, vector<Level>& levels, int& currentLevel, int& lives, GameState& gameState) {
    resetPlayer(player);
    currentLevel=0;
    lives=3;
    for(auto& level:levels) level.coinCollected=false;
    gameState=PLAYING;
}
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const string& text, SDL_Color color) {
    SDL_Surface* surface=TTF_RenderText_Blended(font, text.c_str(), color);
    if(!surface) return nullptr;
    SDL_Texture* texture=SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}
void renderLevelText(SDL_Renderer* renderer, TTF_Font* font, int levelNumber) {
    SDL_Color white={255,255,255,255};
    string levelText="Level "+to_string(levelNumber+1);
    SDL_Texture* levelTex=renderText(renderer, font, levelText, white);
    if(levelTex) {
        int w, h;
        SDL_QueryTexture(levelTex, nullptr, nullptr, &w, &h);
        SDL_Rect levelRect={SCREEN_WIDTH-w-20,20,w,h};
        SDL_RenderCopy(renderer, levelTex, nullptr, &levelRect);
        SDL_DestroyTexture(levelTex);
    }
}
