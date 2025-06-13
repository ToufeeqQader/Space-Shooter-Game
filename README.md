# 🚀 Space Shooter Game

A 2D arcade space shooter game in C++ with SFML, featuring multiple levels, power-ups, and boss fights.

## 🎮 Features
- Multiple levels with challenging enemy waves
- Different types of enemies with unique behaviors
- Boss monster fights
- Power-ups: Extra lives, special attacks, shields
- High Score system with badges (Gold, Silver, Bronze)
- Name input, Pause, Instructions, and Game Over screens
- Built using SFML for graphics and input handling

## ▶️ Controls

| Action        | Key         |
| ------------- | ----------- |
| Move Up       | W           |
| Move Down     | S           |
| Move Left     | A           |
| Move Right    | D           |
| Shoot         | Spacebar    |
| Pause         | Escape      |
| Back to Menu  | M           |

## 📁 Folder Structure
SpaceShooter/
├── assets/ # Game images, fonts
├── Source.cpp # Main C++ code
├── highscores.txt # High score file
├── README.md # This file


## ⚙️ How to Run the Game
1. Install **SFML** library.
   - For Windows → https://www.sfml-dev.org/
   - For Linux → 
     ```bash
     sudo apt-get install libsfml-dev
     ```
2. Compile the code:
   ```bash
   g++ Source.cpp -o SpaceShooter -lsfml-graphics -lsfml-window -lsfml-system

📜 License
This project is for learning and educational purposes.

