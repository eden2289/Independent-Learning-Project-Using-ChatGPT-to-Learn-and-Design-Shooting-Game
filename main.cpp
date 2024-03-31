#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <time.h>
#include <string>

// 定义加载纹理函数
SDL_Texture* LoadTexture(const std::string &filePath, SDL_Renderer* renderer) {
    SDL_Surface* tempSurface = IMG_Load(filePath.c_str());
    if (!tempSurface) {
        SDL_Log("Unable to load image %s! SDL_image Error: %s\n", filePath.c_str(), IMG_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    if (!texture) {
        SDL_Log("Unable to create texture from %s! SDL Error: %s\n", filePath.c_str(), SDL_GetError());
    }

    return texture;
}

int main(int argc, char* argv[]) {
    // 初始化 SDL2、SDL2_image 和 SDL_ttf
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    
    // 載入字體並設定字體大小
    TTF_Font* font = TTF_OpenFont("/Users/linyili/Desktop/game/FZSJ-BHTJW.TTF", 40);

    // 定義計時器變數
    Uint32 startTime = 0; // 遊戲開始時間
    Uint32 currentTime = 0; // 當前時間

    // 設定文本顏色為黑色
    SDL_Color textColor = {255, 255, 255, 255};

    // 定義窗口大小
    int windowWidth = 600;
    int windowHeight = 800;

    // 創建窗口和渲染器
    SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    // 加载紋理
    SDL_Texture* backgroundTexture = LoadTexture("/Users/linyili/Desktop/game/background.png", renderer);
    SDL_Texture* heartTexture = LoadTexture("/Users/linyili/Desktop/game/love.png", renderer);
    SDL_Texture* playerTexture = LoadTexture("/Users/linyili/Desktop/game/main.png", renderer);
    SDL_Texture* enemyTexture = LoadTexture("/Users/linyili/Desktop/game/blue.png", renderer);
    SDL_Texture* specialEnemyTexture = LoadTexture("/Users/linyili/Desktop/game/green.png", renderer);
    SDL_Texture* bulletTexture = LoadTexture("/Users/linyili/Desktop/game/magic.png", renderer);
    SDL_Texture* mainMenuBackground = LoadTexture("/Users/linyili/Desktop/game/mainBackground.png", renderer);

    // 定義玩家矩形初始位置和速度
    int playerX = 270;
    int playerY = 640;
    int playerWidth = 100;
    int playerHeight = 100;
    int playerSpeed = 5;

    // 定義玩家初始生命值和得分
    int playerLives = 5;
    int score = 0;

    // 創建玩家矩形
    SDL_Rect playerRect = { playerX, playerY, playerWidth, playerHeight };

    // 定義敵人數量和屬性
    const int NUM_ENEMIES = 6;
    SDL_Rect enemyRects[NUM_ENEMIES];
    int enemyWidth = 50;
    int enemyHeight = 50;
    int enemySpeedX[NUM_ENEMIES];
    int enemySpeedY[NUM_ENEMIES];

    // 定義特殊敵人的標記數組和回血道具的屬性
    bool isSpecialEnemy[NUM_ENEMIES] = {false}; // 標記特殊敵人的數組
    SDL_Rect healthPowerUps[NUM_ENEMIES]; // 回血道具的位置和大小
    bool healthPowerUpActive[NUM_ENEMIES] = {false}; // 標記回血道具是否處於激活狀態的數組
    int healthPowerUpWidth = 20;
    int healthPowerUpHeight = 20;
    const int SPECIAL_ENEMY_CHANCE = 8; // 特殊敵人出現的機率，此處設為1代表每個敵人都是特殊敵人，可根據需要調整

    // 初始化敵人和回血道具
    for (int i = 0; i < NUM_ENEMIES; i++) {
        enemyRects[i].x = rand() % (windowWidth - enemyWidth);
        enemyRects[i].y = rand() % (windowHeight / 2 - enemyHeight);
        enemySpeedX[i] = rand() % 3 + rand() % 2;
        enemySpeedY[i] = rand() % 3 + rand() % 2;
        enemyRects[i] = (SDL_Rect){enemyRects[i].x, enemyRects[i].y, enemyWidth, enemyHeight};

        // 隨機決定哪些敵人是特殊的
        if (rand() % SPECIAL_ENEMY_CHANCE == 0) {
            isSpecialEnemy[i] = true;
        }
        // 初始化回血道具為非激活狀態
        healthPowerUps[i] = (SDL_Rect){-1, -1, healthPowerUpWidth, healthPowerUpHeight};
    }

    // 定義子彈數量、發射間隔和屬性
    const int NUM_BULLETS = 20;
    const Uint32 bulletFireInterval = 100; // 子彈發射間隔
    Uint32 lastBulletFireTime = 0; // 上一次發射子彈的時間
    SDL_Rect bulletRects[NUM_BULLETS];
    int bulletWidth = 26;
    int bulletHeight = 40;
    int bulletSpeedY = -10;

    // 初始化子彈矩形數組
    for (int i = 0; i < NUM_BULLETS; i++) {
        bulletRects[i].x= -1;
        bulletRects[i].y= -1;
        bulletRects[i] = (SDL_Rect){ bulletRects[i].x, bulletRects[i].y, bulletWidth, bulletHeight };
    }

    // 定義敵人矩形重生時間數組
    Uint32 enemyRespawnTime[NUM_ENEMIES];

    // 初始化敵人矩形重生時間數組
    for (int i = 0; i < NUM_ENEMIES; i++) {
        enemyRespawnTime[i] = 0;
    }

    // 等待窗口關閉事件
    bool quit = false;
    SDL_Event event;

    // 主菜单循环
    bool showMainMenu = true;
    while (showMainMenu) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                showMainMenu = false;
                return 0;  // 退出游戏
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                showMainMenu = false;  // 开始游戏
            }
        }
        // 渲染主菜单背景
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, mainMenuBackground, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    

    // 清除屏幕
    SDL_RenderClear(renderer);
    

    // 初始化計時器
    startTime = SDL_GetTicks(); // 獲取遊戲開始的時間


    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_a:
                        playerX -= playerSpeed;
                        break;
                    case SDLK_d:
                        playerX += playerSpeed;
                        break;
                    case SDLK_w:
                        playerY -= playerSpeed;
                        break;
                    case SDLK_s:
                        playerY += playerSpeed;
                        break;
                    case SDLK_SPACE:
                        // 在玩家矩形的頂部中央位置創建一個新的子彈矩形
                        Uint32 currentTime = SDL_GetTicks();
                        if (currentTime - lastBulletFireTime >= bulletFireInterval) {
                            // 發射子彈的邏輯
                            for (int i = 0; i < NUM_BULLETS; i++) {
                                if (bulletRects[i].y < 0) { // 找到一個未激活的子彈
                                    bulletRects[i].x = playerX + playerWidth / 2 - bulletWidth / 2;
                                    bulletRects[i].y = playerY;
                                    lastBulletFireTime = currentTime; // 更新上次發射子彈的時間
                                    break;
                                }
                            }
                        }
                        break;
                }
            }
        }
        // 更新经过的时间（秒）
        currentTime = (SDL_GetTicks() - startTime) / 1000;

        // 移動敵人矩形
        for (int i = 0; i < NUM_ENEMIES; i++) {
            enemyRects[i].x += enemySpeedX[i];
            enemyRects[i].y += enemySpeedY[i];

            // 如果敵人矩形碰到了窗口的邊緣，改變它的移動方向
            if (enemyRects[i].x <= 0 || enemyRects[i].x >= windowWidth - enemyWidth) {
                enemySpeedX[i] = -enemySpeedX[i];
            }
            if (enemyRects[i].y <= 0 || enemyRects[i].y >= windowHeight  - enemyHeight) {
                enemySpeedY[i] = -enemySpeedY[i];
            }
        }

        // 移動子彈矩形
        for (int i = 0; i < NUM_BULLETS; i++) {
            if (bulletRects[i].y >= 0) {
                bulletRects[i].y += bulletSpeedY;
            }
        }

        // 检测子弹矩形和敌人矩形的碰撞
        for (int i = 0; i < NUM_BULLETS; i++) {
            if (bulletRects[i].y >= 0) {
                for (int j = 0; j < NUM_ENEMIES; j++) {
                    if (SDL_HasIntersection(&bulletRects[i], &enemyRects[j])) {
                        bulletRects[i].y = -1;  // 子弹消失
                        score += 10;  // 增加得分

                        if (isSpecialEnemy[j]) {  // 如果击中的是特殊敌人
                            // 在特殊敌人的位置生成回血道具，并激活它
                            healthPowerUps[j] = (SDL_Rect){enemyRects[j].x, enemyRects[j].y, healthPowerUpWidth, healthPowerUpHeight};
                            healthPowerUpActive[j] = true;
                            isSpecialEnemy[j] = false;  // 将敌人标记为非特殊敌人，防止重复生成回血道具
                        }

                        // 重置敌人位置并随机决定是否变为特殊敌人
                        enemyRects[j].x = rand() % (windowWidth - enemyWidth);
                        enemyRects[j].y = rand() % (windowHeight / 2 - enemyHeight);
                        isSpecialEnemy[j] = (rand() % SPECIAL_ENEMY_CHANCE == 0);  // 有一定概率变成特殊敌人

                        // 如果这个敌人变成了特殊敌人，则初始化回血道具
                        if (isSpecialEnemy[j]) {
                            healthPowerUps[j] = (SDL_Rect){enemyRects[j].x, enemyRects[j].y, healthPowerUpWidth, healthPowerUpHeight};
                            healthPowerUpActive[j] = false;  // 初始时不激活回血道具
                        }
                    }
                }
            }
        }
        



        // 檢測敵人矩形和玩家矩形的碰撞
        for (int i = 0; i < NUM_ENEMIES; i++) {
            if (SDL_HasIntersection(&playerRect, &enemyRects[i])) {
                playerLives--; // 玩家生命值減少
                enemyRects[i].x = rand() % (windowWidth - enemyWidth); // 敵人在隨機位置重生
                enemyRects[i].y = rand() % (windowHeight / 2 - enemyHeight);
                enemyRespawnTime[i] = SDL_GetTicks() + 2000; // 設置重生時間
                if (playerLives == 0) {
                    quit = true; // 生命值為0，遊戲結束
                }
            }
        }

        // 检测玩家与回血道具的碰撞
        for (int i = 0; i < NUM_ENEMIES; i++) {
            if (healthPowerUpActive[i] && SDL_HasIntersection(&playerRect, &healthPowerUps[i])) {
                playerLives += 1; // 玩家回复生命值
                healthPowerUpActive[i] = false; // 回血道具被拾取，设置为非激活状态
            }
        }


        // 清除渲染器上的所有內容
        SDL_RenderClear(renderer);

        // 使用背景纹理填充屏幕
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        

        // 繪製敵人和回血道具
        for (int i = 0; i < NUM_ENEMIES; i++) {
            // 根據敵人是否為特殊敵人來設置不同的顏色
            SDL_Texture* currentTexture = isSpecialEnemy[i] ? specialEnemyTexture : enemyTexture;
            SDL_RenderCopy(renderer, currentTexture, NULL, &enemyRects[i]);


            // 渲染激活的回血道具
            for (int i = 0; i < NUM_ENEMIES; i++) {
                if (healthPowerUpActive[i]) {
                    SDL_RenderCopy(renderer, heartTexture, NULL, &healthPowerUps[i]);
                }
            }

        } 

        

        // 更新玩家位置並繪製
        const Uint8* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_A]) playerX -= playerSpeed;
        if (state[SDL_SCANCODE_D]) playerX += playerSpeed;
        if (state[SDL_SCANCODE_W]) playerY -= playerSpeed;
        if (state[SDL_SCANCODE_S]) playerY += playerSpeed;

        // 檢查玩家矩形是否超出窗口邊界
        playerRect.x = playerX = (playerX < 0) ? 0 : ((playerX > windowWidth - playerWidth) ? windowWidth - playerWidth : playerX);
        playerRect.y = playerY = (playerY < 0) ? 0 : ((playerY > windowHeight - playerHeight) ? windowHeight - playerHeight : playerY);
        
        //繪製玩家
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);

        /// 繪製敵人
        for (int i = 0; i < NUM_ENEMIES; i++) {
            SDL_Texture* currentTexture = isSpecialEnemy[i] ? specialEnemyTexture : enemyTexture;
            SDL_RenderCopy(renderer, currentTexture, NULL, &enemyRects[i]);
        }

        for (int i = 0; i < NUM_BULLETS; i++) {
            if (bulletRects[i].y >= 0) {
                SDL_RenderCopy(renderer, bulletTexture, NULL, &bulletRects[i]);
            }
        }



        // 繪製生命值
        for (int i = 0; i < playerLives; ++i) {
            SDL_Rect heartRect = {10 + i * 35, 10, 40, 40}; // 根據需要調整位置和尺寸
            SDL_RenderCopy(renderer, heartTexture, NULL, &heartRect);
        }

        // 繪製激活狀態的回血道具
        for (int i = 0; i < NUM_ENEMIES; i++) {
            if (healthPowerUpActive[i]) {
                SDL_RenderCopy(renderer, heartTexture, NULL, &healthPowerUps[i]);
            }
        }
   
        // 渲染計時器
        std::string timeText = "Time: " + std::to_string(currentTime) + "s";
        SDL_Surface* timeSurface = TTF_RenderText_Solid(font, timeText.c_str(), textColor);
        SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
        SDL_Rect timeRect = {10, windowHeight - 40, timeSurface->w, timeSurface->h};  // 假設計時器顯示在窗口左下角
        SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);
        SDL_FreeSurface(timeSurface);
        SDL_DestroyTexture(timeTexture);

        // 渲染得分文本
        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_Rect scoreRect = {windowWidth - scoreSurface->w - 10, 10, scoreSurface->w, scoreSurface->h};  // 假設得分顯示在窗口右上角
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);

        
        // 更新渲染器
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(specialEnemyTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();


    return 0;
}

        
