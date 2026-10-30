#pragma once
// ======================================================================
// CGLab - the Computer Graphics toolbox of VECTOR STORM.
// Every function here implements (by hand) a classic CG algorithm, so the
// graphics pipeline is visible in the game instead of hidden in a library.
// ======================================================================

// Runtime toggles (F1..F5)
extern bool cgLabPanel;   // F1  live CG Lab panel + radar + target lock
extern bool cgWireframe;  // F2  glPolygonMode(GL_LINE)
extern bool cgCRT;        // F3  scanlines post effect
extern bool cgDebug;      // F4  enemy flight paths + AABB hitboxes
extern bool cgRaster;     // F5  software rasterisation (Bresenham / midpoint)

extern int   cgWinW, cgWinH;  // window size in pixels (for the viewport transform)
extern float cgFlash;         // full-screen impact flash (0..1)
extern float cgFps;

// ---------- Scan conversion
void bresenhamLine(int x0, int y0, int x1, int y1);
void midpointCircle(int cx, int cy, int r);

// ---------- Clipping (returns false when the line is fully outside)
bool clipLineCohenSutherland(float& x0, float& y0, float& x1, float& y1,
                             float xmin, float ymin, float xmax, float ymax,
                             int* outcode0 = 0, int* outcode1 = 0);

// ---------- Lighting / blending
void drawGlow(float x, float y, float radius, float r, float g, float b, float a);
void addLightFlash(float x, float y, float radius, float r, float g, float b);
void updateLightFlashes();
void drawLightFlashes();

// ---------- Post-processing
void drawScanlines();
void drawScreenFlash();
void updateFps();
