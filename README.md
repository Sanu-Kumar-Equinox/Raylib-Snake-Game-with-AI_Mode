# Snake Game with AI

A modern Snake game built in **C++** using the **Raylib** graphics library.

This project includes:
-  Classic Player Mode
-  AI Mode with future-path prediction
-  Dynamic food spawning
-  Collision detection
-  Main menu system
-  Credits page

---

#  Features

##  Player Mode
- Classic Snake gameplay
- Keyboard controls
- Collision detection with walls
- Self-collision detection
- Snake growth when food is eaten

##  AI Mode
The AI automatically controls the snake and attempts to survive while collecting food.

Features:
- Food-seeking behavior
- Safe path evaluation
- Collision avoidance
- Future-state prediction
- Recursive look-ahead algorithm

---

#  AI Algorithm

The AI uses several layers of decision making:

### Step 1: Food Tracking
The AI first attempts to move directly toward the food.

### Step 2: Safety Check
Before moving, it checks whether the next position would cause:
- Wall collision
- Body collision

### Step 3: Future Prediction
If the direct path is unsafe, the AI:

1. Simulates all possible directions.
2. Recursively predicts future positions.
3. Calculates how many safe paths remain.
4. Chooses the direction with the highest survival score.

This creates a more intelligent and survivable snake compared to a basic food-following algorithm.

---

#  Controls

| Key | Action |
|------|--------|
| ↑ | Move Up |
| ↓ | Move Down |
| ← | Move Left |
| → | Move Right |
| Mouse Left Click | Menu Navigation |
| X Button on screen | Return to Main Menu |

---

#  Built With

- C++
- Raylib
- STL Vector

---

#  Visuals

You can get visuals on mu Instagram account

LINK: https://www.instagram.com/lord_equinoxl?igsh=YTVwenNkc2Jwb2oy
