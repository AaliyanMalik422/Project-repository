#ifndef SWITCHES_H
#define SWITCHES_H

extern int total_switches; 
// Check if a tile at (x,y) is a switch
bool isSwitchTile(int x, int y);

// Get the switch index at (x,y), or -1 if none
int getSwitchIndex(int x, int y);

// Toggle a switch between states A and B
void toggleSwitch(int switchIndex);

// Initialize all switches to default state
void initializeSwitches();

#endif