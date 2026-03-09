#ifndef UI_RENDER_H_
#define UI_RENDER_H_

#include "slot_ESC.h"

void displayStatusLine();
void displayRaceModeSimple(uint8_t selectedItem, bool isEditing);
void displayRaceMode(uint8_t selectedItem, bool isEditing);
void showScreensaver();
void printMainMenu(MenuState_enum currMenuState);

#endif  /* UI_RENDER_H_ */
