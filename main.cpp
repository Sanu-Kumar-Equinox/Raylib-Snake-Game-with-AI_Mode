#include <iostream>
#include <raylib.h>
#include <vector>

// button rectangles:
Rectangle start_buttion     = {175, 200, 250, 50};
Rectangle ai_buttion        = {175, 270, 250, 50};
Rectangle credits_buttion   = {175, 340, 250, 50};
Rectangle exit_buttion      = {175, 410, 250, 50};
Rectangle main_menu_buttion = {215, 350, 150, 30};
Rectangle start_back_button = {540, 0, 40, 20};

// snake body:
std::vector<Vector2> snakebody = {{300, 300}, {280, 300}, {260, 300}};
Vector2 snakespeed = {0, 0}; 
Vector2 food = {300, 200};

double LastUpdateTime = 0.0;
double MovementDelay = 0.2;

// the main game state variable:
enum GameState {
    MENU,
    START,
    AI_MODE,
    CREDITS,
    EXIT
};

GameState gamestate = MENU;

void GameOver() {
    snakebody = {{300, 300}, {280, 300}, {260, 300}};
    snakespeed = {0, 0};
    food = {300, 200};
    gamestate = MENU;
}

bool IsStepSafe(Vector2 nextPosition) {
    if (nextPosition.x < 25 || nextPosition.x >= 575 || 
        nextPosition.y < 20 || nextPosition.y >= 570) {
        return false;
    }

    for (int i = 1; i < snakebody.size(); i++) {
        if (nextPosition.x == snakebody[i].x && nextPosition.y == snakebody[i].y) {
            return false; 
        }
    }

    return true; 
}

// This function recursively simulates steps into the future
int PredictFutureSafety(Vector2 currentPos, int currentDepth, int maxDepth, std::vector<Vector2>& simulatedBody) {
    if (currentDepth == maxDepth) return 1;

    Vector2 directions[4] = { {20, 0}, {-20, 0}, {0, 20}, {0, -20} };
    int totalSafePathsFound = 0;

    for (int i = 0; i < 4; i++) {
        Vector2 nextPos = { currentPos.x + directions[i].x, currentPos.y + directions[i].y };

        // Check if this simulated step hits a wall
        if (nextPos.x < 25 || nextPos.x >= 575 || nextPos.y < 20 || nextPos.y >= 570) {
            continue; 
        }

        // Check if this simulated step hits the snake's body
        bool hitsBody = false;
        for (int b = 1; b < simulatedBody.size(); b++) {
            if (nextPos.x == simulatedBody[b].x && nextPos.y == simulatedBody[b].y) {
                hitsBody = true;
                break;
            }
        }

        if (!hitsBody) {
            // "Move" our simulated body forward to accurately test the next step
            Vector2 oldTail = simulatedBody.back();
            for (int b = simulatedBody.size() - 1; b > 0; b--) {
                simulatedBody[b] = simulatedBody[b - 1];
            }
            simulatedBody[0] = nextPos;

            // Go deeper into the next step!
            totalSafePathsFound += PredictFutureSafety(nextPos, currentDepth + 1, maxDepth, simulatedBody);

            // Undo the simulated body move (backtrack) so the next loop cycle tests cleanly
            for (int b = 0; b < simulatedBody.size() - 1; b++) {
                simulatedBody[b] = simulatedBody[b + 1];
            }
            simulatedBody[simulatedBody.size() - 1] = oldTail;
            simulatedBody[0] = currentPos;
        }
    }

    return totalSafePathsFound;
}

void DrawAI_Mode() {
    Vector2 MousePos = GetMousePosition();
    ClearBackground(DARKGREEN);

    // 1. Draw the Board & Walls
    DrawRectangle(25, 20, 550, 555, GREEN);
    DrawRectangle(25, 20, 550, 5, BLACK);
    DrawRectangle(25, 20, 5, 550, BLACK);
    DrawRectangle(25, 570, 550, 5, BLACK);
    DrawRectangle(575, 20, 5, 555, BLACK);

    // 2. Draw Back Button
    DrawRectangle(540, 0, 40, 20, RED);
    DrawText("X", 550, 0, 30, BLACK);

    // 3. Draw Game Objects
    DrawRectangle(food.x, food.y, 20, 20, RED);
    for (int i = 0; i < snakebody.size(); i++) {
        DrawRectangle(snakebody[i].x, snakebody[i].y, 20, 20, BLACK);
    }

    double CurrentTime = GetTime();

    if (CurrentTime - LastUpdateTime >= MovementDelay) {
        Vector2 desiredSpeed = snakespeed;

        // Step A: Determine the ideal direction to get closer to the food
        if (food.x > snakebody[0].x) {
            desiredSpeed = {20, 0};   
        } else if (food.x < snakebody[0].x) {
            desiredSpeed = {-20, 0};  
        } else if (food.x == snakebody[0].x) {
            if (food.y > snakebody[0].y) desiredSpeed = {0, 20}; 
            else if (food.y < snakebody[0].y) desiredSpeed = {0, -20}; 
        }

        // Step B: Calculate what the next head position would be
        Vector2 nextHeadPos = { snakebody[0].x + desiredSpeed.x, snakebody[0].y + desiredSpeed.y };

        // Step C: If the ideal move is a death trap, search for an alternative!
        if (!IsStepSafe(nextHeadPos)) {
            Vector2 directions[4] = { {20, 0}, {-20, 0}, {0, 20}, {0, -20} };
            bool foundSafeMove = false;
            int bestSafetyScore = -1; 

            for (int d = 0; d < 4; d++) {
                Vector2 testPos = { snakebody[0].x + directions[d].x, snakebody[0].y + directions[d].y };

                if (IsStepSafe(testPos)) {
                    // Create a temporary copy of the snake body to safely simulate movements on
                    std::vector<Vector2> simulatedBody = snakebody;
                    
                    // Fire off our 5-step lookahead tree!
                    int safetyScore = PredictFutureSafety(testPos, 1, 5, simulatedBody);

                    // Pick the direction that branches into the absolute highest number of safe survival states
                    if (safetyScore > bestSafetyScore) {
                        bestSafetyScore = safetyScore;
                        desiredSpeed = directions[d];
                        foundSafeMove = true;
                    }
                }
            }
            
            // Panic option: If absolutely no move is safe, just keep going straight and accept fate
            if (!foundSafeMove) {
                desiredSpeed = snakespeed;
            }
        }

        // Apply our verified safe speed choice
        snakespeed = desiredSpeed;

        // Move body segments forward
        for (int i = snakebody.size() - 1; i > 0; i--) {
            snakebody[i] = snakebody[i - 1];
        }

        // movement
        snakebody[0].x = snakebody[0].x + snakespeed.x;
        snakebody[0].y = snakebody[0].y + snakespeed.y;

        // Collision Checks
        if (snakespeed.x != 0 || snakespeed.y != 0) {
            if (snakebody[0].x < 25 || snakebody[0].x >= 575 || snakebody[0].y < 20 || snakebody[0].y >= 570) {
                GameOver(); 
            }
            for (int i = 1; i < snakebody.size(); i++) {
                if (snakebody[0].x == snakebody[i].x && snakebody[0].y == snakebody[i].y) {
                    GameOver();
                }
            }
        }

        // Eat food check
        if (snakebody[0].x == food.x && snakebody[0].y == food.y) {
            snakebody.push_back(snakebody[snakebody.size() - 1]);

            bool foodOnSnake = true;
            while (foodOnSnake) {
                food.x = 40 + (GetRandomValue(0, 25) * 20);
                food.y = 40 + (GetRandomValue(0, 25) * 20);

                foodOnSnake = false;
                for (int i = 0; i < snakebody.size(); i++) {
                    if (food.x == snakebody[i].x && food.y == snakebody[i].y) {
                        foodOnSnake = true; 
                        break;
                    }
                }
            }
        }

        LastUpdateTime = CurrentTime;
    }

    // Handle back button exit
    if (CheckCollisionPointRec(MousePos, start_back_button)) {
        DrawRectangle(540, 0, 40, 20, MAROON);
        DrawText("X", 550, 0, 30, BLACK);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gamestate = MENU;
            GameOver();
        }
    }
}

void DrawCredits() {
    Vector2 MousePos = GetMousePosition();
    ClearBackground(BLACK);
    
    DrawText("CREDITS", 125, 65, 75, LIGHTGRAY);
    DrawRectangle(122, 130, 352, 5, LIGHTGRAY);

    DrawText("CREATOR", 180, 225, 45, LIGHTGRAY);
    DrawText("Sanu Kumar", 188, 280, 35, RAYWHITE);

    DrawRectangle(215, 350, 150, 30, RED);
    DrawText("Main Menu", 229, 353, 25, RAYWHITE);

    if (CheckCollisionPointRec(MousePos, main_menu_buttion)) {
        DrawRectangle(215, 350, 150, 30, MAROON);
        DrawText("Main Menu", 229, 353, 25, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gamestate = MENU;
        }
    }
}

void DrawExit() {
    CloseWindow();
    exit(0);
}

void DrawStart() {
    Vector2 MousePos = GetMousePosition();
    ClearBackground(DARKGREEN);

    // drawing central rectangle:
    DrawRectangle(25, 20, 550, 555, GREEN);

    // drawing walls:
    DrawRectangle(25, 20, 550, 5, BLACK);
    DrawRectangle(25, 20, 5, 550, BLACK);
    DrawRectangle(25, 570, 550, 5, BLACK);
    DrawRectangle(575, 20, 5, 555, BLACK);

    // drawing back button:
    DrawRectangle(540, 0, 40, 20, RED);
    DrawText("X", 550, 0, 30, BLACK);

    DrawRectangle(food.x, food.y, 20, 20, RED);

    for (int i = 0; i < snakebody.size(); i++) {
        DrawRectangle(snakebody[i].x, snakebody[i].y, 20, 20, BLUE);
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        snakespeed.x = 20; 
        snakespeed.y = 0; 
    }
    if (IsKeyPressed(KEY_LEFT)) {
        snakespeed.x = -20;
        snakespeed.y = 0;
    }
    if (IsKeyPressed(KEY_UP)) {
        snakespeed.x = 0;
        snakespeed.y = -20; 
    }
    if (IsKeyPressed(KEY_DOWN)) {
        snakespeed.x = 0;
        snakespeed.y = 20;  
    }

    double CurrentTime = GetTime();

    if (CurrentTime - LastUpdateTime >= MovementDelay) {
        for (int i = snakebody.size() - 1; i > 0; i--) {
            snakebody[i] = snakebody[i - 1];
        }

        snakebody[0].x = snakebody[0].x + snakespeed.x;
        snakebody[0].y = snakebody[0].y + snakespeed.y;

        if (snakespeed.x != 0 || snakespeed.y != 0) {
            if (snakebody[0].x < 25 || snakebody[0].x >= 575 || snakebody[0].y < 20 || snakebody[0].y >= 570) {
                GameOver();
            }

            for (int i = 1; i < snakebody.size(); i++) {
                if (snakebody[0].x == snakebody[i].x && snakebody[0].y == snakebody[i].y) {
                    GameOver(); 
                }
            }
        }

        if (snakebody[0].x == food.x && snakebody[0].y == food.y) {
            snakebody.push_back(snakebody[snakebody.size() - 1]);

            bool foodOnSnake = true;
            while (foodOnSnake) {
                food.x = 40 + (GetRandomValue(0, 25) * 20);
                food.y = 40 + (GetRandomValue(0, 25) * 20);

                foodOnSnake = false;
                for (int i = 0; i < snakebody.size(); i++) {
                    if (food.x == snakebody[i].x && food.y == snakebody[i].y) {
                        foodOnSnake = true; 
                        break;
                    }
                }
            }
        }

        LastUpdateTime = CurrentTime;
    }

    if (CheckCollisionPointRec(MousePos, start_back_button)) {
        DrawRectangle(540, 0, 40, 20, MAROON);
        DrawText("X", 550, 0, 30, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gamestate = MENU;
            GameOver();
        }
    }
}

void DrawMenu() {
    Vector2 MousePos = GetMousePosition();
    ClearBackground(DARKGRAY);
    
    // drawing title:
    DrawText("Snake Game!", 150, 85, 50, BLACK);
    
    // drawing buttons:
    DrawRectangle(175, 200, 250, 50, BLACK);
    DrawRectangle(175, 270, 250, 50, BLACK);
    DrawRectangle(175, 340, 250, 50, BLACK);
    DrawRectangle(175, 410, 250, 50, BLACK);

    // text on buttons:
    DrawText("START", 260, 213, 25, RAYWHITE);
    DrawText("AI MODE", 248, 283, 25, RAYWHITE);
    DrawText("CREDITS", 248, 353, 25, RAYWHITE);
    DrawText("EXIT",  270, 423, 25, RAYWHITE);

    if (CheckCollisionPointRec(MousePos, start_buttion)) {
        DrawRectangle(175, 200, 250, 50, RAYWHITE);
        DrawText("START", 260, 213, 25, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gamestate = START;
        }
    }
    if (CheckCollisionPointRec(MousePos, ai_buttion)) {
        DrawRectangle(175, 270, 250, 50, RAYWHITE);
        DrawText("AI MODE", 248, 283, 25, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gamestate = AI_MODE;
        }
    }
    if (CheckCollisionPointRec(MousePos, credits_buttion)) {
        DrawRectangle(175, 340, 250, 50, RAYWHITE);
        DrawText("CREDITS", 248, 353, 25, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gamestate = CREDITS;
        }
    }
    if (CheckCollisionPointRec(MousePos, exit_buttion)) {
        DrawRectangle(175, 410, 250, 50, RAYWHITE);
        DrawText("EXIT", 270, 423, 25, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            gamestate = EXIT;
        }
    }
}

int main() {
    InitWindow(600, 600, "Snake Game");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        
        switch (gamestate) {
            case MENU    : DrawMenu();    break;
            case START   : DrawStart();   break;
            case AI_MODE : DrawAI_Mode(); break;
            case CREDITS : DrawCredits(); break;
            case EXIT    : DrawExit();    break;
        };

        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
