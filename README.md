# 🏓 Pong — VS Computer Edition

> A graphical Pong game built in C++ using the Raylib library, featuring a polished visual style with glowing effects, motion trails, and a complete game flow including menus, win condition, and a persistent high score leaderboard.

---

## 📸 Preview

```
  +============================================+
  |                   PONG                     |
  |           VS Computer EDITION              |
  |                                            |
  |        [ Start Game  ]                     |
  |        [ High Scores ]                     |
  |        [    Exit     ]                     |
  +============================================+
```

---

## ✨ Features

- 🤖 Play against an AI-controlled CPU paddle with real-time ball tracking
- 🎨 Glow rendering on paddles and ball for a polished neon aesthetic
- 🌀 Ball motion trail effect for dynamic, satisfying gameplay feel
- ⚡ Flash effect on every scored point for visual feedback
- 🏆 First to **10 points** wins, triggering a dedicated Game Over screen
- 💾 Persistent high score leaderboard saved to `highscores.txt` across sessions
- 🖥️ Full game state management — Menu, Playing, Game Over, High Scores

---

## 🛠️ Technologies Used

| Technology | Purpose |
|---|---|
| C++ | Core language |
| [Raylib](https://www.raylib.com/) | Graphics, input & window management |
| OOP (Classes & Inheritance) | Ball, paddle, and game architecture |
| `fstream` | High score persistence |
| STL `vector` | Leaderboard management |

---

## 🎮 Controls

| Action | Key |
|---|---|
| Move paddle up | `W` or `↑` |
| Move paddle down | `S` or `↓` |
| Confirm menu selection | `ENTER` |
| Return to menu mid-game | `ESC` |

---

## 📊 Scoring & Win Condition

| Event | Result |
|---|---|
| Ball passes opponent's paddle | +1 point |
| First to reach 10 points | Wins the match |
| Match end or mid-game exit | Score saved to leaderboard |

---

## 🖥️ Game States

| State | Description |
|---|---|
| `Menu` | Main menu with navigation |
| `Playing` | Active match vs CPU |
| `Game Over` | Win/loss result with final score |
| `High Scores` | Top 5 all-time results |
| `Exit` | Clean shutdown with score save |

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).
