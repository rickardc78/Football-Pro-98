/* ��������������������������������������������������������������������������
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.

   lge_parser.cpp

   Implementation of parser for .lge (league) files used in Football Pro '98.
   Parses blockified data including league, conference, division, 
   team, and roster information.

   Created: 2024, Parser implementation for .lge files
�������������������������������������������������������������������������� */

#include "lge_parser.h"

/* ��������������������������������������������������������������������������
   Constructor and Destructor
�������������������������������������������������������������������������� */

LgeParser::LgeParser()
{
    fileReader = NULL;
    league = NULL;
    verbose = 0;
    memset(currentFile, 0, sizeof(currentFile));
}

LgeParser::~LgeParser()
{
    CloseFile();
}

/* ��������������������������������������������������������������������������
   File Operations
�������������������������������������������������������������������������� */

DYN_16 LgeParser::OpenFile(const DYN_8 *filename)
{
    if (!filename)
        return FALSE;
        
    CloseFile(); // Close any previously opened file
    
    fileReader = new LeagueReadBFile();
    if (!fileReader)
        return FALSE;
        
    if (!fileReader->Open(filename))
    {
        delete fileReader;
        fileReader = NULL;
        return FALSE;
    }
    
    strcpy(currentFile, filename);
    return TRUE;
}

void LgeParser::CloseFile()
{
    if (fileReader)
    {
        fileReader->Close();
        delete fileReader;
        fileReader = NULL;
    }
    
    if (league)
    {
        delete league;
        league = NULL;
    }
    
    memset(currentFile, 0, sizeof(currentFile));
}

DYN_16 LgeParser::IsFileOpen() const
{
    return (fileReader != NULL);
}

const DYN_8 *LgeParser::GetCurrentFile() const
{
    return currentFile;
}

void LgeParser::SetVerbose(DYN_16 verboseLevel)
{
    verbose = verboseLevel;
}

/* ��������������������������������������������������������������������������
   Main Parsing Method
�������������������������������������������������������������������������� */

DYN_16 LgeParser::ParseFile()
{
    if (!fileReader)
        return FALSE;
        
    // Load the league data using existing framework
    try
    {
        league = new League(currentFile, League::LOAD_ALL);
        if (!league)
            return FALSE;
    }
    catch (...)
    {
        if (verbose)
            printf("Error: Failed to load league from file: %s\n", currentFile);
        return FALSE;
    }
    
    return TRUE;
}

/* ��������������������������������������������������������������������������
   Output Methods
�������������������������������������������������������������������������� */

void LgeParser::OutputData(FILE *output, OutputFormat format)
{
    if (!output || !league)
        return;
        
    switch (format)
    {
        case FORMAT_JSON:
            OutputJson(output);
            break;
        case FORMAT_TEXT:
            OutputPlainText(output);
            break;
        default:
            OutputPlainText(output);
            break;
    }
}

void LgeParser::OutputData(const DYN_8 *filename, OutputFormat format)
{
    if (!filename)
        return;
        
    FILE *output = fopen(filename, "w");
    if (!output)
    {
        if (verbose)
            printf("Error: Cannot open output file: %s\n", filename);
        return;
    }
    
    OutputData(output, format);
    fclose(output);
}

/* ��������������������������������������������������������������������������
   JSON Output Implementation
�������������������������������������������������������������������������� */

void LgeParser::OutputJson(FILE *output)
{
    if (!output || !league)
        return;
        
    fprintf(output, "{\n");
    fprintf(output, "  \"file\": \"%s\",\n", currentFile);
    
    OutputLeagueJson(output);
    
    fprintf(output, "}\n");
}

void LgeParser::OutputLeagueJson(FILE *output)
{
    if (!output || !league)
        return;
        
    fprintf(output, "  \"league\": {\n");
    fprintf(output, "    \"name\": \"%s\",\n", league->GetName());
    fprintf(output, "    \"trophy\": \"%s\",\n", league->GetTrophy());
    fprintf(output, "    \"numSeasons\": %d,\n", league->GetNumSeasons());
    fprintf(output, "    \"inception\": %d,\n", league->GetInception());
    
    // Output conferences
    fprintf(output, "    \"conferences\": [\n");
    
    GenList<Team> teamList;
    league->BuildTeamList(&teamList);
    
    DYN_8 currentConf = 0;
    DYN_8 maxConf = 2; // Typical NFL structure
    
    for (DYN_8 confId = 0; confId < maxConf; confId++)
    {
        Conference *conf = league->GetConf(confId);
        if (conf)
        {
            if (confId > 0)
                fprintf(output, ",\n");
            OutputConferenceJson(output, conf);
        }
    }
    
    fprintf(output, "\n    ]\n");
    fprintf(output, "  }\n");
}

void LgeParser::OutputConferenceJson(FILE *output, Conference *conf)
{
    if (!output || !conf)
        return;
        
    fprintf(output, "      {\n");
    fprintf(output, "        \"name\": \"%s\",\n", conf->GetName());
    fprintf(output, "        \"id\": %d,\n", conf->GetId());
    
    // Output divisions
    fprintf(output, "        \"divisions\": [\n");
    
    DYN_8 maxDiv = 3; // Typical NFL structure
    for (DYN_8 divId = 0; divId < maxDiv; divId++)
    {
        Division *div = conf->GetDiv(divId);
        if (div)
        {
            if (divId > 0)
                fprintf(output, ",\n");
            OutputDivisionJson(output, div);
        }
    }
    
    fprintf(output, "\n        ]\n");
    fprintf(output, "      }");
}

void LgeParser::OutputDivisionJson(FILE *output, Division *div)
{
    if (!output || !div)
        return;
        
    fprintf(output, "          {\n");
    fprintf(output, "            \"name\": \"%s\",\n", div->GetName());
    fprintf(output, "            \"id\": %d,\n", div->GetId());
    
    // Output teams
    fprintf(output, "            \"teams\": [\n");
    
    DYN_8 maxTeam = 5; // Typical NFL structure
    for (DYN_8 teamId = 0; teamId < maxTeam; teamId++)
    {
        Team *team = div->GetTeam(teamId);
        if (team)
        {
            if (teamId > 0)
                fprintf(output, ",\n");
            OutputTeamJson(output, team);
        }
    }
    
    fprintf(output, "\n            ]\n");
    fprintf(output, "          }");
}

void LgeParser::OutputTeamJson(FILE *output, Team *team)
{
    if (!output || !team)
        return;
        
    fprintf(output, "              {\n");
    fprintf(output, "                \"name\": \"%s\",\n", team->GetName());
    fprintf(output, "                \"mascot\": \"%s\",\n", team->GetMascot());
    fprintf(output, "                \"abbreviation\": \"%s\",\n", team->GetAbbr());
    fprintf(output, "                \"stadium\": \"%s\",\n", team->GetStadiumName());
    fprintf(output, "                \"coach\": \"%s\",\n", team->GetCoach());
    fprintf(output, "                \"wins\": %d,\n", team->GetWins());
    fprintf(output, "                \"losses\": %d,\n", team->GetLosses());
    fprintf(output, "                \"ties\": %d,\n", team->GetTies());
    
    // Output roster information
    OutputRosterJson(output, team);
    
    fprintf(output, "              }");
}

void LgeParser::OutputRosterJson(FILE *output, Team *team)
{
    if (!output || !team)
        return;
        
    fprintf(output, "                \"roster\": {\n");
    
    Roster *roster = team->GetRPtr();
    if (roster)
    {
        fprintf(output, "                  \"players\": [\n");
        
        // Try to output roster information
        // Note: Without full access to roster internals, this is a placeholder
        fprintf(output, "                    {\"note\": \"Roster parsing requires deeper integration\"}\n");
        
        fprintf(output, "                  ]\n");
    }
    else
    {
        fprintf(output, "                  \"players\": []\n");
    }
    
    fprintf(output, "                }\n");
}

/* ��������������������������������������������������������������������������
   Plain Text Output Implementation
�������������������������������������������������������������������������� */

void LgeParser::OutputPlainText(FILE *output)
{
    if (!output || !league)
        return;
        
    fprintf(output, "===============================================\n");
    fprintf(output, "Football Pro '98 League File Parser\n");
    fprintf(output, "===============================================\n");
    fprintf(output, "File: %s\n\n", currentFile);
    
    OutputLeagueText(output);
}

void LgeParser::OutputLeagueText(FILE *output)
{
    if (!output || !league)
        return;
        
    fprintf(output, "LEAGUE INFORMATION:\n");
    fprintf(output, "  Name: %s\n", league->GetName());
    fprintf(output, "  Trophy: %s\n", league->GetTrophy());
    fprintf(output, "  Seasons: %d\n", league->GetNumSeasons());
    fprintf(output, "  Inception: %d\n", league->GetInception());
    fprintf(output, "\n");
    
    // Output conferences
    DYN_8 maxConf = 2;
    for (DYN_8 confId = 0; confId < maxConf; confId++)
    {
        Conference *conf = league->GetConf(confId);
        if (conf)
        {
            OutputConferenceText(output, conf);
        }
    }
}

void LgeParser::OutputConferenceText(FILE *output, Conference *conf)
{
    if (!output || !conf)
        return;
        
    fprintf(output, "CONFERENCE: %s (ID: %d)\n", conf->GetName(), conf->GetId());
    fprintf(output, "----------------------------------------\n");
    
    DYN_8 maxDiv = 3;
    for (DYN_8 divId = 0; divId < maxDiv; divId++)
    {
        Division *div = conf->GetDiv(divId);
        if (div)
        {
            OutputDivisionText(output, div);
        }
    }
    fprintf(output, "\n");
}

void LgeParser::OutputDivisionText(FILE *output, Division *div)
{
    if (!output || !div)
        return;
        
    fprintf(output, "  DIVISION: %s (ID: %d)\n", div->GetName(), div->GetId());
    
    DYN_8 maxTeam = 5;
    for (DYN_8 teamId = 0; teamId < maxTeam; teamId++)
    {
        Team *team = div->GetTeam(teamId);
        if (team)
        {
            OutputTeamText(output, team);
        }
    }
    fprintf(output, "\n");
}

void LgeParser::OutputTeamText(FILE *output, Team *team)
{
    if (!output || !team)
        return;
        
    fprintf(output, "    TEAM: %s %s (%s)\n", 
            team->GetName(), team->GetMascot(), team->GetAbbr());
    fprintf(output, "      Stadium: %s\n", team->GetStadiumName());
    fprintf(output, "      Coach: %s\n", team->GetCoach());
    fprintf(output, "      Record: %d-%d-%d\n", 
            team->GetWins(), team->GetLosses(), team->GetTies());
    
    OutputRosterText(output, team);
    fprintf(output, "\n");
}

void LgeParser::OutputRosterText(FILE *output, Team *team)
{
    if (!output || !team)
        return;
        
    fprintf(output, "      Roster: ");
    
    Roster *roster = team->GetRPtr();
    if (roster)
    {
        fprintf(output, "Available (detailed parsing requires deeper integration)\n");
    }
    else
    {
        fprintf(output, "Not loaded\n");
    }
}

/* ��������������������������������������������������������������������������
   Utility Functions
�������������������������������������������������������������������������� */

DYN_16 ParseLgeFile(const DYN_8 *inputFile, const DYN_8 *outputFile, LgeParser::OutputFormat format)
{
    if (!inputFile)
        return FALSE;
        
    LgeParser parser;
    
    if (!parser.OpenFile(inputFile))
    {
        printf("Error: Cannot open input file: %s\n", inputFile);
        return FALSE;
    }
    
    if (!parser.ParseFile())
    {
        printf("Error: Failed to parse file: %s\n", inputFile);
        return FALSE;
    }
    
    if (outputFile)
    {
        parser.OutputData(outputFile, format);
    }
    else
    {
        parser.OutputData(stdout, format);
    }
    
    return TRUE;
}

void PrintUsage()
{
    printf("Usage: lge_parser <input.lge> [output_file] [--json|--text]\n");
    printf("  input.lge    - Input .lge file to parse\n");
    printf("  output_file  - Optional output file (default: stdout)\n");
    printf("  --json       - Output in JSON format (default)\n");
    printf("  --text       - Output in plain text format\n");
    printf("\n");
    printf("Examples:\n");
    printf("  lge_parser NFLPI95.lge\n");
    printf("  lge_parser NFLPI95.lge output.json --json\n");
    printf("  lge_parser NFLPI95.lge output.txt --text\n");
}