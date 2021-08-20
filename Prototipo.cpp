#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>

using namespace std;


const float FPS = 60.0;

SDL_Renderer* renderer;
SDL_Texture* sceneTexture;
SDL_Texture* arenTexture;
SDL_Texture* crabTexture;
SDL_Texture* coinTexture;
SDL_Texture* bulletTexture;
SDL_Texture* hudTexture;
SDL_Texture* minicoinTexture;
SDL_Texture* shotgunItemTexture;
SDL_Texture* fireItemTexture;

TTF_Font* vikingFont;
SDL_Surface* moneySurface;
SDL_Texture* moneyTexture;
SDL_Color color = { 255, 255, 255 };

bool isGameRunning = true;
bool shoot = false;
bool shotgun = false;
bool fire = false;

float gameTimer = 0;

int scene = 0;
int topBulletY;
int botBulletY;
int crabState = 2;
int balance = 0;

typedef struct InputState {
    bool right;
    bool left;
    bool fire;
};

InputState inputState;

typedef struct Coordinates {
    int x;
    int y;
};

// Acá se pueden modificar directamente las ubicaciones del hud, el personaje y el cangrejo
Coordinates hud = { 0, 0 };
Coordinates minicoin = { hud.x + 85, hud.y + 82 };
Coordinates money = { hud.x + 135, hud.y + 90 };
Coordinates aren = { 200, 320 };
Coordinates bullet = { 395, 481 };
Coordinates crab = { scene + 1500, 470 };


void initializeSDL() {

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Prototipo", 100, 100, 1280, 700, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_SetRenderDrawColor(renderer, 167, 247, 245, 97);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}


SDL_Texture* loadTexture(string filename, SDL_Renderer* renderer) {

    SDL_Texture* texture;

    printf("Loading %s", filename.c_str());

    texture = IMG_LoadTexture(renderer, filename.c_str());
    return texture;

}

SDL_Texture* loadLabel(string text, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer) {

    SDL_Surface* surface;
    SDL_Texture* texture;

    surface = TTF_RenderText_Blended(font, text.c_str(), color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    return texture;
}

void draw(SDL_Texture* texture, int x, int y, SDL_Renderer* renderer) {

    SDL_Rect dest;
    dest.x = x;
    dest.y = y;

    SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
    SDL_RenderCopy(renderer, texture, NULL, &dest);
}


void loadAssets() {
    sceneTexture = loadTexture("assets/scene.png", renderer);
    arenTexture = loadTexture("assets/aren.png", renderer);
    crabTexture = loadTexture("assets/crab.png", renderer);
    coinTexture = loadTexture("assets/coin.png", renderer);
    fireItemTexture = loadTexture("assets/burn.png", renderer);
    bulletTexture = loadTexture("assets/bullet.png", renderer);
    hudTexture = loadTexture("assets/hud.png", renderer);
    minicoinTexture = loadTexture("assets/minicoin.png", renderer);
    shotgunItemTexture = loadTexture("assets/shotgun.png", renderer);
    vikingFont = TTF_OpenFont("assets/viking.ttf", 30);
    moneyTexture = loadLabel(to_string(balance).c_str(), vikingFont, color, renderer);
}


void initializeGame() {

    initializeSDL();
    loadAssets();
}


void updateGame(float deltaTimeInSeconds) {

    const float PLAYER_SPEED = 600;

    const float BULLET_SPEED = 1500;


    if (inputState.fire && !shoot) {
        shoot = true;
        bullet.x = 395;
        topBulletY = 481;
        botBulletY = 481;
    }

    if (shoot && bullet.x <= 1280) {
        bullet.x = bullet.x + BULLET_SPEED * deltaTimeInSeconds;
        topBulletY--;
        botBulletY++;
    }
    else {
        shoot = false;
        bullet.x = scene;
    }

    if (crabState == 2)
        crab.x = scene + 1000 + cos(gameTimer * 2) * 100;
    else
        crab.x = scene + 1000;

    if (bullet.x > crab.x && bullet.x < crab.x + 250 && crabState == 2)
        crabState = 1;

    if (crab.x < 420 && crab.x > 70 && crabState == 1) {
        crabState = 0;
        balance += 60;
        moneySurface = TTF_RenderText_Blended(vikingFont, to_string(balance).c_str(), color);
        moneyTexture = SDL_CreateTextureFromSurface(renderer, moneySurface);
    }

    if (scene < -4250 && !shotgun && balance >= 50) {
        shotgun = true;
        balance -= 50;
        moneySurface = TTF_RenderText_Blended(vikingFont, to_string(balance).c_str(), color);
        moneyTexture = SDL_CreateTextureFromSurface(renderer, moneySurface);
    }

    if (scene < -8980 && !fire) {
        fire = true;
        bulletTexture = loadTexture("assets/fire.png", renderer);
    }


    if (inputState.left) {
        scene += PLAYER_SPEED * deltaTimeInSeconds;
    }
    else if (inputState.right) {
        scene -= PLAYER_SPEED * deltaTimeInSeconds;
    }


}


// Con estas dos funciones se pueden modificar las teclas de movimiento y disparo

void onKeyUp(int keyCode) {

    if (keyCode == SDLK_SPACE)
        inputState.fire = false;

    if (keyCode == SDLK_LEFT)
        inputState.left = false;
    else if (keyCode == SDLK_RIGHT)
        inputState.right = false;
}

void onKeyDown(int keyCode) {

    if (keyCode == SDLK_SPACE)
        inputState.fire = true;

    if (keyCode == SDLK_LEFT) {
        inputState.left = true;
        inputState.right = false;
    }
    else if (keyCode == SDLK_RIGHT) {
        inputState.left = false;
        inputState.right = true;
    }
}



void updateInput() {

    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        switch (event.type) {
        
            case SDL_QUIT:
                exit(0);
                break;

            case SDL_KEYUP:
                onKeyUp(event.key.keysym.sym);
                break;

            case SDL_KEYDOWN:
                onKeyDown(event.key.keysym.sym);
                break;

            default:
                break;
        }
    }
}


void render() {

    // Limpiar el renderer
    SDL_RenderClear(renderer);


    // Colocar texturas en el renderer

    draw(sceneTexture, scene, 0, renderer);
    draw(arenTexture, aren.x, aren.y, renderer);
    draw(hudTexture, hud.x, hud.y, renderer);
    draw(minicoinTexture, minicoin.x, minicoin.y, renderer);
    draw(moneyTexture, money.x, money.y, renderer);

    switch (crabState) {
        case 2:
            draw(crabTexture, crab.x, crab.y, renderer);
            break;
        case 1:
            draw(coinTexture, crab.x, crab.y, renderer);
            break;
    }

    if (shoot) {

        draw(bulletTexture, bullet.x, bullet.y, renderer);
        if (shotgun) {
            draw(bulletTexture, bullet.x, topBulletY, renderer);
            draw(bulletTexture, bullet.x, botBulletY, renderer);
        }
    }

    if (!shotgun)
        draw(shotgunItemTexture, scene + 4670, 355, renderer);

    if (!fire)
        draw(fireItemTexture, scene + 9400, 380, renderer);

    
    // Actualizar el renderer
    SDL_RenderPresent(renderer);
}



int main(int argc, char* args[]) {

    initializeGame();

    int currentTime = SDL_GetTicks();

    // Game Loop
    while (isGameRunning) {

        int previousTime = currentTime;
        currentTime = SDL_GetTicks();
        float deltaTime = currentTime - previousTime;
        float deltaTimeInSeconds = deltaTime / 1000.0;
        gameTimer += deltaTimeInSeconds;

        updateInput();
        updateGame(deltaTimeInSeconds);
        render();

        SDL_Delay(1000 / FPS);
    }

    return 0;
}