/* ��������������������������������������������������������������������������
   Simple .lge file parser for Football Pro '98
   
   This parser reads .lge files directly as binary data and extracts
   readable information without depending on the complex class hierarchy.
   
   Created: 2024
�������������������������������������������������������������������������� */

#ifndef _SIMPLE_LGE_PARSER_H
#define _SIMPLE_LGE_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Position text mappings (declared in .cpp file)
extern const char *position_names[];

// Block identifiers
#define BLOCK_LEAGUE   "L03:"
#define BLOCK_CONF     "C03:"
#define BLOCK_DIV      "D03:"
#define BLOCK_TEAM     "T03:"
#define BLOCK_ROSTER   "R01:"

// Structure to hold parsed league data
typedef struct {
    char name[25];
    char trophy[25];
    int numSeasons;
    int inception;
} LeagueInfo;

typedef struct {
    char name[25];
    int id;
} ConferenceInfo;

typedef struct {
    char name[25];
    int id;
} DivisionInfo;

typedef struct {
    char name[17];
    char mascot[17];
    char abbreviation[5];
    char stadium[25];
    char coach[25];
    int wins;
    int losses;
    int ties;
    int id;
} TeamInfo;

// Main parser class
class SimpleLgeParser {
private:
    FILE *file;
    unsigned char *data;
    long fileSize;
    long currentPos;
    
    // Helper methods
    int ReadInt16(long offset);
    int ReadInt32(long offset);
    char* ReadString(long offset, int maxLen);
    long FindBlock(const char* blockId);
    void ParseLeagueBlock(long offset, LeagueInfo *info);
    void ParseConferenceBlock(long offset, ConferenceInfo *info);
    void ParseDivisionBlock(long offset, DivisionInfo *info);
    void ParseTeamBlock(long offset, TeamInfo *info);
    int ParseCSourceFile(const char* filename);
    
public:
    SimpleLgeParser();
    ~SimpleLgeParser();
    
    int OpenFile(const char* filename);
    void CloseFile();
    int ParseFile();
    void OutputJSON(FILE *output);
    void OutputText(FILE *output);
    
    // Data access
    LeagueInfo league;
    ConferenceInfo conferences[8];
    DivisionInfo divisions[16];
    TeamInfo teams[32];
    int numConferences;
    int numDivisions;
    int numTeams;
};

// Utility functions
int ParseLgeFileSimple(const char *inputFile, const char *outputFile, int useJSON);
void PrintUsageSimple();

#endif // _SIMPLE_LGE_PARSER_H