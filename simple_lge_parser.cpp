/* ��������������������������������������������������������������������������
   Simple .lge file parser implementation for Football Pro '98
   
   This parser reads .lge files directly as binary data and extracts
   readable information without depending on the complex class hierarchy.
   
   Created: 2024
�������������������������������������������������������������������������� */

#include "simple_lge_parser.h"

// Position text mappings (from LCDTUTIL.CPP)
const char *position_names[] = {
    "QB",      // 0
    "FB", "HB", // 1-2
    "TE", "WR", // 3-4
    "C", "G", "T", // 5-7
    "DE", "DT", // 8-9
    "LB",      // 10
    "CB", "S", // 11-12
    "K",       // 13
    "P",       // 14
    "RB",      // 15
    "REC",     // 16
    "OL",      // 17
    "DL",      // 18
    "DB",      // 19
    NULL
};

// Constructor
SimpleLgeParser::SimpleLgeParser() {
    file = NULL;
    data = NULL;
    fileSize = 0;
    currentPos = 0;
    numConferences = 0;
    numDivisions = 0;
    numTeams = 0;
    memset(&league, 0, sizeof(league));
    memset(conferences, 0, sizeof(conferences));
    memset(divisions, 0, sizeof(divisions));
    memset(teams, 0, sizeof(teams));
}

// Destructor
SimpleLgeParser::~SimpleLgeParser() {
    CloseFile();
}

// Open and read the entire file into memory
int SimpleLgeParser::OpenFile(const char* filename) {
    CloseFile();
    
    file = fopen(filename, "r"); // Open as text first to check format
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return 0;
    }
    
    // Check if this is a C source file with hex data
    char firstLine[256];
    if (fgets(firstLine, sizeof(firstLine), file)) {
        if (strstr(firstLine, "unsigned char") && strstr(firstLine, "DataBlock")) {
            fclose(file);
            return ParseCSourceFile(filename);
        }
    }
    
    // If not C source, treat as binary file
    fclose(file);
    file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Cannot open file %s as binary\n", filename);
        return 0;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read entire file into memory
    data = (unsigned char*)malloc(fileSize);
    if (!data) {
        printf("Error: Cannot allocate memory for file\n");
        fclose(file);
        file = NULL;
        return 0;
    }
    
    if (fread(data, 1, fileSize, file) != (size_t)fileSize) {
        printf("Error: Cannot read file data\n");
        free(data);
        data = NULL;
        fclose(file);
        file = NULL;
        return 0;
    }
    
    return 1;
}

// Close file and cleanup
void SimpleLgeParser::CloseFile() {
    if (file) {
        fclose(file);
        file = NULL;
    }
    if (data) {
        free(data);
        data = NULL;
    }
    fileSize = 0;
    currentPos = 0;
}

// Helper to read 16-bit integer from data
int SimpleLgeParser::ReadInt16(long offset) {
    if (offset + 1 >= fileSize) return 0;
    return (data[offset] | (data[offset + 1] << 8));
}

// Helper to read 32-bit integer from data
int SimpleLgeParser::ReadInt32(long offset) {
    if (offset + 3 >= fileSize) return 0;
    return (data[offset] | (data[offset + 1] << 8) | 
            (data[offset + 2] << 16) | (data[offset + 3] << 24));
}

// Helper to read null-terminated string from data
char* SimpleLgeParser::ReadString(long offset, int maxLen) {
    static char buffer[256];
    int i;
    
    if (offset >= fileSize) return (char*)"";  
    
    for (i = 0; i < maxLen - 1 && offset + i < fileSize; i++) {
        buffer[i] = data[offset + i];
        if (buffer[i] == 0) break;
    }
    buffer[i] = 0;
    
    return buffer;
}

// Find a block by its identifier
long SimpleLgeParser::FindBlock(const char* blockId) {
    long pos = 0;
    int blockLen = strlen(blockId);
    
    while (pos < fileSize - blockLen) {
        if (memcmp(&data[pos], blockId, blockLen) == 0) {
            return pos;
        }
        pos++;
    }
    
    return -1;
}

// Parse league block (L03:)
void SimpleLgeParser::ParseLeagueBlock(long offset, LeagueInfo *info) {
    if (offset < 0 || offset + 100 >= fileSize) return;
    
    // Skip block header (4 bytes)
    offset += 4;
    
    // Read league data structure
    // This is based on analyzing the binary structure
    offset += 8; // Skip some header data
    
    // League name appears to be around offset +24 from block start
    strncpy(info->name, ReadString(offset + 20, sizeof(info->name)), sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = 0;
    
    // Trophy name appears to be around offset +48
    strncpy(info->trophy, ReadString(offset + 44, sizeof(info->trophy)), sizeof(info->trophy) - 1);
    info->trophy[sizeof(info->trophy) - 1] = 0;
    
    // These are estimates based on typical league data
    info->numSeasons = 1;
    info->inception = 1995;
}

// Parse conference block (C03:)
void SimpleLgeParser::ParseConferenceBlock(long offset, ConferenceInfo *info) {
    if (offset < 0 || offset + 50 >= fileSize) return;
    
    // Skip block header
    offset += 4;
    offset += 8; // Skip header data
    
    // Conference name
    strncpy(info->name, ReadString(offset + 8, sizeof(info->name)), sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = 0;
    
    // ID from the data
    info->id = data[offset + 4];
}

// Parse division block (D03:)
void SimpleLgeParser::ParseDivisionBlock(long offset, DivisionInfo *info) {
    if (offset < 0 || offset + 50 >= fileSize) return;
    
    // Skip block header
    offset += 4;
    offset += 8; // Skip header data
    
    // Division name
    strncpy(info->name, ReadString(offset + 8, sizeof(info->name)), sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = 0;
    
    // ID from the data
    info->id = data[offset + 4];
}

// Parse team block (T03:)
void SimpleLgeParser::ParseTeamBlock(long offset, TeamInfo *info) {
    if (offset < 0 || offset + 200 >= fileSize) return;
    
    // Skip block header
    offset += 4;
    offset += 8; // Skip header data
    
    // Team data structure is complex, this is a simplified extraction
    // Names are typically stored as null-terminated strings
    
    // Try to extract team name (rough approximation)
    char *nameStr = ReadString(offset + 20, 50);
    if (strlen(nameStr) > 0 && strlen(nameStr) < 30) {
        strncpy(info->name, nameStr, sizeof(info->name) - 1);
        info->name[sizeof(info->name) - 1] = 0;
    }
    
    // Default values for now
    strcpy(info->mascot, "Team");
    strcpy(info->abbreviation, "TM");
    strcpy(info->stadium, "Stadium");
    strcpy(info->coach, "Coach");
    info->wins = 0;
    info->losses = 0;
    info->ties = 0;
    info->id = numTeams;
}

// Main parsing function
int SimpleLgeParser::ParseFile() {
    if (!data) return 0;
    
    long offset;
    
    // Parse league block
    offset = FindBlock(BLOCK_LEAGUE);
    if (offset >= 0) {
        ParseLeagueBlock(offset, &league);
    } else {
        // Try alternative league block formats
        offset = FindBlock("L02:");
        if (offset >= 0) {
            ParseLeagueBlock(offset, &league);
        } else {
            strcpy(league.name, "Unknown League");
            strcpy(league.trophy, "Championship");
        }
    }
    
    // Parse conferences
    numConferences = 0;
    offset = 0;
    while ((offset = FindBlock(BLOCK_CONF)) >= 0 && numConferences < 8) {
        ParseConferenceBlock(offset, &conferences[numConferences]);
        numConferences++;
        
        // Remove this block from future searches by overwriting it
        if (offset + 4 < fileSize) {
            memset(&data[offset], 0, 4);
        }
    }
    
    // Parse divisions
    numDivisions = 0;
    offset = 0;
    while ((offset = FindBlock(BLOCK_DIV)) >= 0 && numDivisions < 16) {
        ParseDivisionBlock(offset, &divisions[numDivisions]);
        numDivisions++;
        
        // Remove this block from future searches
        if (offset + 4 < fileSize) {
            memset(&data[offset], 0, 4);
        }
    }
    
    // Parse teams
    numTeams = 0;
    offset = 0;
    while ((offset = FindBlock(BLOCK_TEAM)) >= 0 && numTeams < 32) {
        ParseTeamBlock(offset, &teams[numTeams]);
        numTeams++;
        
        // Remove this block from future searches
        if (offset + 4 < fileSize) {
            memset(&data[offset], 0, 4);
        }
    }
    
    return 1;
}

// Output in JSON format
void SimpleLgeParser::OutputJSON(FILE *output) {
    fprintf(output, "{\n");
    fprintf(output, "  \"league\": {\n");
    fprintf(output, "    \"name\": \"%s\",\n", league.name);
    fprintf(output, "    \"trophy\": \"%s\",\n", league.trophy);
    fprintf(output, "    \"numSeasons\": %d,\n", league.numSeasons);
    fprintf(output, "    \"inception\": %d\n", league.inception);
    fprintf(output, "  },\n");
    
    fprintf(output, "  \"conferences\": [\n");
    for (int i = 0; i < numConferences; i++) {
        if (i > 0) fprintf(output, ",\n");
        fprintf(output, "    {\n");
        fprintf(output, "      \"name\": \"%s\",\n", conferences[i].name);
        fprintf(output, "      \"id\": %d\n", conferences[i].id);
        fprintf(output, "    }");
    }
    fprintf(output, "\n  ],\n");
    
    fprintf(output, "  \"divisions\": [\n");
    for (int i = 0; i < numDivisions; i++) {
        if (i > 0) fprintf(output, ",\n");
        fprintf(output, "    {\n");
        fprintf(output, "      \"name\": \"%s\",\n", divisions[i].name);
        fprintf(output, "      \"id\": %d\n", divisions[i].id);
        fprintf(output, "    }");
    }
    fprintf(output, "\n  ],\n");
    
    fprintf(output, "  \"teams\": [\n");
    for (int i = 0; i < numTeams; i++) {
        if (i > 0) fprintf(output, ",\n");
        fprintf(output, "    {\n");
        fprintf(output, "      \"name\": \"%s\",\n", teams[i].name);
        fprintf(output, "      \"mascot\": \"%s\",\n", teams[i].mascot);
        fprintf(output, "      \"abbreviation\": \"%s\",\n", teams[i].abbreviation);
        fprintf(output, "      \"stadium\": \"%s\",\n", teams[i].stadium);
        fprintf(output, "      \"coach\": \"%s\",\n", teams[i].coach);
        fprintf(output, "      \"record\": {\n");
        fprintf(output, "        \"wins\": %d,\n", teams[i].wins);
        fprintf(output, "        \"losses\": %d,\n", teams[i].losses);
        fprintf(output, "        \"ties\": %d\n", teams[i].ties);
        fprintf(output, "      }\n");
        fprintf(output, "    }");
    }
    fprintf(output, "\n  ]\n");
    fprintf(output, "}\n");
}

// Output in plain text format
void SimpleLgeParser::OutputText(FILE *output) {
    fprintf(output, "===============================================\n");
    fprintf(output, "Football Pro '98 League File Parser\n");
    fprintf(output, "===============================================\n\n");
    
    fprintf(output, "LEAGUE: %s\n", league.name);
    fprintf(output, "Trophy: %s\n", league.trophy);
    fprintf(output, "Seasons: %d\n", league.numSeasons);
    fprintf(output, "Inception: %d\n\n", league.inception);
    
    if (numConferences > 0) {
        fprintf(output, "CONFERENCES:\n");
        for (int i = 0; i < numConferences; i++) {
            fprintf(output, "  %d. %s (ID: %d)\n", i + 1, conferences[i].name, conferences[i].id);
        }
        fprintf(output, "\n");
    }
    
    if (numDivisions > 0) {
        fprintf(output, "DIVISIONS:\n");
        for (int i = 0; i < numDivisions; i++) {
            fprintf(output, "  %d. %s (ID: %d)\n", i + 1, divisions[i].name, divisions[i].id);
        }
        fprintf(output, "\n");
    }
    
    if (numTeams > 0) {
        fprintf(output, "TEAMS:\n");
        for (int i = 0; i < numTeams; i++) {
            fprintf(output, "  %d. %s %s (%s)\n", i + 1, 
                    teams[i].name, teams[i].mascot, teams[i].abbreviation);
            fprintf(output, "      Stadium: %s, Coach: %s\n", 
                    teams[i].stadium, teams[i].coach);
            fprintf(output, "      Record: %d-%d-%d\n", 
                    teams[i].wins, teams[i].losses, teams[i].ties);
        }
    }
}

// Utility function for external use
int ParseLgeFileSimple(const char *inputFile, const char *outputFile, int useJSON) {
    SimpleLgeParser parser;
    
    if (!parser.OpenFile(inputFile)) {
        return 0;
    }
    
    if (!parser.ParseFile()) {
        printf("Error: Failed to parse file\n");
        return 0;
    }
    
    FILE *output = stdout;
    if (outputFile) {
        output = fopen(outputFile, "w");
        if (!output) {
            printf("Error: Cannot open output file: %s\n", outputFile);
            return 0;
        }
    }
    
    if (useJSON) {
        parser.OutputJSON(output);
    } else {
        parser.OutputText(output);
    }
    
    if (outputFile && output != stdout) {
        fclose(output);
    }
    
    return 1;
}

// Print usage information
void PrintUsageSimple() {
    printf("Usage: simple_lge_parser <input.lge> [output_file] [--json|--text]\n");
    printf("  input.lge    - Input .lge file to parse\n");
    printf("  output_file  - Optional output file (default: stdout)\n");
    printf("  --json       - Output in JSON format (default)\n");
    printf("  --text       - Output in plain text format\n");
    printf("\n");
    printf("Examples:\n");
    printf("  simple_lge_parser NFLPI95.lge\n");
    printf("  simple_lge_parser NFLPI95.lge output.json --json\n");
    printf("  simple_lge_parser NFLPI95.lge output.txt --text\n");
}

// Parse C source file with hex data
int SimpleLgeParser::ParseCSourceFile(const char* filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    
    // This method doesn't use the member 'file' pointer
    file = NULL; // Make sure member file is NULL
    
    // Estimate size - count hex values
    char line[1024];
    int estimatedSize = 0;
    
    while (fgets(line, sizeof(line), f)) {
        // Count 0x patterns in the line
        char *ptr = line;
        while ((ptr = strstr(ptr, "0x")) != NULL) {
            estimatedSize++;
            ptr += 2;
        }
    }
    
    if (estimatedSize == 0) {
        fclose(f);
        return 0;
    }
    
    // Allocate data buffer
    data = (unsigned char*)malloc(estimatedSize);
    if (!data) {
        fclose(f);
        return 0;
    }
    
    // Parse hex values
    rewind(f);
    fileSize = 0;
    
    while (fgets(line, sizeof(line), f)) {
        char *ptr = line;
        while ((ptr = strstr(ptr, "0x")) != NULL) {
            if (fileSize >= estimatedSize) break;
            
            unsigned int value;
            if (sscanf(ptr, "0x%x", &value) == 1) {
                data[fileSize++] = (unsigned char)value;
            }
            ptr += 2;
        }
    }
    
    fclose(f);
    return 1;
}