#include <SDL.h> // "老大哥" SDL庫，讓我們可以製作遊戲的窗口和各種圖形。
#include <SDL_ttf.h> // 用來在遊戲裡顯示文字的工具包。
#include <SDL_image.h> // 這個是用來加載和處理圖片的，讓我們的遊戲不只是文字那麼無聊。
#include <stdlib.h> // 標準庫，有很多基本的功能，比如隨機數。
#include <time.h> // 時間庫，可以用來搞定一些與時間有關的操作。
#include <string> // 字符串庫，用來處理一堆字。

// 下面這個函數是用來加載圖片的，讓圖片能在遊戲裡面顯示出來。
SDL_Texture* LoadTexture(const std::string &filePath, SDL_Renderer* renderer) {
    SDL_Surface* tempSurface = IMG_Load(filePath.c_str()); // 先用IMG_Load把圖片搞成一個表面（surface）。
    if (!tempSurface) { // 如果沒能加載圖片，就報錯。
        SDL_Log("Unable to load image %s! SDL_image Error: %s\n", filePath.c_str(), IMG_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface); // 把表面轉換成紋理（texture），這樣才能在遊戲裡用。
    SDL_FreeSurface(tempSurface); // 把剛才那個表面刪了，因為我們已經不需要它了。

    if (!texture) { // 如果紋理沒有成功創建，也報錯。
        SDL_Log("Unable to create texture from %s! SDL Error: %s\n", filePath.c_str(), SDL_GetError());
    }

    return texture; // 把紋理返回，這樣其他地方就能用這個紋理了。
}

int main(int argc, char* argv[]) {
    // 先初始化所有我們需要用到的SDL庫。
    SDL_Init(SDL_INIT_VIDEO); // 啟動SDL的視頻部分。
    IMG_Init(IMG_INIT_PNG); // 讓SDL_image支持PNG格式的圖片。
    TTF_Init(); // 啟動SDL_ttf，這樣我們就可以在遊戲裡顯示文字了。
    
    // 打開一個字體文件，並設定字體的大小。
    TTF_Font* font = TTF_OpenFont("/Users/linyili/Desktop/game/FZSJ-BHTJW.TTF", 40); // 這裡假設字體檔案在你電腦上的某個位置。

    // 下面一堆變數，主要用來記錄遊戲進行的時間。
    Uint32 startTime = 0; // 遊戲開始的時間。
    Uint32 currentTime = 0; // 現在的時間。

    // 設定文字的顏色，這裡用的是白色。
    SDL_Color textColor = {255, 255, 255, 255};

    // 定義遊戲窗口的大小。
    int windowWidth = 600;
    int windowHeight = 800;

    // 創建遊戲窗口和渲染器。窗口是玩家看到遊戲畫面的地方，渲染器則是用來把圖形畫到窗口上。
    SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    // 加載一堆圖片，這些圖片會在遊戲裡被用到。
    SDL_Texture* backgroundTexture = LoadTexture("/Users/linyili/Desktop/game/background.png", renderer); // 背景圖
    SDL_Texture* heartTexture = LoadTexture("/Users/linyili/Desktop/game/love.png", renderer); // 表示生命值的圖片
    SDL_Texture* playerTexture = LoadTexture("/Users/linyili/Desktop/game/main.png", renderer); // 玩家角色的圖片
    SDL_Texture* enemyTexture = LoadTexture("/Users/linyili/Desktop/game/blue.png", renderer); // 敵人的圖片
    SDL_Texture* specialEnemyTexture = LoadTexture("/Users/linyili/Desktop/game/green.png", renderer); // 特殊敵人的圖片
    SDL_Texture* bulletTexture = LoadTexture("/Users/linyili/Desktop/game/magic.png", renderer); // 子彈的圖片
    SDL_Texture* mainMenuBackground = LoadTexture("/Users/linyili/Desktop/game/mainBackground.png", renderer); // 主菜單的背景圖

    // 定義玩家的一些屬性，比如位置和大小。
    int playerX = 270; // 玩家的水平位置
    int playerY = 640; // 玩家的垂直位置
    int playerWidth = 100; // 玩家的寬度
    int playerHeight = 100; // 玩家的高度
    int playerSpeed = 5; // 玩家移動的速度

    // 玩家的生命值和得分。
    int playerLives = 5; // 玩家有5條命
    int score = 0; // 初始得分是0

    // 玩家的矩形，SDL用矩形來表示遊戲中的物體。
    SDL_Rect playerRect = { playerX, playerY, playerWidth, playerHeight };

    // 定義敵人的數量和屬性。
    const int NUM_ENEMIES = 6; // 敵人的數量
    SDL_Rect enemyRects[NUM_ENEMIES]; // 存儲敵人矩形的數組
    int enemyWidth = 50; // 敵人的寬度
    int enemyHeight = 50; // 敵人的高度
    int enemySpeedX[NUM_ENEMIES]; // 敵人在水平方向上的速度
    int enemySpeedY[NUM_ENEMIES]; // 敵人在垂直方向上的速度

    // 這裡有一些關於特殊敵人和回血道具的設置。
    bool isSpecialEnemy[NUM_ENEMIES] = {false}; // 每個敵人是否是特殊敵人的標記
    SDL_Rect healthPowerUps[NUM_ENEMIES]; // 回血道具的矩形
    bool healthPowerUpActive[NUM_ENEMIES] = {false}; // 每個回血道具是否激活的標記
    int healthPowerUpWidth = 20; // 回血道具的寬度
    int healthPowerUpHeight = 20; // 回血道具的高度
    const int SPECIAL_ENEMY_CHANCE = 8; // 特殊敵人出現的機率

    // 初始化敵人和回血道具。這裡用了一些隨機數來確定敵人的初始位置和速度。
    for (int i = 0; i < NUM_ENEMIES; i++) {
        enemyRects[i].x = rand() % (windowWidth - enemyWidth); // 敵人的初始水平位置是隨機的
        enemyRects[i].y = rand() % (windowHeight / 2 - enemyHeight); // 敵人的初始垂直位置也是隨機的
        enemySpeedX[i] = rand() % 3 + rand() % 2; // 敵人的水平速度是隨機的
        enemySpeedY[i] = rand() % 3 + rand() % 2; // 敵人的垂直速度也是隨機的
        enemyRects[i] = (SDL_Rect){enemyRects[i].x, enemyRects[i].y, enemyWidth, enemyHeight}; // 把敵人的位置和大小設定到矩形裡

        // 隨機決定哪些敵人是特殊的
        if (rand() % SPECIAL_ENEMY_CHANCE == 0) {
            isSpecialEnemy[i] = true;
        }
        // 初始化回血道具為非激活狀態
        healthPowerUps[i] = (SDL_Rect){-1, -1, healthPowerUpWidth, healthPowerUpHeight};
    }

    // 定義子彈的數量、發射間隔和屬性。
    const int NUM_BULLETS = 20; // 子彈的數量
    const Uint32 bulletFireInterval = 100; // 子彈的發射間隔，單位是毫秒
    Uint32 lastBulletFireTime = 0; // 上一次發射子彈的時間
    SDL_Rect bulletRects[NUM_BULLETS]; // 存儲子彈矩形的數組
    int bulletWidth = 26; // 子彈的寬度
    int bulletHeight = 40; // 子彈的高度
    int bulletSpeedY = -10; // 子彈在垂直方向上的速度

    // 初始化子彈矩形數組。這裡的子彈都是“未激活”的，意味著它們不會在遊戲畫面上出現。
    for (int i = 0; i < NUM_BULLETS; i++) {
        bulletRects[i].x= -1; // 子彈的初始位置在畫面之外
        bulletRects[i].y= -1;
        bulletRects[i] = (SDL_Rect){ bulletRects[i].x, bulletRects[i].y, bulletWidth, bulletHeight };
    }

    // 定義敵人矩形重生時間數組。這個用來記錄每個敵人重生的時間。
    Uint32 enemyRespawnTime[NUM_ENEMIES];

    // 初始化敵人矩形重生時間數組。一開始，所有敵人都是立即可用的，所以這裡不需要設置具體的重生時間。
    for (int i = 0; i < NUM_ENEMIES; i++) {
        enemyRespawnTime[i] = 0;
    }

    // 等待窗口關閉事件。這個bool變數用來控制遊戲是否繼續運行。
    bool quit = false;
    SDL_Event event; // SDL_Event用來處理各種事件，比如鍵盤按鍵、鼠標點擊等。

    // 主菜單循環。這個循環負責顯示遊戲的主菜單，等待玩家做出選擇。
    bool showMainMenu = true;
    while (showMainMenu) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) { // 不斷檢查是否有事件發生
            if (event.type == SDL_QUIT) { // 如果是退出事件，比如點擊窗口的關閉按鈕
                showMainMenu = false;
                return 0;  // 退出遊戲
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) { // 如果按下了回車鍵
                showMainMenu = false;  // 開始遊戲
            }
        }
        // 渲染主菜單背景
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, mainMenuBackground, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    

    // 清除屏幕。這個操作會清除渲染器上的所有內容，為新的一幀做準備。
    SDL_RenderClear(renderer);
    

    // 初始化計時器。從這一刻開始計算遊戲時間。
    startTime = SDL_GetTicks(); // 獲取遊戲開始的時間


    // 遊戲主循環。這個循環是遊戲的核心，負責遊戲的運行直到玩家選擇退出。
    while (!quit) {
        while (SDL_PollEvent(&event)) { // 不斷檢查事件
            if (event.type == SDL_QUIT) { // 如果是退出事件
                quit = true;
            }
            if (event.type == SDL_KEYDOWN) { // 如果是鍵盤按下事件
                switch (event.key.keysym.sym) { // 根據按下的鍵來決定動作
                    case SDLK_a: // 如果按下A鍵，玩家向左移動
                        playerX -= playerSpeed;
                        break;
                    case SDLK_d: // 如果按下D鍵，玩家向右移動
                        playerX += playerSpeed;
                        break;
                    case SDLK_w: // 如果按下W鍵，玩家向上移動
                        playerY -= playerSpeed;
                        break;
                    case SDLK_s: // 如果按下S鍵，玩家向下移動
                        playerY += playerSpeed;
                        break;
                    case SDLK_SPACE: // 如果按下空格鍵，玩家發射子彈
                        // 在玩家矩形的頂部中央位置創建一個新的子彈矩形
                        Uint32 currentTime = SDL_GetTicks();
                        if (currentTime - lastBulletFireTime >= bulletFireInterval) { // 確保子彈發射有一定間隔
                            // 發射子彈的邏輯
                            for (int i = 0; i < NUM_BULLETS; i++) {
                                if (bulletRects[i].y < 0) { // 找到一個未激活的子彈
                                    bulletRects[i].x = playerX + playerWidth / 2 - bulletWidth / 2; // 設置子彈的位置
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
        // 更新經過的時間（秒）
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

        // 檢測子彈矩形和敵人矩形的碰撞
        for (int i = 0; i < NUM_BULLETS; i++) {
            if (bulletRects[i].y >= 0) {
                for (int j = 0; j < NUM_ENEMIES; j++) {
                    if (SDL_HasIntersection(&bulletRects[i], &enemyRects[j])) { // 如果子彈和敵人相交
                        bulletRects[i].y = -1;  // 子彈消失
                        score += 10;  // 增加得分

                        if (isSpecialEnemy[j]) {  // 如果擊中的是特殊敵人
                            // 在特殊敵人的位置生成回血道具，並激活它
                            healthPowerUps[j] = (SDL_Rect){enemyRects[j].x, enemyRects[j].y, healthPowerUpWidth, healthPowerUpHeight};
                            healthPowerUpActive[j] = true;
                            isSpecialEnemy[j] = false;  // 將敵人標記為非特殊敵人，防止重複生成回血道具
                        }

                        // 重置敵人位置並隨機決定是否變為特殊敵人
                        enemyRects[j].x = rand() % (windowWidth - enemyWidth); // 敵人在隨機位置重生
                        enemyRects[j].y = rand() % (windowHeight / 2 - enemyHeight);
                        isSpecialEnemy[j] = (rand() % SPECIAL_ENEMY_CHANCE == 0);  // 有一定概率變成特殊敵人

                        // 如果這個敵人變成了特殊敵人，則初始化回血道具
                        if (isSpecialEnemy[j]) {
                            healthPowerUps[j] = (SDL_Rect){enemyRects[j].x, enemyRects[j].y, healthPowerUpWidth, healthPowerUpHeight};
                            healthPowerUpActive[j] = false;  // 初始時不激活回血道具
                        }
                    }
                }
            }
        }

        // 檢測敵人矩形和玩家矩形的碰撞
        for (int i = 0; i < NUM_ENEMIES; i++) {
            if (SDL_HasIntersection(&playerRect, &enemyRects[i])) { // 如果玩家和敵人相交
                playerLives--; // 玩家生命值減少
                enemyRects[i].x = rand() % (windowWidth - enemyWidth); // 敵人在隨機位置重生
                enemyRects[i].y = rand() % (windowHeight / 2 - enemyHeight);
                enemyRespawnTime[i] = SDL_GetTicks() + 2000; // 設置重生時間
                if (playerLives == 0) {
                    quit = true; // 生命值為0，遊戲結束
                }
            }
        }

        // 檢測玩家與回血道具的碰撞
        for (int i = 0; i < NUM_ENEMIES; i++) {
            if (healthPowerUpActive[i] && SDL_HasIntersection(&playerRect, &healthPowerUps[i])) { // 如果玩家和回血道具相交
                playerLives += 1; // 玩家回復生命值
                healthPowerUpActive[i] = false; // 回血道具被拾取，設置為非激活狀態
            }
        }

        // 清除渲染器上的所有內容，準備繪製新的一幀
        SDL_RenderClear(renderer);

        // 使用背景紋理填充屏幕
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
        

        // 繪製敵人和回血道具
        for (int i = 0; i < NUM_ENEMIES; i++) {
            // 根據敵人是否為特殊敵人來設置不同的紋理
            SDL_Texture* currentTexture = isSpecialEnemy[i] ? specialEnemyTexture : enemyTexture;
            SDL_RenderCopy(renderer, currentTexture, NULL, &enemyRects[i]);

            // 渲染激活的回血道具
            if (healthPowerUpActive[i]) {
                SDL_RenderCopy(renderer, heartTexture, NULL, &healthPowerUps[i]);
            }
        } 

        // 更新玩家位置並繪製
        const Uint8* state = SDL_GetKeyboardState(NULL); // 獲取當前鍵盤狀態
        if (state[SDL_SCANCODE_A]) playerX -= playerSpeed; // 如果A鍵被按下，玩家向左移動
        if (state[SDL_SCANCODE_D]) playerX += playerSpeed; // 如果D鍵被按下，玩家向右移動
        if (state[SDL_SCANCODE_W]) playerY -= playerSpeed; // 如果W鍵被按下，玩家向上移動
        if (state[SDL_SCANCODE_S]) playerY += playerSpeed; // 如果S鍵被按下，玩家向下移動

        // 檢查玩家矩形是否超出窗口邊界
        playerRect.x = playerX = (playerX < 0) ? 0 : ((playerX > windowWidth - playerWidth) ? windowWidth - playerWidth : playerX);
        playerRect.y = playerY = (playerY < 0) ? 0 : ((playerY > windowHeight - playerHeight) ? windowHeight - playerHeight : playerY);
        
        //繪製玩家
        SDL_RenderCopy(renderer, playerTexture, NULL, &playerRect);

        // 繪製子彈
        for (int i = 0; i < NUM_BULLETS; i++) {
            if (bulletRects[i].y >= 0) { // 只繪製在畫面上的子彈
                SDL_RenderCopy(renderer, bulletTexture, NULL, &bulletRects[i]);
            }
        }

        // 繪製生命值
        for (int i = 0; i < playerLives; ++i) {
            SDL_Rect heartRect = {10 + i * 35, 10, 40, 40}; // 根據需要調整位置和尺寸
            SDL_RenderCopy(renderer, heartTexture, NULL, &heartRect); // 繪製每一顆生命值
        }

        // 渲染計時器
        std::string timeText = "Time: " + std::to_string(currentTime) + "s"; // 將經過的時間轉換成文本
        SDL_Surface* timeSurface = TTF_RenderText_Solid(font, timeText.c_str(), textColor); // 用字體和顏色渲染文本到一個表面上
        SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface); // 將表面轉換成紋理
        SDL_Rect timeRect = {10, windowHeight - 40, timeSurface->w, timeSurface->h};  // 計時器顯示的位置
        SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect); // 繪製計時器
        SDL_FreeSurface(timeSurface); // 釋放表面資源
        SDL_DestroyTexture(timeTexture); // 釋放紋理資源

        // 渲染得分文本
        std::string scoreText = "Score: " + std::to_string(score); // 將得分轉換成文本
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor); // 用字體和顏色渲染文本到一個表面上
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface); // 將表面轉換成紋理
        SDL_Rect scoreRect = {windowWidth - scoreSurface->w - 10, 10, scoreSurface->w, scoreSurface->h};  // 得分顯示的位置
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect); // 繪製得分
        SDL_FreeSurface(scoreSurface); // 釋放表面資源
        SDL_DestroyTexture(scoreTexture); // 釋放紋理資源

        // 更新渲染器，顯示這一幀的所有內容
        SDL_RenderPresent(renderer);
    }

    // 遊戲結束後，釋放所有使用的資源
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(specialEnemyTexture);
    SDL_DestroyTexture(bulletTexture);
    SDL_DestroyRenderer(renderer); // 釋放渲染器
    SDL_DestroyWindow(window); // 關閉窗口
    IMG_Quit(); // 退出SDL_image
    SDL_Quit(); // 退出SDL

    return 0; // 程式結束
}
