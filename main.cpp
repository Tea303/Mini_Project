#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iomanip> // Required for formatting the table output
#include <fstream>

#include "BPlusTree.h"
#include "CSVParser.h"
#include "StorageManager.h"

void printHelp() {
    std::cout << "\nAvailable Commands:\n"
              << "  PEEK <filename.csv>                 - Display column indexes and sample values.\n"
              << "  UPLOAD <filename.csv> <PRIMARY_COL> - Build index and save to disk.\n"
              << "  FIND <csv_name> <key_value>         - Search and display structured record.\n"
              << "  HELP                                - Show command syntax.\n"
              << "  EXIT                                - Close the engine.\n\n";
}

int main() {
    std::string input_line;
    std::cout << "===============================================\n";
    std::cout << "        B+ Tree REPL Query Engine              \n";
    std::cout << "===============================================\n";
    std::cout << "Type HELP for syntax instructions.\n\n";

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input_line)) break;

        std::stringstream ss(input_line);
        std::string command;
        ss >> command;

        if (command.empty()) continue;

        if (command == "EXIT") {
            std::cout << "Exiting. Goodbye!\n";
            break;
        } 
        else if (command == "HELP") {
            printHelp();
        } 
        else if (command == "PEEK") {
            std::string filename;
            if (ss >> filename) {
                CSVParser::peek(filename);
            } else {
                std::cerr << "Usage Error: PEEK <filename.csv>\n";
            }
        } 
        else if (command == "UPLOAD") {
            std::string filename, primary_col;
            // First, extract the filename (one word)
            if (ss >> filename) {
                // Then, capture the rest of the line as the column name
                std::getline(ss >> std::ws, primary_col); 
                
                if (primary_col.empty()) {
                    std::cerr << "Usage Error: UPLOAD <filename.csv> <PRIMARY_COL>\n";
                    continue;
                }

                std::vector<std::vector<std::string>> valid_records;
                int pk_index = -1;

                if (CSVParser::validateAndParse(filename, primary_col, valid_records, pk_index)) {
                    BPlusTree tree(4);
                    
                    std::ifstream file(filename);
                    std::string header_line;
                    if (std::getline(file, header_line)) {
                        tree.setHeaders(CSVParser::parseLine(header_line));
                    }
                    file.close();

                    std::cout << "Indexing " << valid_records.size() << " records using [" << primary_col << "]...\n";
                    for (const auto& record : valid_records) {
                        tree.insert(record[pk_index], record); 
                    }
                    StorageManager::saveIndex(tree, filename);
                }
            }
        }
        else if (command == "FIND") {
            std::string csv_name, key_value;
            
            // 1. Get the dataset name (e.g., "amazon")
            if (!(ss >> csv_name)) {
                std::cerr << "Usage Error: FIND <csv_name> <key_value>\n";
                continue;
            }

            // 2. Capture EVERYTHING else on the line as the key
            std::getline(ss >> std::ws, key_value);

            // 3. Clean up the key (remove any trailing \r or \n)
            if (!key_value.empty()) {
                size_t last = key_value.find_last_not_of(" \n\r\t");
                if (last != std::string::npos) key_value = key_value.substr(0, last + 1);
            }

            if (key_value.empty()) {
                std::cerr << "Usage Error: Missing key value.\n";
                continue;
            }

            try {
                // Use csv_name directly. If you typed "amazon.csv", 
                // formatFilename handles stripping the ".csv"
                BPlusTree* tree = StorageManager::loadIndex(csv_name, 4);
                std::vector<std::string> result = tree->find(key_value);
                
                if (result.empty()) {
                    std::cout << "Error: Key [" << key_value << "] not found.\n";
                } else {
                    // ... (Your table display logic) ...
                }
                delete tree;
            } catch (const std::exception& e) {
                std::cerr << "Engine Error: " << e.what() << "\n";
            }
        }
        else {
            std::cerr << "Unrecognized command. Type HELP for syntax.\n";
        }
    }
    return 0;
}