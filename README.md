# Pong-Raylib-Cpp
A graphical Pong game built in C++ using the Raylib library, featuring a polished visual style with glowing effects, motion trails, and a full game flow including menus, a win condition, and a persistent high score system.

# Features

Play against an AI-controlled CPU paddle with adaptive tracking
Smooth paddle movement with interpolated visual lag for a polished feel
Ball trail effect and glow rendering for dynamic motion visuals
First to 10 points wins, triggering a dedicated Game Over screen
Persistent high score leaderboard saved across sessions to a local file
Flash effect on every score point for satisfying visual feedback


# Technologies Used

C++
Raylib (graphics, input, window management)
Object-Oriented Programming (OOP)
File Handling (fstream)
STL Vectors


# How to Play
W / S or Arrow Keys — Move your paddle up and down
ENTER — Confirm menu selections
ESC — Return to the main menu mid-game

Scoring & Win Condition
EventResultBall passes opponent's paddle+1 pointFirst to reach 10 pointsWins the matchMatch end or exit mid-gameScore saved to leaderboard
