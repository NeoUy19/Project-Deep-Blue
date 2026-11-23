#include <Arduino.h>
// ---------------- DEEP BLUE DIVE ----------------
// Shared sketch: upload with BOARD_ID = 1, 2, 3, or 4.
// Team A = boards 1+2; Team B = boards 3+4.
// Board 1 is always the I2C coordinator.
// Tested hardware pinout comes from the original shared sketch.

#include <LiquidCrystal.h>
#include <IRremote.hpp>
#include <Wire.h>
#include <stdio.h>
#include <string.h>

// ---------------- BOARD CONFIGURATION ----------------
///%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

const byte BOARD_ID = 1;  // PLAYER ID each board has it's unique ID

const byte I2C_ADDRESS[5] = {0, 0, 8, 9, 10};
const byte TEAM_A = 1;
const byte TEAM_B = 2;

// ---------------- HARDWARE PINS ----------------
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const byte BUZZER = 6;
const byte JOYSTICK_X = A3;
const byte JOYSTICK_Y = A2;
const byte JOYSTICK_BUTTON = A1;
const byte IR_PIN = A0;
const byte RED_PINS[3] = {8, 9, 10};
const byte GREEN_PIN = LED_BUILTIN;

// ---------------- GAME PHASES ----------------
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
enum GamePhase : byte {
    WAIT_START, LEVEL_START, ENTER_XY, SIGNAL_XY, GUESS_XY,
    GUESS_RESULT, EVADE_COUNTDOWN, PROJECTILE_WARNING,
    PROJECTILE_MOVING, PROJECTILE_RESULT, ATTACK_RESULT,
    LEVEL_RESULT, TURN_RESULT, MATCH_RESULT
};

// ---------------- GAME CONSTANTS ----------------
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const byte MAX_HEALTH = 100;
const byte DAMAGE = 25;
const byte TORPEDOES_PER_LEVEL = 3;
const byte PROJECTILES_PER_ATTACK = 3;
const byte MAX_TURNS = 4;
const byte SUB_WIDTH = 4;
const byte TORP_WIDTH = 2;

// Torpids Speed, make them faster here 
// GAME SPEED 
//////////////////////////////////////////////////////////////
const unsigned long PROJECTILE_STEP_MS[3] = {299, 199, 99};
//////////////////////////////////////////////////////////////
const unsigned long PROJECTILE_WARNING_MS[3] = {600, 450, 350};
const unsigned long BEEP_ON_MS[3] = {150, 120, 100};
const unsigned long BEEP_GAP_MS[3] = {300, 200, 130};
const unsigned long I2C_POLL_MS = 35;
const unsigned long I2C_BROADCAST_MS = 70;
const unsigned long LCD_REFRESH_MS = 90;

// ---------------- I2C MESSAGE TYPES ----------------
const byte MSG_STATE = 0xA5;
const byte EVENT_NONE = 0;
const byte EVENT_START = 1;
const byte EVENT_UP = 2;
const byte EVENT_DOWN = 3;
const byte EVENT_DIGIT = 4;

// ---------------- GAME SNAPSHOT ----------------
// 19 bytes, within the 32-byte Wire buffer
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
struct GameSnapshot {
    byte marker;   // What message
    byte phase;     // Which phase of the game 
    byte turn;      // who's turn is it 
    byte level;     // which lvl are we on 
    byte ammo;      //
    byte health;
    byte scientistTeam;
    byte x;
    byte y;
    byte guessX;
    byte guessY;
    byte projectile;
    byte torpColumn;
    byte torpLane;
    byte subLane;
    byte outcome;
    byte scoreA;
    byte scoreB;
    byte sequence;
};

// ---------------- SHARED GAME STATE ----------------
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
GameSnapshot game = {MSG_STATE, WAIT_START, 1, 0, 3, 100, TEAM_A, 
                                    255, 255, 255, 255, 0, 15, 0, 1, 0, 0, 0, 0};
//--------------------------------------------------------

unsigned long phaseStarted = 0;
unsigned long lastMovement = 0;
unsigned long lastPoll = 0;
unsigned long lastBroadcast = 0;
unsigned long lastLCD = 0;
byte inputDigits = 0;
byte pendingX = 255;
byte pendingY = 255;
byte lastOutcome = 0;   // 1 = evaded, 2 = hit, 3 = decoded
byte lastJoystickDirection = 0;
byte previousButton = HIGH;
unsigned long lastButtonChange = 0;
byte previousPhase = 255;
byte previousSequence = 255;

// ---------------- I2C SLAVE EVENT QUEUE ----------------
// Callbacks do not touch LCDs, Serial, or game logic.
volatile byte queuedType = EVENT_NONE;
volatile byte queuedValue = 0;
volatile byte receivedSnapshot[sizeof(GameSnapshot)];
volatile bool snapshotReady = false;

// ---------------- LCD GRAPHICS CHARACTERS ----------------
byte submarineParts[4][8] = {
    {B00000,B00000,B00001,B00111,B11111,B11111,B01111,B00000},
    {B00000,B00000,B00100,B11111,B11111,B11111,B11111,B00000},
    {B00100,B00100,B11111,B11111,B11111,B11111,B11111,B00000},
    {B00000,B00000,B00000,B11100,B11111,B11111,B11100,B00000}
};
byte damagedParts[4][8] = {
    {B00000,B10001,B01010,B00100,B10010,B00101,B01000,B00010},
    {B00100,B10001,B00100,B01010,B10000,B00101,B00010,B01000},
    {B10000,B00101,B01010,B00100,B10010,B00001,B01000,B00100},
    {B00010,B01000,B00101,B10010,B00100,B01010,B10001,B00000}
};
// Torpedo has a pointed LEFT-facing nose and right-facing tail fins.
byte torpedoParts[2][8] = {
    {B00000,B00010,B01111,B11111,B01111,B00010,B00000,B00000},
    {B00000,B00100,B11111,B11111,B11111,B00100,B00000,B00000}
};
byte splashChar[8] = {B00100,B10101,B01110,B11111,B01110,B10101,B00100,B00000};
byte blankChar[8] = {0,0,0,0,0,0,0,0};
byte lcdCache[2][16];
bool damagedGraphics = false;

// ---------------- SOUND SEQUENCER ----------------
// Only boards 1 and 3 ever play sounds, one per team.
byte soundMode = 0; // 0 silent, 1 melody, 2 coordinate signal
byte soundIndex = 0;
byte soundStep = 0;
byte soundX = 0;
byte soundY = 0;
byte soundLevel = 0;
unsigned long soundNext = 0;
const unsigned int soundNotes[6][5] = {
    {660,880,1047,0,0},      // startup / stage
    {1047,1319,1568,0,0},    // success
    {220,165,110,0,0},       // defeat
    {180,120,90,0,0},        // impact
    {784,988,1175,1568,0},  // final win
    {440,440,0,0,0}          // warning
};
const byte soundLengths[6] = {3,3,3,3,4,2};

// ---------------- FUNCTIONS  ----------------
// ***************************************************
void receiveEvent(int count);
void requestEvent();
void handle_event(byte board, byte type, byte value);
void change_phase(byte nextPhase);
void handle_game();
void handle_IR();
void handle_joystick();
void handle_i2c();
void handle_LCD();
void handle_LEDs();
void handle_sound();
void handle_sound_events();
// ***************************************************

// ---------------- BOARD TEAM ----------------
byte team_of(byte id) {
    return id <= 2 ? TEAM_A : TEAM_B;
}

// ---------------- INVADER TEAM ----------------
byte invader_team() {
    return game.scientistTeam == TEAM_A ? TEAM_B : TEAM_A;
}

// ---------------- BUZZER OWNERSHIP ----------------
bool is_sound_board() {
    return BOARD_ID == 1 || BOARD_ID == 3;
}

// ---------------- SOUND FOR THIS TEAM ----------------
bool is_scientist_sound_board() {
    return is_sound_board() && team_of(BOARD_ID) == game.scientistTeam;
}

// ---------------- LOAD LCD GRAPHICS ----------------
void load_graphics(bool damaged) {
    damagedGraphics = damaged;
    for (byte i = 0; i < 4; i++) {
        lcd.createChar(i, damaged ? damagedParts[i] : submarineParts[i]);
    }
    lcd.createChar(4, torpedoParts[0]);
    lcd.createChar(5, torpedoParts[1]);
    lcd.createChar(6, splashChar);
    lcd.createChar(7, blankChar);
    memset(lcdCache, 255, sizeof(lcdCache));
}

// ---------------- START SOUND EFFECT ----------------
void start_melody(byte melody) {
    if (!is_sound_board()) return;
    noTone(BUZZER);
    soundMode = 1;
    soundIndex = 0;
    soundStep = melody;
    soundNext = millis();
}

// ---------------- START COORDINATE BEEPS ----------------
void start_coordinate_sound() {
    if (!is_scientist_sound_board()) return;
    noTone(BUZZER);
    soundMode = 2;
    soundIndex = 0;
    soundStep = 0;
    soundX = game.x;
    soundY = game.y;
    soundLevel = game.level;
    soundNext = millis();
}
// ---------------- UPDATE SOUND WITHOUT DELAY ----------------
void handle_sound() {
    if (!is_sound_board() || soundMode == 0) return;
    unsigned long now = millis();
    if ((long)(now - soundNext) < 0) return;

    if (soundMode == 1) {
        if (soundIndex >= soundLengths[soundStep] * 2) {
            noTone(BUZZER);
            soundMode = 0;
            return;
        }
        if ((soundIndex & 1) == 0) {
            tone(BUZZER, soundNotes[soundStep][soundIndex / 2]);
            soundNext = now + 125;
        } else {
            noTone(BUZZER);
            soundNext = now + 75;
        }
        soundIndex++;
        return;
    }
    // X, separator, Y. Zero is represented by one long tone.
    byte digit = soundStep == 0 ? soundX : soundY;
    byte count = digit == 0 ? 1 : digit;
    unsigned long onTime = digit == 0 ? 450 : BEEP_ON_MS[soundLevel];
    if (soundStep >= 2) {
        noTone(BUZZER);
        soundMode = 0;
    } else if (soundIndex < count * 2) {
        if ((soundIndex & 1) == 0) {
            tone(BUZZER, soundStep == 0 ? 850 : 1050);
            soundNext = now + onTime;
        } else {
            noTone(BUZZER);
            soundNext = now + BEEP_GAP_MS[soundLevel];
        }
        soundIndex++;
    } else {
        noTone(BUZZER);
        soundStep++;
        soundIndex = 0;
        soundNext = now + 750;
    }
}
// ---------------- SIGNAL DURATION ----------------
unsigned long signal_duration() {
    byte xCount = game.x == 0 ? 1 : game.x;
    byte yCount = game.y == 0 ? 1 : game.y;
    unsigned long xOn = game.x == 0 ? 450 : BEEP_ON_MS[game.level];
    unsigned long yOn = game.y == 0 ? 450 : BEEP_ON_MS[game.level];
    return xCount * (xOn + BEEP_GAP_MS[game.level]) +
           yCount * (yOn + BEEP_GAP_MS[game.level]) + 2000;
}

// ---------------- SETUP ----------------
void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2);
    load_graphics(false);
    pinMode(BUZZER, OUTPUT);
    pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);
    for (byte i = 0; i < 3; i++) pinMode(RED_PINS[i], OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);
    randomSeed(analogRead(A2) ^ micros());

    if (BOARD_ID == 1) {
        Wire.begin();
    } else {
        Wire.begin(I2C_ADDRESS[BOARD_ID]);
        Wire.onReceive(receiveEvent);
        Wire.onRequest(requestEvent);
    }
    phaseStarted = millis();
    Serial.print(F("Deep Blue Dive board "));
    Serial.println(BOARD_ID);
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ---------------- MAIN LOOP ----------------
void loop() {
    handle_IR();
    handle_joystick();
    handle_i2c();
    if (BOARD_ID == 1) handle_game();
    handle_sound_events();
    handle_sound();
    handle_LEDs();
    handle_LCD();
}

// ---------------- IR COMMAND TO DIGIT ----------------

int commandToDigit(uint16_t command) {
    switch (command) {
        case 22: return 0;
        case 12: return 1;
        case 24: return 2;
        case 94: return 3;
        case 8:  return 4;
        case 28: return 5;
        case 90: return 6;
        case 66: return 7;
        case 82: return 8;
        case 74: return 9;
        default: return -1;
    }
}

// ---------------- READ IR REMOTE ----------------
void handle_IR() {
    if (!IrReceiver.decode()) return;
    if (IrReceiver.decodedIRData.protocol != UNKNOWN &&
        !(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
        int digit = commandToDigit(IrReceiver.decodedIRData.command);
        bool allowed = (game.phase == ENTER_XY && team_of(BOARD_ID) != game.scientistTeam) ||
                       (game.phase == GUESS_XY && team_of(BOARD_ID) == game.scientistTeam);
        if (digit >= 0 && allowed) {
            if (BOARD_ID == 1) handle_event(BOARD_ID, EVENT_DIGIT, digit);
            else {
                noInterrupts();
                queuedType = EVENT_DIGIT;
                queuedValue = digit;
                interrupts();
            }
        }
    }
    IrReceiver.resume();
}

// ---------------- READ JOYSTICK AND START BUTTON ----------------
void handle_joystick() {
    static unsigned long lastSample = 0;
    if (millis() - lastSample < 35) return;
    lastSample = millis();

    byte button = digitalRead(JOYSTICK_BUTTON);
    if (button != previousButton && millis() - lastButtonChange > 80) {
        previousButton = button;
        lastButtonChange = millis();
        if (button == LOW && (game.phase == WAIT_START || game.phase == MATCH_RESULT)) {
            if (BOARD_ID == 1) handle_event(BOARD_ID, EVENT_START, 0);
            else {
                noInterrupts();
                queuedType = EVENT_START;
                queuedValue = 0;
                interrupts();
            }
        }
    }

    int x = analogRead(JOYSTICK_X);
    byte direction = x > 650 ? EVENT_UP : (x < 350 ? EVENT_DOWN : EVENT_NONE);
    if (direction == lastJoystickDirection) return;
    lastJoystickDirection = direction;
    if (direction == EVENT_NONE || game.phase != PROJECTILE_MOVING ||
        team_of(BOARD_ID) != game.scientistTeam) return;
    if (BOARD_ID == 1) handle_event(BOARD_ID, direction, 0);
    else {
        noInterrupts();
        queuedType = direction;
        queuedValue = 0;
        interrupts();
    }
}

// ---------------- RECEIVE I2C SNAPSHOT (SLAVE CALLBACK) ----------------
void receiveEvent(int count) {
    if (count != (int)sizeof(GameSnapshot)) {
        while (Wire.available()) Wire.read();
        return;
    }
    for (byte i = 0; i < sizeof(GameSnapshot); i++) {
        receivedSnapshot[i] = Wire.read();
    }
    if (receivedSnapshot[0] == MSG_STATE) snapshotReady = true;
}

// ---------------- RETURN LOCAL INPUT EVENT (SLAVE CALLBACK) ----------------
void requestEvent() {
    byte packet[2] = {queuedType, queuedValue};
    Wire.write(packet, 2);
    queuedType = EVENT_NONE;
}

// ---------------- APPLY RECEIVED GAME SNAPSHOT ----------------
void apply_snapshot() {
    if (!snapshotReady) return;
    byte buffer[sizeof(GameSnapshot)];
    noInterrupts();
    for (byte i = 0; i < sizeof(GameSnapshot); i++) buffer[i] = receivedSnapshot[i];
    snapshotReady = false;
    interrupts();
    if (buffer[0] != MSG_STATE) return;
    if (buffer[18] != game.sequence) phaseStarted = millis();
    memcpy(&game, buffer, sizeof(GameSnapshot));
}

// ---------------- COORDINATE FOUR BOARDS ----------------
void handle_i2c() {
    if (BOARD_ID != 1) {
        apply_snapshot();
        return;
    }
    unsigned long now = millis();
    if (now - lastPoll >= I2C_POLL_MS) {
        lastPoll = now;
        for (byte id = 2; id <= 4; id++) {
            byte count = Wire.requestFrom(I2C_ADDRESS[id], (byte)2);
            if (count == 2) {
                byte type = Wire.read();
                byte value = Wire.read();
                if (type != EVENT_NONE) handle_event(id, type, value);
            } else {
                while (Wire.available()) Wire.read();
            }
        }
    }
    if (now - lastBroadcast >= I2C_BROADCAST_MS) {
        lastBroadcast = now;
        for (byte id = 2; id <= 4; id++) {
            Wire.beginTransmission(I2C_ADDRESS[id]);
            Wire.write((const uint8_t*)&game, sizeof(GameSnapshot));
            byte error = Wire.endTransmission();
            if (error != 0) {
                Serial.print(F("I2C error board "));
                Serial.println(id);
            }
        }
    }
}

// ---------------- ENTER GAME PHASE ----------------
void change_phase(byte nextPhase) {
    game.phase = nextPhase;
    game.sequence++;
    phaseStarted = millis();
}

// ---------------- SETUP NEW TURN ----------------
//*************************************************************
//------------------- INITIALIZATION ----------------------
void begin_turn() {
    game.scientistTeam = (game.turn & 1) ? TEAM_A : TEAM_B;
    game.level = 0;
    game.ammo = 3;
    game.health = 100;
    game.subLane = 1;
    game.x = game.y = game.guessX = game.guessY = 255;
    inputDigits = 0;
    change_phase(WAIT_START);
}

// ---------------- SETUP NEW DIFFICULTY ----------------
void begin_level() {
    game.ammo = 3;
    change_phase(LEVEL_START);
}

// ---------------- SETUP NEXT MAIN TORPEDO ----------------
void begin_attack() {
    game.x = game.y = game.guessX = game.guessY = 255;
    inputDigits = 0;
    change_phase(ENTER_XY);
}

// ---------------- START PROJECTILE ----------------
void begin_projectile() {
    game.torpColumn = 14;
    game.torpLane = game.projectile == 0 ? game.subLane : random(0, 2);
    change_phase(PROJECTILE_WARNING);
}

// ---------------- SCORE A FINISHED TURN ----------------
void finish_turn() {
    // Quarter-points: 100% = 4, 75% = 3, ... 0% = 0.
    byte scientistPoints = game.health / 25;
    byte invaderPoints = 4 - scientistPoints;
    if (game.scientistTeam == TEAM_A) {
        game.scoreA += scientistPoints;
        game.scoreB += invaderPoints;
    } else {
        game.scoreB += scientistPoints;
        game.scoreA += invaderPoints;
    }
    change_phase(TURN_RESULT);
}

// ---------------- HANDLE PLAYER INPUTS ON COORDINATOR ----------------
void handle_event(byte board, byte type, byte value) {
    if (BOARD_ID != 1) return;
    byte team = team_of(board);
    if (type == EVENT_START) {
        if (game.phase == WAIT_START && team == game.scientistTeam) begin_level();
        else if (game.phase == MATCH_RESULT) {
            game.turn = 1;
            game.scoreA = game.scoreB = 0;
            begin_turn();
        }
        return;
    }
    if ((type == EVENT_UP || type == EVENT_DOWN) &&
        team == game.scientistTeam && game.phase == PROJECTILE_MOVING) {
        game.subLane = type == EVENT_UP ? 0 : 1;
        return;
    }
    if (type != EVENT_DIGIT) return;
    byte maxDigit = game.level == 0 ? 5 : (game.level == 1 ? 7 : 9);
    if (value > maxDigit) return;

    if (game.phase == ENTER_XY && team == invader_team()) {
        if (inputDigits == 0) {
            game.x = value;
            inputDigits = 1;
        } else {
            game.y = value;
            inputDigits = 0;
            change_phase(SIGNAL_XY);
        }
    } else if (game.phase == GUESS_XY && team == game.scientistTeam) {
        if (inputDigits == 0) {
            game.guessX = value;
            inputDigits = 1;
        } else {
            game.guessY = value;
            inputDigits = 0;
            game.outcome = (game.guessX == game.x && game.guessY == game.y) ? 3 : 0;
            change_phase(GUESS_RESULT);
        }
    }
}

// ---------------- ADVANCE GAME AUTOMATICALLY ----------------
// ****************************************************************
// GAME LOGIC 

void handle_game() {
    unsigned long elapsed = millis() - phaseStarted;
    switch (game.phase) {

        case WAIT_START:
        case ENTER_XY:
        case GUESS_XY:
        case MATCH_RESULT:
            break;
        
        case LEVEL_START:
            if (elapsed >=2500) begin_attack();
            break;
        
        case SIGNAL_XY:
            if (elapsed >= signal_duration()) change_phase(GUESS_XY);
            break;
        
        case GUESS_RESULT:
            if (elapsed >= 1700) {
                if (game.outcome == 3) change_phase(ATTACK_RESULT);
                else change_phase(EVADE_COUNTDOWN);
            }
            break;
        
        case EVADE_COUNTDOWN:
            if (elapsed >=3000) {
                game.projectile = 0;
                begin_projectile();
            }
            break;
        case PROJECTILE_WARNING:
            if (elapsed >=PROJECTILE_WARNING_MS[game.level]) {
                lastMovement = millis();
                change_phase(PROJECTILE_MOVING);
            }
            break;
        case PROJECTILE_MOVING:
            if (millis() - lastMovement >= PROJECTILE_STEP_MS[game.level]) {
                lastMovement = millis();
                if (game.torpColumn > SUB_WIDTH)  game.torpColumn--;
                else {
                    game.outcome = (game.subLane ==game.torpLane) ? 2 : 1;
                    if (game.outcome == 2) {
                        game.health = game.health >= DAMAGE ? game.health - DAMAGE : 0;
                    }
                    change_phase(PROJECTILE_RESULT);
                }
            }
            break;
        case PROJECTILE_RESULT:
            // First 1 second shows damaged submarine / evasion animation.
            // The following 1.2 seconds show the explicit result message.

            if (elapsed >= 2200) {
                if (game.health == 0) finish_turn();
                else if (game.projectile + 1 < PROJECTILES_PER_ATTACK) {
                    game.projectile++;
                    begin_projectile();
                } else change_phase(ATTACK_RESULT);
            }
            break;
        case ATTACK_RESULT:
            if (elapsed >= 1800) {
                if (game.ammo > 0) game.ammo--;
                if (game.ammo > 0) begin_attack();
                else change_phase(LEVEL_RESULT);
            }
            break;
        case LEVEL_RESULT:
            if (elapsed >= 2600) {
                if (game.level < 2) {
                    game.level++;
                    begin_level();
                } else finish_turn();
            }
            break;
        case TURN_RESULT:
            if (elapsed >= 4000) {
                if (game.turn < MAX_TURNS) {
                    game.turn++;
                    begin_turn();
                } else change_phase(MATCH_RESULT);
            }
            break;
    }
}

// ---------------- LCD TEXT HELPERS ----------------
void put_text(byte row, byte col, const char* text, byte screen[2][16]) {
    while (*text && col < 16) screen[row][col++] = (byte)*text++;
}

// ---------------- LCD NUMBER HELPER ----------------
void put_number(byte row, byte col, int value, byte screen[2][16]) {
    char tmp[8];
    if (value == 255) strcpy(tmp, "_");
    else snprintf(tmp, sizeof(tmp), "%d", value);
    put_text(row, col, tmp, screen);
}

// ---------------- LCD SCORE HELPER ----------------
void put_score(byte row, byte col, byte quarters, byte screen[2][16]) {
    char tmp[8];
    snprintf(tmp, sizeof(tmp), "%d.%02d", quarters / 4, (quarters % 4) * 25);
    put_text(row, col, tmp, screen);
}

// ---------------- RENDER LCD SCREEN ----------------
void handle_LCD() {
    if (millis() - lastLCD < LCD_REFRESH_MS) return;
    lastLCD = millis();
    bool scientists = team_of(BOARD_ID) == game.scientistTeam;
    byte screen[2][16];
    memset(screen, ' ', sizeof(screen));
    char tmp[17];
    bool showDamage = game.phase == PROJECTILE_RESULT &&
                      game.outcome == 2 && millis() - phaseStarted < 1000;
    if (showDamage != damagedGraphics) load_graphics(showDamage);

    switch (game.phase) {
        case WAIT_START:
            put_text(0, 0, scientists ? "SCIENTISTS" : "INVADERS", screen);
            put_text(0, 12, game.scientistTeam == TEAM_A ? (scientists ? "A" : "B") : (scientists ? "B" : "A"), screen);
            put_text(1, 0, scientists ? "PRESS TO START" : "WAIT FOR SCIENCE", screen);
            break;
        case LEVEL_START:
            put_text(0, 0, game.level == 0 ? "EASY STARTING" :
                            (game.level == 1 ? "MEDIUM STARTING" : "HARD STARTING"), screen);
            snprintf(tmp, sizeof(tmp), "SUB HEALTH: %u%%", game.health);
            put_text(1, 0, tmp, screen);
            break;
        case ENTER_XY:
            if (scientists) {
                put_text(0, 0, "WAIT FOR INVADERS", screen);
                snprintf(tmp, sizeof(tmp), "HP:%u%%  TORP:%u/3", game.health, 4 - game.ammo);
                put_text(1, 0, tmp, screen);
            } else {
                put_text(0, 0, game.level == 0 ? "CHOOSE X,Y: 0-5" :
                                (game.level == 1 ? "CHOOSE X,Y: 0-7" : "CHOOSE X,Y: 0-9"), screen);
                put_text(1, 0, "X:", screen);
                put_number(1, 2, game.x, screen);
                put_text(1, 5, "Y:", screen);
                put_number(1, 7, game.y, screen);
                put_text(1, 11, "IR", screen);
            }
            break;
        case SIGNAL_XY:
            put_text(0, 0, scientists ? "INCOMING SIGNAL" : "TORPEDO LAUNCHED", screen);
            put_text(1, 0, scientists ? "LISTEN: X THEN Y" : "ENEMY DECODING", screen);
            break;
        case GUESS_XY:
            if (scientists) {
                put_text(0, 0, "DECODE X,Y", screen);
                put_text(1, 0, "X:", screen);
                put_number(1, 2, game.guessX, screen);
                put_text(1, 5, "Y:", screen);
                put_number(1, 7, game.guessY, screen);
            } else {
                put_text(0, 0, "SCIENTISTS GUESS", screen);
                put_text(1, 0, "WAIT FOR RESULT", screen);
            }
            break;
        case GUESS_RESULT:
            put_text(0, 0, game.outcome == 3 ? "CODE DECODED!" : "WRONG CODE!", screen);
            put_text(1, 0, game.outcome == 3 ? "ATTACK STOPPED" : "PREPARE TO EVADE", screen);
            break;
        case EVADE_COUNTDOWN:
            put_text(0, 0, scientists ? "EVASION STARTS" : "ENEMY EVADING", screen);
            snprintf(tmp, sizeof(tmp), "READY IN %u", (unsigned)(3 - ((millis()-phaseStarted)/1000 > 2 ? 2 : (millis()-phaseStarted)/1000)));
            put_text(1, 0, tmp, screen);
            break;
        case PROJECTILE_WARNING:
            put_text(0, 0, scientists ? "INCOMING TORPEDO" : "ATTACK INCOMING", screen);
            snprintf(tmp, sizeof(tmp), "WAVE %u/3 HP:%u%%", game.projectile+1, game.health);
            put_text(1, 0, tmp, screen);
            break;
        case PROJECTILE_MOVING:
            if (scientists) {
                for (byte i = 0; i < 4; i++) screen[game.subLane][i] = i;
                if (game.torpColumn < 16) screen[game.torpLane][game.torpColumn] = 4;
                if (game.torpColumn + 1 < 16) screen[game.torpLane][game.torpColumn+1] = 5;
            } else {
                snprintf(tmp, sizeof(tmp), "ATTACK WAVE %u/3", game.projectile+1);
                put_text(0, 0, tmp, screen);
                snprintf(tmp, sizeof(tmp), "ENEMY HP: %u%%", game.health);
                put_text(1, 0, tmp, screen);
            }
            break;
        case PROJECTILE_RESULT:
            if (scientists && millis() - phaseStarted < 1000) {
                for (byte i = 0; i < 4; i++) screen[game.subLane][i] = i;
                if (game.outcome == 1) screen[1-game.subLane][3] = 6;
            } else {
                put_text(0, 0, game.outcome == 2 ?
                         (scientists ? "DIRECT HIT!" : "TARGET HIT!") :
                         (scientists ? "TORPEDO EVADED!" : "ATTACK MISSED!"), screen);
                snprintf(tmp, sizeof(tmp), "%s%u%%", scientists ? "HEALTH: " : "ENEMY HP: ", game.health);
                put_text(1, 0, tmp, screen);
            }
            break;
        case ATTACK_RESULT:
            put_text(0, 0, game.outcome == 3 ? "CODE INTERCEPTED" : "ATTACK COMPLETE", screen);
            snprintf(tmp, sizeof(tmp), "HP:%u%%  LEFT:%u", game.health, game.ammo-1);
            put_text(1, 0, tmp, screen);
            break;
        case LEVEL_RESULT:
            put_text(0, 0, game.level == 0 ? "EASY COMPLETE!" :
                            (game.level == 1 ? "MEDIUM COMPLETE!" : "HARD COMPLETE!"), screen);
            snprintf(tmp, sizeof(tmp), "SUB HEALTH: %u%%", game.health);
            put_text(1, 0, tmp, screen);
            break;
        case TURN_RESULT:
            put_text(0, 0, game.health == 0 ? "SUB DESTROYED!" : "MISSION SURVIVED", screen);
            put_text(1, 0, "A:", screen);
            put_score(1, 2, game.scoreA, screen);
            put_text(1, 8, "B:", screen);
            put_score(1, 10, game.scoreB, screen);
            break;
        case MATCH_RESULT:
            put_text(0, 0, game.scoreA == game.scoreB ? "IT'S A DRAW!" :
                            (game.scoreA > game.scoreB ? "TEAM A WINS!" : "TEAM B WINS!"), screen);
            put_text(1, 0, "A:", screen);
            put_score(1, 2, game.scoreA, screen);
            put_text(1, 8, "B:", screen);
            put_score(1, 10, game.scoreB, screen);
            break;
    }

    for (byte row = 0; row < 2; row++) {
        for (byte col = 0; col < 16; col++) {
            if (screen[row][col] == lcdCache[row][col]) continue;
            lcd.setCursor(col, row);
            lcd.write(screen[row][col]);
            lcdCache[row][col] = screen[row][col];
        }
    }
}

// ---------------- UPDATE LED INDICATORS ----------------
void handle_LEDs() {
    bool scientists = team_of(BOARD_ID) == game.scientistTeam;
    bool warning = game.phase == SIGNAL_XY || game.phase == GUESS_XY ||
                   game.phase == EVADE_COUNTDOWN || game.phase == PROJECTILE_WARNING ||
                   game.phase == PROJECTILE_MOVING;
    bool blink = (millis() / (game.level == 0 ? 350 : (game.level == 1 ? 240 : 150))) & 1;
    if (scientists) {
        for (byte i = 0; i < 3; i++) digitalWrite(RED_PINS[i], warning && blink);
        bool safe = !warning && game.phase != PROJECTILE_RESULT && game.health > 0;
        digitalWrite(GREEN_PIN, safe);
    } else {
        for (byte i = 0; i < 3; i++) digitalWrite(RED_PINS[i], i < game.ammo);
        bool hitFlash = game.phase == PROJECTILE_RESULT && game.outcome == 2 &&
                        millis() - phaseStarted < 1200 && ((millis()/150)&1);
        digitalWrite(GREEN_PIN, hitFlash);
    }
}

// ---------------- TRIGGER SOUNDS ON STATE CHANGES ----------------
// Called from loop after game state has been synchronized.
void handle_sound_events() {
    if (game.phase == previousPhase && game.sequence == previousSequence) return;
    previousPhase = game.phase;
    previousSequence = game.sequence;
    bool scientists = team_of(BOARD_ID) == game.scientistTeam;
    if (!is_sound_board()) return;
    if (game.phase == LEVEL_START && scientists) start_melody(0);
    else if (game.phase  == SIGNAL_XY && scientists) start_coordinate_sound();
    else if (game.phase ==  GUESS_RESULT && scientists) start_melody(game.outcome == 3 ? 1 : 5);
    else if (game.phase == PROJECTILE_RESULT && scientists) start_melody(game.outcome == 2 ? 3 : 1);
    else if (game.phase ==  LEVEL_RESULT && scientists) start_melody(1);
    else if (game.phase == TURN_RESULT && scientists) start_melody(game.health == 0 ? 2 : 1);
    else if (game.phase ==  MATCH_RESULT &&
             (game.scoreA ==game.scoreB ? BOARD_ID == 1 :
              team_of(BOARD_ID) == (game.scoreA > game.scoreB ? TEAM_A : TEAM_B))) start_melody(4);
}
