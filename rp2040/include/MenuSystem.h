#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <Arduino.h>
#include "config.h"

// Forward declarations
class DisplayManager;
class PIDController;
class SensorManager;
class RelayController;

enum MenuState {
    MENU_MAIN,
    MENU_PID_SETTINGS,
    MENU_SENSOR_SELECT,
    MENU_SENSOR_CALIBRATE,
    MENU_RELAY_SETTINGS,
    MENU_SYSTEM_INFO,
    MENU_SAVE_LOAD,
    MENU_EDIT_VALUE
};

enum EditMode {
    EDIT_NONE,
    EDIT_SETPOINT,
    EDIT_KP,
    EDIT_KI,
    EDIT_KD,
    EDIT_HYSTERESIS,
    EDIT_CAL_OFFSET,
    EDIT_CAL_SCALE,
    EDIT_CAL_REFERENCE
};

struct MenuItem {
    const char* label;
    MenuState nextState;
    bool isAction;
    void (*action)();
};

class MenuSystem {
private:
    DisplayManager* display;
    PIDController* pid;
    SensorManager* sensors;
    RelayController* relays;

    MenuState currentState;
    MenuState previousState;
    int selectedIndex;
    int scrollOffset;

    EditMode editMode;
    double editValue;
    double editStep;
    double editMin;
    double editMax;

    unsigned long lastActivity;
    unsigned long lastBlink;
    bool blinkState;

    static MenuSystem* instance;

public:
    MenuSystem();
    void begin(DisplayManager* disp, PIDController* p, SensorManager* s, RelayController* r);

    void update();
    void handleInput(int encoder, bool btnEnter, bool btnBack);

    // Navigation
    void navigate(int direction);
    void selectItem();
    void goBack();
    void resetTimeout();

    // Edit mode
    void startEdit(EditMode mode, double currentValue, double step, double min, double max);
    void updateEdit(int direction);
    void confirmEdit();
    void cancelEdit();

    // Menu rendering
    void renderCurrentMenu();

    // Getters
    MenuState getState() const { return currentState; }
    bool isEditing() const { return editMode != EDIT_NONE; }
    bool isTimeout() const;

private:
    void renderMainMenu();
    void renderPIDSettings();
    void renderSensorSelect();
    void renderSensorCalibrate();
    void renderRelaySettings();
    void renderSystemInfo();
    void renderSaveLoad();
    void renderEditValue();

    void drawMenuItem(int y, const char* label, const char* value, bool selected);
    void drawScrollIndicators(int totalItems, int visibleItems);

    // Menu actions
    static void actionTogglePID();
    static void actionResetPID();
    static void actionStartAutoCal();
    static void actionSaveConfig();
    static void actionLoadConfig();
    static void actionResetConfig();
    static void actionReboot();
};

#endif // MENU_SYSTEM_H
