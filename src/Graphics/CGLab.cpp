#include "Graphics/CGLab.hpp"
#include "Core/Config.hpp"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>

static const float PI = 3.14159265f;

bool cgLabPanel  = false;
bool cgWireframe = false;
bool cgCRT       = false;
bool cgDebug     = false;
bool cgRaster    = false;

int   cgWinW = WIDTH, cgWinH = HEIGHT;
float cgFlash = 0.0f;
float cgFps   = 60.0f;

// ======================================================================
// SCAN CONVERSION - integer-only line and circle algorithms
// ======================================================================

void bresenhamLine(int x0, int y0, int x1, int y1)
{
    int dx = std::abs(x1 - x0), dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;                       // decision variable
    glBegin(GL_POINTS);
    for (;;)
    {
        glVertex2i(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    glEnd();
}

void midpointCircle(int cx, int cy, int r)
{
    int x = 0, y = r, d = 1 - r;             // midpoint decision parameter
    glBegin(GL_POINTS);
    while (x <= y)
    {
        glVertex2i(cx + x, cy + y); glVertex2i(cx - x, cy + y);
        glVertex2i(cx + x, cy - y); glVertex2i(cx - x, cy - y);
        glVertex2i(cx + y, cy + x); glVertex2i(cx - y, cy + x);
        glVertex2i(cx + y, cy - x); glVertex2i(cx - y, cy - x);
        if (d < 0) d += 2 * x + 3;
        else { d += 2 * (x - y) + 5; y--; }
        x++;
    }
    glEnd();
}

// ======================================================================
// COHEN-SUTHERLAND LINE CLIPPING
// ======================================================================

static int outCode(float x, float y, float xmin, float ymin, float xmax, float ymax)
{
    int c = 0;
    if (x < xmin) c |= 1;       // LEFT
    else if (x > xmax) c |= 2;  // RIGHT
    if (y < ymin) c |= 4;       // BOTTOM
    else if (y > ymax) c |= 8;  // TOP
    return c;
}

bool clipLineCohenSutherland(float& x0, float& y0, float& x1, float& y1,
                             float xmin, float ymin, float xmax, float ymax,
                             int* oc0, int* oc1)
{
    int c0 = outCode(x0, y0, xmin, ymin, xmax, ymax);
    int c1 = outCode(x1, y1, xmin, ymin, xmax, ymax);
    if (oc0) *oc0 = c0;
    if (oc1) *oc1 = c1;

    for (int guard = 0; guard < 8; guard++)
    {
        if (!(c0 | c1)) return true;     // trivially inside
        if (c0 & c1) return false;       // trivially outside
        int co = c0 ? c0 : c1;
        float x = 0, y = 0;
        if (co & 8)      { x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0); y = ymax; }
        else if (co & 4) { x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0); y = ymin; }
        else if (co & 2) { y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0); x = xmax; }
        else             { y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0); x = xmin; }
        if (co == c0) { x0 = x; y0 = y; c0 = outCode(x0, y0, xmin, ymin, xmax, ymax); }
        else          { x1 = x; y1 = y; c1 = outCode(x1, y1, xmin, ymin, xmax, ymax); }
    }
    return false;
}

// ======================================================================
// LIGHTING & BLENDING
//   drawGlow: triangle fan whose centre vertex is bright and rim vertices are
//   transparent. OpenGL's Gouraud (smooth) shading interpolates the colour
//   across the fan = a radial light falloff, alpha-blended over the scene.
//   (drawScreenFlash below uses additive blending, GL_ONE, instead.)
// ======================================================================

void drawGlow(float x, float y, float radius, float r, float g, float b, float a)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_FAN);
        glColor4f(r, g, b, a);
        glVertex2f(x, y);
        glColor4f(r, g, b, 0.0f);
        for (int i = 0; i <= 14; i++)
        {
            float t = i * 2.0f * PI / 14.0f;
            glVertex2f(x + cosf(t) * radius, y + sinf(t) * radius);
        }
    glEnd();
}

// Short-lived point lights spawned by explosions
struct LightFlash { float x, y, radius, r, g, b; int life, maxLife; bool active; };
static const int MAX_LIGHTS = 24;
static LightFlash lights[MAX_LIGHTS];

void addLightFlash(float x, float y, float radius, float r, float g, float b)
{
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (!lights[i].active)
        {
            lights[i] = LightFlash{x, y, radius, r, g, b, 14, 14, true};
            return;
        }
    }
}

void updateLightFlashes()
{
    for (int i = 0; i < MAX_LIGHTS; i++)
        if (lights[i].active && --lights[i].life <= 0) lights[i].active = false;
}

void drawLightFlashes()
{
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (!lights[i].active) continue;
        float t = (float)lights[i].life / lights[i].maxLife;
        drawGlow(lights[i].x, lights[i].y, lights[i].radius * (1.6f - 0.6f * t),
                 lights[i].r, lights[i].g, lights[i].b, 0.55f * t);
    }
}

// ======================================================================
// POST-PROCESSING (2D overlays)
// ======================================================================

void drawScanlines()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.16f);
    glBegin(GL_LINES);
    for (int y = 0; y < HEIGHT; y += 3)
    {
        glVertex2f(0.0f, (float)y);
        glVertex2f((float)WIDTH, (float)y);
    }
    glEnd();
}

void drawScreenFlash()
{
    if (cgFlash < 0.01f) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0f, 0.75f, 0.45f, std::min(0.55f, cgFlash) * 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(WIDTH, 0); glVertex2f(WIDTH, HEIGHT); glVertex2f(0, HEIGHT);
    glEnd();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void updateFps()
{
    static int last = 0, frames = 0;
    frames++;
    int now = glutGet(GLUT_ELAPSED_TIME);
    if (now - last >= 500)
    {
        cgFps = frames * 1000.0f / (now - last);
        frames = 0;
        last = now;
    }
}
