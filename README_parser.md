# Football Pro '98 .lge File Parser

A parser for `.lge` (league) files used in Football Pro '98. This tool can extract and interpret league data including teams, conferences, divisions, and basic roster information from the blockified file format.

## Features

- **Blockified Data Parsing**: Reads the structured block format (L03:, C03:, D03:, T03:, R01:)
- **Multiple File Formats**: Handles both binary `.lge` files and C source files with hex data
- **Human-readable Output**: Provides both JSON and plain text output formats
- **League Structure**: Extracts league names, conference/division structure, and team information
- **Position Mappings**: Includes position text mappings from the original LCDTUTIL.CPP
- **Modular Design**: Separate functions for handling each block type

## Building

The parser is written in C++ and can be built with the included Makefile:

```bash
make -f Makefile.simple clean
make -f Makefile.simple all
```

This will create the `simple_lge_parser` executable.

## Usage

### Basic Usage

```bash
# Parse to stdout (JSON format by default)
./simple_lge_parser NFLPI95.lge

# Parse to file with JSON output
./simple_lge_parser NFLPI95.lge output.json --json

# Parse to file with plain text output
./simple_lge_parser NFLPI95.lge output.txt --text
```

### Command Line Options

- `input.lge` - Input .lge file to parse (required)
- `output_file` - Optional output file (default: stdout)
- `--json` - Output in JSON format (default)
- `--text` - Output in plain text format
- `--help` - Show usage information

### Examples

```bash
# Parse NFLPI95 league file to JSON
./simple_lge_parser NFLPI95.lge nfl95.json --json

# Parse NFLPI97 league file to text
./simple_lge_parser NFLPI97.lge nfl97.txt --text

# Parse to stdout for quick viewing
./simple_lge_parser NFLPI95.lge --text
```

## File Format

The `.lge` files use a blockified binary format with the following block types:

- **L03:** - League data (name, trophy, seasons, inception year)
- **C03:** - Conference data (name, ID)
- **D03:** - Division data (name, ID)
- **T03:** - Team data (name, mascot, abbreviation, stadium, coach, record)
- **R01:** - Roster data (player information)

### File Variants

The parser handles two file format variants:

1. **Binary .lge files** - Direct binary data
2. **C source .lge files** - Files containing hex data as C arrays (e.g., `unsigned char ucDataBlock[17329] = { 0x4C, 0x30, ... }`)

## Output Format

### JSON Output

```json
{
  "league": {
    "name": "NFLPI95",
    "trophy": "The Super Bowl",
    "numSeasons": 1,
    "inception": 1995
  },
  "conferences": [
    {
      "name": "American",
      "id": 1
    },
    {
      "name": "National", 
      "id": 2
    }
  ],
  "divisions": [
    {
      "name": "Eastern",
      "id": 1
    }
  ],
  "teams": [
    {
      "name": "Team Name",
      "mascot": "Mascot",
      "abbreviation": "TM",
      "stadium": "Stadium Name",
      "coach": "Coach Name",
      "record": {
        "wins": 0,
        "losses": 0,
        "ties": 0
      }
    }
  ]
}
```

### Plain Text Output

```
===============================================
Football Pro '98 League File Parser
===============================================

LEAGUE: NFLPI95
Trophy: The Super Bowl
Seasons: 1
Inception: 1995

CONFERENCES:
  1. American (ID: 1)
  2. National (ID: 2)

DIVISIONS:
  1. Eastern (ID: 1)
  2. Central (ID: 2)
  3. Western (ID: 3)

TEAMS:
  1. Team Name (TM)
      Stadium: Stadium Name, Coach: Coach Name
      Record: 0-0-0
```

## Testing

Run the included tests with:

```bash
make -f Makefile.simple test
```

This will test the parser with the included sample files and generate output files for verification.

## Limitations

- **Team Names**: Team name parsing is partially implemented due to complex binary structure
- **Roster Data**: Player roster parsing requires deeper integration with the original codebase
- **Records**: Win/loss records are not fully extracted from the binary format
- **Advanced Features**: Some advanced features like trade parsing require the full LCDTUTIL.CPP integration

## Technical Details

### Dependencies

- Standard C++ libraries (stdio.h, stdlib.h, string.h)
- No external dependencies required

### Position Mappings

The parser includes position text mappings compatible with the original LCDTUTIL.CPP:

```cpp
"QB", "FB", "HB", "TE", "WR", "C", "G", "T", 
"DE", "DT", "LB", "CB", "S", "K", "P", 
"RB", "REC", "OL", "DL", "DB"
```

### Binary Format Analysis

The parser identifies blocks by searching for 4-byte identifiers:
- `0x4C 0x30 0x33 0x3A` = "L03:" (League)
- `0x43 0x30 0x33 0x3A` = "C03:" (Conference)  
- `0x44 0x30 0x33 0x3A` = "D03:" (Division)
- `0x54 0x30 0x33 0x3A` = "T03:" (Team)
- `0x52 0x30 0x31 0x3A` = "R01:" (Roster)

## Contributing

This parser was created to provide utility for reading Football Pro '98 league files. The modular design allows for easy extension to parse additional data types or improve the existing parsing accuracy.

### Future Enhancements

- Improve team name and roster parsing accuracy
- Add support for player statistics and attributes
- Implement trade information parsing using LCDTUTIL functions
- Add support for schedule data parsing
- Improve error handling and validation

## License

This parser is provided as-is for educational and utility purposes, respecting the original Sierra On-Line copyright notice found in the source files.