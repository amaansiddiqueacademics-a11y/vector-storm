#pragma once

// ======================================================
// SECTOR THEMES & LEVEL DEFINITIONS
// ======================================================

struct SectorTheme
{
    const char* name;        // shown in the HUD and the sector banner
    const char* subtitle;    // one-line hint under the banner
    float scrollSpeedMult;   // how fast the ground scrolls in this sector
    int targetScore;         // score needed to spawn the sector boss
    int tempo;               // music tempo (BPM)
};

const int NUM_SECTORS = 5;
extern SectorTheme sectorThemes[NUM_SECTORS];

extern int currentSector;
extern int sectorAnnouncementTimer;

// Scrolling world (one cartoon planet per sector)
void initStars();
void updateStars();
void drawBackgroundAndStars();
