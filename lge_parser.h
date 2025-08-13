/* ��������������������������������������������������������������������������
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.

   lge_parser.h

   Parser for .lge (league) files used in Football Pro '98.
   Parses blockified data including league, conference, division, 
   team, and roster information.

   Created: 2024, Parser implementation for .lge files
�������������������������������������������������������������������������� */

#ifndef _LGE_PARSER_H
#define _LGE_PARSER_H

#include <stdio.h>
#include <string.h>
#include "DYNDEFS.H"
#include "LGEFILE.H"
#include "LCDTUTIL.H"
#include "LEAGUE.H"
#include "CONFER.H"
#include "DIVISION.H"
#include "TEAM.H"
#include "ROSTER.H"

class LgeParser
{
private:
    // Data members
    LeagueReadBFile     *fileReader;
    League              *league;
    DYN_8               currentFile[256];
    DYN_16              verbose;

    // Private helper methods
    void                OutputJson(FILE *output);
    void                OutputPlainText(FILE *output);
    DYN_16              ParseLeagueBlock();
    DYN_16              ParseConferenceBlocks();
    DYN_16              ParseDivisionBlocks();
    DYN_16              ParseTeamBlocks();
    DYN_16              ParseRosterBlocks();
    
    void                OutputLeagueJson(FILE *output);
    void                OutputConferenceJson(FILE *output, Conference *conf);
    void                OutputDivisionJson(FILE *output, Division *div);
    void                OutputTeamJson(FILE *output, Team *team);
    void                OutputRosterJson(FILE *output, Team *team);
    
    void                OutputLeagueText(FILE *output);
    void                OutputConferenceText(FILE *output, Conference *conf);
    void                OutputDivisionText(FILE *output, Division *div);
    void                OutputTeamText(FILE *output, Team *team);
    void                OutputRosterText(FILE *output, Team *team);

public:
    enum OutputFormat
    {
        FORMAT_JSON,
        FORMAT_TEXT
    };

    // Constructor and destructor
                        LgeParser();
                        ~LgeParser();

    // Main interface methods
    DYN_16              OpenFile(const DYN_8 *filename);
    void                CloseFile();
    DYN_16              ParseFile();
    void                SetVerbose(DYN_16 verboseLevel);
    
    // Output methods
    void                OutputData(FILE *output, OutputFormat format);
    void                OutputData(const DYN_8 *filename, OutputFormat format);
    
    // Information methods
    const DYN_8         *GetCurrentFile() const;
    DYN_16              IsFileOpen() const;
};

// Utility functions for external use
extern DYN_16           ParseLgeFile(const DYN_8 *inputFile, 
                                    const DYN_8 *outputFile, 
                                    LgeParser::OutputFormat format);
extern void             PrintUsage();

#endif // _LGE_PARSER_H