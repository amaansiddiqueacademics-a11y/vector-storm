#include "Core/SaveSystem.hpp"
#include "Core/Config.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>

namespace
{
    const char* SCORES_FILE   = "scores.dat";
    const char* SETTINGS_FILE = "settings.cfg";

    std::vector<ScoreEntry> s_board;

    std::string currentDateStr()
    {
        time_t now = time(NULL);
        char buf[32] = "";
        struct tm* tmInfo = localtime(&now);
        if (tmInfo) strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tmInfo);
        return std::string(buf);
    }

    float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
    int   clampi(int v, int lo, int hi)       { return v < lo ? lo : (v > hi ? hi : v); }

    // Pilot names are stored in a '|' separated file, so keep them simple.
    std::string cleanName(const std::string& in)
    {
        std::string out;
        for (size_t i = 0; i < in.size() && out.size() < 12; i++)
        {
            char c = in[i];
            if (c != '|' && c != '\n' && c != '\r') out += c;
        }
        return out.empty() ? std::string("PILOT") : out;
    }

    bool byScoreDesc(const ScoreEntry& a, const ScoreEntry& b) { return a.score > b.score; }
}

namespace SaveSystem
{

void init()
{
    loadSettings();
    loadScores();
}

void shutdown()
{
    saveSettings();
    saveScores();
}

void loadSettings()
{
    std::ifstream f(SETTINGS_FILE);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line))
    {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        float val = (float)atof(line.c_str() + eq + 1);

        if      (key == "masterVolume") masterVolume       = clampf(val, 0.0f, 1.0f);
        else if (key == "sfxVolume")    sfxVolume          = clampf(val, 0.0f, 1.0f);
        else if (key == "musicVolume")  musicVolume        = clampf(val, 0.0f, 1.0f);
        else if (key == "screenShake")  screenShakeSetting = clampi((int)val, 0, SHAKE_COUNT - 1);
        else if (key == "shipAccent")   shipAccentSetting  = clampi((int)val, 0, ACCENT_COUNT - 1);
    }
}

void saveSettings()
{
    std::ofstream f(SETTINGS_FILE);
    if (!f.is_open()) return;

    f << "# VECTOR STORM settings - auto-generated\n";
    f << "masterVolume=" << masterVolume       << "\n";
    f << "sfxVolume="    << sfxVolume          << "\n";
    f << "musicVolume="  << musicVolume        << "\n";
    f << "screenShake="  << screenShakeSetting << "\n";
    f << "shipAccent="   << shipAccentSetting  << "\n";
}

void loadScores()
{
    s_board.clear();
    std::ifstream f(SCORES_FILE);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        if (line.empty() || line[0] == '#') continue;

        // Format: NAME|SCORE|SECTOR|DATE  (a damaged line is skipped, never a crash)
        std::istringstream ss(line);
        ScoreEntry e;
        std::string scoreStr, sectorStr;
        if (std::getline(ss, e.pilotName, '|') &&
            std::getline(ss, scoreStr, '|') &&
            std::getline(ss, sectorStr, '|'))
        {
            std::getline(ss, e.date);
            e.pilotName = cleanName(e.pilotName);
            e.score  = atoi(scoreStr.c_str());
            e.sector = atoi(sectorStr.c_str());
            if (e.score > 0) s_board.push_back(e);
        }
    }

    std::stable_sort(s_board.begin(), s_board.end(), byScoreDesc);
    if ((int)s_board.size() > MAX_SCORES) s_board.resize(MAX_SCORES);
}

void saveScores()
{
    std::ofstream f(SCORES_FILE);
    if (!f.is_open()) return;

    f << "# VECTOR STORM high scores - auto-generated\n";
    for (size_t i = 0; i < s_board.size(); i++)
    {
        const ScoreEntry& e = s_board[i];
        f << e.pilotName << "|" << e.score << "|" << e.sector << "|" << e.date << "\n";
    }
}

int registerScore(int score, int sector, const std::string& name)
{
    if (score <= 0) return 0;

    ScoreEntry e;
    e.pilotName = cleanName(name);
    e.score     = score;
    e.sector    = sector;
    e.date      = currentDateStr();

    // Insert after any equal scores so older records keep their rank
    std::vector<ScoreEntry>::iterator it = s_board.begin();
    while (it != s_board.end() && it->score >= score) ++it;
    int rank = (int)(it - s_board.begin()) + 1;
    if (rank > MAX_SCORES) return 0;

    s_board.insert(it, e);
    if ((int)s_board.size() > MAX_SCORES) s_board.resize(MAX_SCORES);

    saveScores();   // persist immediately
    return rank;
}

const std::vector<ScoreEntry>& getLeaderboard()
{
    return s_board;
}

int getHighScore()
{
    return s_board.empty() ? 0 : s_board[0].score;
}

}
