/* ��������������������������������������������������������������������������
   Simple command-line interface for the .lge file parser
   
   This provides a basic way to parse Football Pro '98 league files
   and output the data in JSON or plain text format.
   
   Created: 2024
�������������������������������������������������������������������������� */

#include "simple_lge_parser.h"

int main(int argc, char *argv[]) {
    // Parse command line arguments
    if (argc < 2) {
        PrintUsageSimple();
        return 1;
    }
    
    const char *inputFile = argv[1];
    const char *outputFile = NULL;
    int useJSON = 1; // Default to JSON format
    
    // Process additional arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--json") == 0) {
            useJSON = 1;
        } else if (strcmp(argv[i], "--text") == 0) {
            useJSON = 0;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            PrintUsageSimple();
            return 0;
        } else if (argv[i][0] != '-') {
            // Assume it's an output file
            outputFile = argv[i];
        } else {
            printf("Unknown option: %s\n", argv[i]);
            PrintUsageSimple();
            return 1;
        }
    }
    
    // Parse the file
    printf("Parsing .lge file: %s\n", inputFile);
    
    int result = ParseLgeFileSimple(inputFile, outputFile, useJSON);
    
    if (result) {
        if (outputFile) {
            printf("Output written to: %s\n", outputFile);
        } else {
            printf("\n(Output complete)\n");
        }
    } else {
        printf("Failed to parse file.\n");
        return 1;
    }
    
    return 0;
}