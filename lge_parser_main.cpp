/* ��������������������������������������������������������������������������
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.

   lge_parser_main.cpp

   Command-line interface for the .lge file parser.
   Provides a simple way to parse Football Pro '98 league files
   and output the data in JSON or plain text format.

   Created: 2024, CLI for .lge file parser
�������������������������������������������������������������������������� */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lge_parser.h"

int main(int argc, char *argv[])
{
    // Parse command line arguments
    if (argc < 2)
    {
        PrintUsage();
        return 1;
    }
    
    const char *inputFile = argv[1];
    const char *outputFile = NULL;
    LgeParser::OutputFormat format = LgeParser::FORMAT_JSON;
    
    // Process additional arguments
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "--json") == 0)
        {
            format = LgeParser::FORMAT_JSON;
        }
        else if (strcmp(argv[i], "--text") == 0)
        {
            format = LgeParser::FORMAT_TEXT;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            PrintUsage();
            return 0;
        }
        else if (argv[i][0] != '-')
        {
            // Assume it's an output file
            outputFile = argv[i];
        }
        else
        {
            printf("Unknown option: %s\n", argv[i]);
            PrintUsage();
            return 1;
        }
    }
    
    // Initialize the league system (required for the existing framework)
    League::Init();
    
    // Parse the file
    printf("Parsing .lge file: %s\n", inputFile);
    
    DYN_16 result = ParseLgeFile(inputFile, outputFile, format);
    
    if (result)
    {
        if (outputFile)
            printf("Output written to: %s\n", outputFile);
        else
            printf("\n(Output complete)\n");
    }
    else
    {
        printf("Failed to parse file.\n");
        League::DeInit();
        return 1;
    }
    
    // Cleanup
    League::DeInit();
    
    return 0;
}