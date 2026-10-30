#pragma once

#include <string>
#include <vector>

// ======================================================
// SAVE SYSTEM
// Persistent high scores (top 10) and user settings.
//   scores.dat   - one score per line:  NAME|SCORE|SECTOR|DATE
//   settings.cfg - key=value lines (volumes, screen shake, ship colour)
// Both files are written next to the game (the working directory).
// Missing or damaged files are ignored and the defaults are used.
// ======================================================

struct ScoreEntry
{
    std::string pilotName;
    int         score;
    int         sector;   // 1-based sector reached
    std::string date;     // "YYYY-MM-DD HH:MM"
};

namespace SaveSystem
{
    // Lifecycle
    void init();       // load settings + scores (call once at start-up)
    void shutdown();   // save settings + scores (safe to call more than once)

    // Settings (reads/writes the globals declared in Config.hpp)
    void loadSettings();
    void saveSettings();

    // High scores
    void loadScores();
    void saveScores();

    /// Adds a finished run to the leaderboard (keeps the top 10, highest first)
    /// and saves immediately. Scores of 0 are ignored.
    /// Returns the rank (1-10) the score reached, or 0 if it did not place.
    int registerScore(int score, int sector, const std::string& name = "PILOT");

    const std::vector<ScoreEntry>& getLeaderboard();
    int getHighScore();

    const int MAX_SCORES = 10;
}
