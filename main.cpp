#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "def.h"
using namespace std;
const int SCREEN_WIDTH=800;
const int SCREEN_HEIGHT=600;
const int PLAYER_SIZE=50;
int main (int argc, char* argv[]) {
    SDL_Init (SDL_INIT_VIDEO);
    IMG_Init (IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    SDL_Window* window=SDL_CreateWindow("GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer= SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //Load textures
    SDL_Texture* playerTex=IMG_LoadTexture(renderer, "player.png");
    SDL_Texture* coinTex=IMG_LoadTexture(renderer, "coin.png");
    SDL_Texture* backgroundTex=IMG_LoadTexture(renderer,"background.png");
    SDL_Texture* heartTex=IMG_LoadTexture(renderer, "heart.png");

    //Load font
    TTF_Font* font=TTF_OpenFont("arial.ttf",28);

    //Load sounds
    Mix_Chunk* coinSound=Mix_LoadWAV("coin.wav");
    Mix_Chunk* hitSound=Mix_LoadWAV("hit.wav");

    Player player={100,100};
    vector<Level> levels;
    int currentLevel=0;
    int lives=3;
    loadLevels(levels);
    bool running=true;
    SDL_Event e;
    float gravity=0.5f;
    float jumpForce=-10.0f;
    GameState gameState=MENU;
    Uint32 deathTime=0;
    bool flipped=false;

    //Hieu ung nhap nhay
    Uint8 alpha=255;
    bool fadingOut=true;
    Uint32 lastBlinkTime=SDL_GetTicks();

    //Hieu ung mo dan
    Uint8 fadeAlpha=0;
    bool fadingIn=true;
    Uint32 lastFadeTime=SDL_GetTicks();

    while(running) {
        while(SDL_PollEvent(&e)) {
            if(e.type==SDL_QUIT) running=false;
            if(e.type==SDL_KEYDOWN) {
                if(gameState==MENU) {
                    if(e.key.keysym.sym==SDLK_RETURN) gameState=PLAYING;
                    else if(e.key.keysym.sym==SDLK_ESCAPE) running=false;
                    else if(e.key.keysym.sym==SDLK_i) gameState=INSTRUCTIONS;
                }
                if(gameState==INSTRUCTIONS) {
                    if(e.key.keysym.sym==SDLK_ESCAPE||e.key.keysym.sym==SDLK_RETURN) gameState=MENU;
                }
                if(gameState==PLAYING&&e.key.keysym.sym==SDLK_SPACE&&player.onGround) {
                    player.velY=jumpForce;
                    player.onGround=false;
                }
                if(gameState==GAME_OVER||gameState==VICTORY) {
                    if(e.key.keysym.sym==SDLK_r) resetGame(player, levels, currentLevel, lives, gameState);
                    if(e.key.keysym.sym==SDLK_SPACE) running=false;
                }
            }
        }
        if(gameState==PLAYING) {
            const Uint8* keys=SDL_GetKeyboardState(nullptr);
            if(keys[SDL_SCANCODE_LEFT]) {
                player.x-=5;
                flipped=true;
            }
            if(keys[SDL_SCANCODE_RIGHT]) {
                player.x+=5;
                flipped=false;
            }
            player.velY+=gravity;
            player.y+=player.velY;
            SDL_Rect playerRect={(int)player.x,(int)player.y, PLAYER_SIZE, PLAYER_SIZE};
            Level& level=levels[currentLevel];
            player.onGround=false;
            for(auto& plat:level.platforms) {
                if(plat.visible&&checkCollision(playerRect, plat.rect)&&player.velY>=0) {
                    player.y=plat.rect.y-PLAYER_SIZE;
                    player.velY=0;
                    player.onGround=true;
                    if(plat.isMoving) {
                        plat.rect.x+=plat.dx;
                        if(plat.rect.x<plat.minX||plat.rect.x>plat.maxX) {
                            plat.dx=-plat.dx;
                            plat.rect.x+=plat.dx;
                        }
                    }
                    if(plat.isDisappearing) plat.disappearTime=SDL_GetTicks();
                    break;
                }
            }

            //Enemy
            for(auto& enemy:level.enemies) {
                enemy.rect.x+=enemy.dx;
                if(enemy.rect.x<enemy.minX||enemy.rect.x>enemy.maxX) {
                    enemy.dx=-enemy.dx;
                    enemy.rect.x+=enemy.dx;
                }
            }

            Uint32 now=SDL_GetTicks();
            for(auto& plat:level.platforms) {
                    if(plat.isDisappearing) {
                        if(plat.disappearTime==0&&checkCollision(playerRect, plat.rect)) plat.disappearTime=now;
                        if(plat.disappearTime>0) {
                            if(now-plat.disappearTime<2000) plat.visible=true;
                            else if(now-plat.disappearTime<4000) plat.visible=false;
                            else {
                                plat.visible=false;
                                plat.disappearTime=0;
                            }
                        }
                    }
                    if(!level.coinCollected&&checkCollision(playerRect, level.coin)) {
                            level.coinCollected=true;
                            Mix_PlayChannel(-1, coinSound, 0);
                    }
            }
            for(auto& trap:level.traps) {
                if(checkCollision(playerRect, trap)) {
                    lives--;
                    Mix_PlayChannel(-1, hitSound, 0);
                    if(lives<=0) {
                        gameState=DYING;
                        deathTime=SDL_GetTicks();
                    }
                    else resetPlayer(player);
                    break;
                }
            }
            for(auto& enemy: level.enemies) {
                if(checkCollision(playerRect, enemy.rect)) {
                    lives--;
                    Mix_PlayChannel(-1, hitSound, 0);
                    if(lives<=0) {
                        gameState=DYING;
                        deathTime=SDL_GetTicks();
                    }
                    else resetPlayer(player);
                    break;
                }
            }

            if(player.y>SCREEN_HEIGHT) {
                lives--;
                Mix_PlayChannel(-1,hitSound,0);
                if(lives<=0) {
                    gameState=DYING;
                    deathTime=SDL_GetTicks();
                }
                else resetPlayer(player);
            }
            if(level.coinCollected&&currentLevel+1<(int)levels.size()) {
                currentLevel++;
                resetPlayer(player);
            }
            else if(level.coinCollected&&currentLevel+1>=(int)levels.size()) gameState=VICTORY;
        }

        //render
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
        SDL_RenderClear(renderer);

        //Menu background
        if(gameState==MENU) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_Color white={255,255,255,255};

            //Game Start
            SDL_Texture* startTex=renderText(renderer, font, "Press Enter to Start", white);
            if(startTex) {
                int w, h;
                SDL_QueryTexture(startTex, nullptr, nullptr, &w, &h);
                SDL_Rect startRect={SCREEN_WIDTH/2-w/2, 200, w, h};
                SDL_RenderCopy(renderer, startTex, nullptr, &startRect);
                SDL_DestroyTexture(startTex);
            }

            //Instructions
            SDL_Texture* instructionsTex=renderText(renderer, font, "Press I for Instructions", white);
            if(instructionsTex) {
                int w, h;
                SDL_QueryTexture(instructionsTex, nullptr, nullptr, &w, &h);
                SDL_Rect instructionsRect={SCREEN_WIDTH/2-w/2, 250, w, h};
                SDL_RenderCopy(renderer, instructionsTex, nullptr, &instructionsRect);
                SDL_DestroyTexture(instructionsTex);
            }
        }
        if(gameState==INSTRUCTIONS) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_Color white={255, 255, 255, 255};
            vector<string> instructions={"Instructions:", "Arrow Left/Right: Move", "Space: Jump", "Avoid traps and collect coins!", "Press ESC to return to the menu"};
            for(int i=0;i<instructions.size(); i++) {
                SDL_Texture* lineTex=renderText(renderer, font, instructions[i], white);
                if(lineTex) {
                    int lw, lh;
                    SDL_QueryTexture(lineTex, nullptr, nullptr, &lw, &lh);
                    SDL_Rect lineRect={SCREEN_WIDTH/2-lw/2, 200+i*40, lw, lh};
                    SDL_RenderCopy(renderer, lineTex, nullptr, &lineRect);
                    SDL_DestroyTexture(lineTex);
                }
            }
        }

        if(gameState==PLAYING||gameState==DYING||gameState==GAME_OVER||gameState==VICTORY) {
            Level& level=levels[currentLevel];

            //Background
            SDL_Rect bgRect={0,0,SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(renderer, backgroundTex, nullptr, &bgRect);

            //Lives
            for(int i=0;i<lives;i++) {
                SDL_Rect heartRect={20+i*40, 20, 32, 32};
                SDL_RenderCopy(renderer, heartTex, nullptr, &heartRect);
            }

            //Platform
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            for(auto& plat:level.platforms) {
                if(plat.visible) {
                    SDL_RenderFillRect(renderer, &plat.rect);
                }
            }

            //Traps
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            for(auto& trap: level.traps) SDL_RenderFillRect(renderer, &trap);

            //Enemy
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            for(auto& enemy: level.enemies) SDL_RenderFillRect(renderer, &enemy.rect);

            //Coin
            if(!level.coinCollected) SDL_RenderCopy(renderer, coinTex, nullptr, &level.coin);

            //Player
            SDL_Rect playerRect={(int)player.x, (int)player.y, PLAYER_SIZE, PLAYER_SIZE};

            renderLevelText(renderer, font, currentLevel);

            if(gameState==DYING) {
                Uint32 now=SDL_GetTicks();
                if(now-lastBlinkTime>=100) {
                    alpha=fadingOut? alpha-50:alpha+50;
                    if(alpha<=50||alpha>=255) fadingOut=!fadingOut;
                    lastBlinkTime=now;
                }
                SDL_SetTextureAlphaMod(playerTex, alpha);
                SDL_RenderCopyEx(renderer, playerTex, nullptr, &playerRect, 0, nullptr, !flipped? SDL_FLIP_HORIZONTAL: SDL_FLIP_NONE);
                if(now-deathTime>2000) {
                    gameState=GAME_OVER;
                    fadeAlpha=0;
                    fadingIn=true;
                    lastFadeTime=now;
                }
            }
            else {
                SDL_SetTextureAlphaMod(playerTex, 255);
                SDL_RenderCopyEx(renderer, playerTex, nullptr, &playerRect, 0, nullptr, !flipped? SDL_FLIP_HORIZONTAL: SDL_FLIP_NONE);
            }

            //game over
            if(gameState==GAME_OVER) {
                Uint32 now=SDL_GetTicks();
                if(now-lastFadeTime>30&&fadeAlpha<200) {
                    fadeAlpha+=10;
                    lastFadeTime=now;
                }
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, fadeAlpha);
                SDL_Rect overlay={0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_RenderFillRect(renderer, &overlay);
                SDL_Color white={255,255,255,255};
                SDL_Texture*gameOverTex=renderText(renderer, font, "Game Over! Press R to Restart", white);
                if(gameOverTex) {
                    int w, h;
                    SDL_QueryTexture(gameOverTex, nullptr, nullptr, &w, &h);
                    SDL_Rect msgRect={SCREEN_WIDTH/2-w/2, SCREEN_HEIGHT/2-h/2, w,h};
                    SDL_RenderCopy(renderer, gameOverTex, nullptr, &msgRect);
                    SDL_DestroyTexture(gameOverTex);
                }
            }

            //victory
            if(gameState==VICTORY) {
                SDL_Color white={255,255,255,255};
                SDL_Texture* msgTex=renderText(renderer, font, "Victory! Press R to Restart", white);
                if(msgTex) {
                    int w,h;
                    SDL_QueryTexture(msgTex, nullptr, nullptr, &w,&h);
                    SDL_Rect dst={SCREEN_WIDTH/2-w/2, SCREEN_HEIGHT/2-h/2,w,h};
                    SDL_RenderCopy(renderer, msgTex, nullptr, &dst);
                    SDL_DestroyTexture(msgTex);
                }
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
}
Mix_FreeChunk(coinSound);
Mix_FreeChunk(hitSound);
Mix_CloseAudio();
SDL_DestroyTexture(playerTex);
SDL_DestroyTexture(heartTex);
SDL_DestroyTexture(coinTex);
SDL_DestroyTexture(backgroundTex);
TTF_CloseFont(font);
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
IMG_Quit();
SDL_Quit();
TTF_Quit();
return 0;
}
