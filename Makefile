# Makefile for LGE Parser
# Simple build system for the Football Pro '98 .lge file parser

# Compiler settings
CXX = g++
CXXFLAGS = -Wall -g -DLCDT=0 -I.

# Target executable
TARGET = lge_parser

# Source files for the parser
PARSER_SOURCES = lge_parser_main.cpp lge_parser.cpp

# Required source files from the original codebase
REQUIRED_SOURCES = LCDTUTIL.CPP LCDTGLOB.CPP

# Object files
PARSER_OBJECTS = $(PARSER_SOURCES:.cpp=.o)
REQUIRED_OBJECTS = $(REQUIRED_SOURCES:.CPP=.o)
ALL_OBJECTS = $(PARSER_OBJECTS) $(REQUIRED_OBJECTS)

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(ALL_OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(ALL_OBJECTS)

# Compile C++ files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile CPP files (original codebase)
%.o: %.CPP
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test with existing .lge files
test: $(TARGET)
	@echo "Testing with NFLPI95.lge..."
	./$(TARGET) NFLPI95.lge test_output.json --json
	@echo "Testing with plain text output..."
	./$(TARGET) NFLPI95.lge test_output.txt --text

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(ALL_OBJECTS) $(TARGET) test_output.json test_output.txt

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build the lge_parser executable"
	@echo "  test     - Build and test with sample files"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help message"

.PHONY: all test clean help