#include "CSVParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

void CSVParser::peek(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return;
    }

    std::string header, first_row;
    if (std::getline(file, header) && std::getline(file, first_row)) {
        std::vector<std::string> col_names = parseLine(header);
        std::vector<std::string> values = parseLine(first_row);

        // Determine the actual number of fields to display to avoid trailing data issues
        size_t display_count = std::max(col_names.size(), values.size());

        std::cout << "\n--- Dataset Structure Peek ---\n";
        for (size_t i = 0; i < display_count; ++i) {
            // Get the header name or a placeholder if missing
            std::string header_name = (i < col_names.size()) ? col_names[i] : "Unknown";
            // Get the sample value or [EMPTY] if missing
            std::string sample_val = (i < values.size()) ? values[i] : "[EMPTY]";

            // Format with clear spacing and no "Col" prefix
            std::cout << std::left << std::setw(3) << i << " " 
                      << std::left << std::setw(20) << header_name 
                      << " : " << sample_val << "\n";
        }
        std::cout << "------------------------------\n\n";
    }
}

std::vector<std::string> CSVParser::parseLine(const std::string& line) {
    std::vector<std::string> result;
    std::string current_cell;
    bool in_quotes = false;
    
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        
        // Skip carriage return characters (\r) that often appear in Windows-saved CSVs
        if (c == '\r') continue;

        if (c == '\"') {
            in_quotes = !in_quotes; 
        } else if (c == ',' && !in_quotes) {
            result.push_back(current_cell);
            current_cell.clear();
        } else {
            current_cell += c;
        }
    }
    
    // Final cleanup of the last cell to remove any trailing newline or whitespace
    if (!current_cell.empty()) {
        size_t last = current_cell.find_last_not_of(" \n\r\t");
        if (last != std::string::npos) {
            current_cell = current_cell.substr(0, last + 1);
        }
    }
    
    result.push_back(current_cell);
    return result;
}

bool CSVParser::validateAndParse(const std::string& filename, const std::string& primary_col, std::vector<std::vector<std::string>>& valid_records, int& pk_index) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return false;
    }

    std::string line;
    if (!std::getline(file, line)) {
        std::cerr << "Error: File is empty.\n";
        return false;
    }

    std::vector<std::string> headers = parseLine(line);
    pk_index = -1;
    
    for (size_t i = 0; i < headers.size(); ++i) {
        if (headers[i] == primary_col) {
            pk_index = i;
            break;
        }
    }

    if (pk_index == -1) {
        std::cerr << "Error: Primary column '" << primary_col << "' not found in headers.\n";
        return false;
    }

    std::unordered_set<std::string> seen_keys;
    int row_number = 2; 
    int expected_cols = headers.size();

    while (std::getline(file, line)) {
        if (line.empty()) {
            row_number++;
            continue;
        }

        std::vector<std::string> record = parseLine(line);

        // Before: if (record.size() != expected_cols) {
        if (record.size() != static_cast<size_t>(expected_cols)) {
            std::cerr << "Warning: Anomalous formatting at row " << row_number 
                      << ". Expected " << expected_cols << " columns, found " << record.size() 
                      << ". Skipping row.\n";
            row_number++;
            continue;
        }

        std::string pk_value = record[pk_index];

        if (seen_keys.find(pk_value) != seen_keys.end()) {
            std::cerr << "Error: Duplicate primary key '" << pk_value 
                      << "' found at row " << row_number << ". Halting load.\n";
            return false;
        }

        seen_keys.insert(pk_value);
        valid_records.push_back(record);
        row_number++;
    }

    return true;
}