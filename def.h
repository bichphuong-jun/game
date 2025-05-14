#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
using namespace std;
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
bool checkCollision (SDL_Rect a, SDL_Rect b);
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
void loadLevels(vector<Level>& levels);
void resetPlayer(Player& player);
void resetGame(Player& player, vector<Level>& levels, int& currentLevel, int& lives, GameState& gameState);
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const string& text, SDL_Color color);
void renderLevelText(SDL_Renderer* renderer, TTF_Font* font, int levelNumber);
