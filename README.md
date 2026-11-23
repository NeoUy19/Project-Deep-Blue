# Deep Blue Dive

A four-player Arduino game for our CS 361 group project. Two teams take turns as **Scientists**, who steer a research submarine, and **Invaders**, who attack it. The Invaders send secret coordinates, and the Scientists hear them as buzzer beeps and must decode them in time. If they get it wrong, they have to dodge torpedoes on the LCD.

The repo also contains **PDBSite**, a small companion website with the story behind the game and the rules.

## Team

| Board ID | Member   |
| -------- | -------- |
| 1        | Farshid  |
| 2        | Neo      |
| 3        | Kostia   |
| 4        | Santiago |

## Repository layout

```
Deep_Blue_Dive_90.ino   Arduino sketch (the same code runs on all 4 boards)
PDBSite/                Companion website (React + TypeScript + Vite)
```

## How to play

1. **Teams and roles.** Team A (boards 1 and 2) and Team B (boards 3 and 4) take turns as Scientists and Invaders.
2. **Start.** A Scientist presses the joystick button. The sub starts at 100% health on Easy.
3. **Attack.** The Invaders use their IR remotes to enter two secret digits, X and Y. The allowed range depends on the difficulty: Easy is 0–5, Medium is 0–7 and Hard is 0–9. Each level has 3 attacks.
4. **Decode.** The Scientists count the buzzer beeps for each digit and enter their guess with their IR remotes. If both digits are correct and in order, the attack is intercepted.
5. **Evade.** On a wrong guess, 3 torpedoes come in from the right side of the LCD. The Scientists use the joystick to move the sub between rows. Each hit costs 25% health, and the round ends when health reaches 0.
6. **Win.** The game moves from Easy to Medium to Hard as long as the sub survives. The remaining health becomes that team's points. After 4 turns, the team with the highest total wins.

## Arduino setup

### Hardware (per board)

- Arduino Uno (or compatible)
- 16×2 LCD (pins 12, 11, 5, 4, 3, 2)
- Passive buzzer on pin 6
- Analog joystick: X on A3, Y on A2, button on A1
- IR receiver on A0, plus an IR remote
- 3 red LEDs on pins 8, 9 and 10 (health), with the green built-in LED
- All 4 boards wired together over I2C (SDA/SCL and a shared GND)

### Libraries

- `LiquidCrystal` (built in)
- `Wire` (built in)
- `IRremote` (install it from the Arduino Library Manager)

### Uploading

The same sketch goes on every board. **Change `BOARD_ID` (1–4) at the top of `Deep_Blue_Dive_90.ino` before each upload.** Board 1 is always the I2C coordinator.

## Website (PDBSite)

A three-page static site:

- **Home:** an intro, credits and a QR code linking to this repo
- **Story:** the backstory, told in three chapters
- **How to Play:** the rules, shown as submarine cards

It uses React 19, TypeScript, React Router (`HashRouter`) and plain CSS, with an animated ASCII underwater background from [`ascii-magic-art`](https://www.npmjs.com/package/ascii-magic-art).

### Running it locally

Requires Node.js 20 or newer.

```bash
cd PDBSite
npm install
npm run dev       # start the dev server
npm run build     # type-check and build to dist/
npm run preview   # preview the production build
npm run lint      # run ESLint
```
