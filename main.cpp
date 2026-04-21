#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

#include "BPlusTree.h"
#include "CSVParser.h"
#include "StorageManager.h"

void printHelp() {
    std::cout << "\nAvailable Commands:\n"
              << "  PEEK <filename.csv>                 - Reads only the first two rows and displays column indexes and sample values.\n"
              << "  UPLOAD <filename.csv> <PRIMARY_COL> - Reads the file, builds the primary B+ tree index, and saves it to disk.\n"
              << "  FIND <csv_name> <key_value>         - Constructs the index filename, loads the tree from disk, and retrieves the record.\n"
              << "  HELP                                - Prints a summary of all available commands and their syntax.\n"
              << "  EXIT                                - Exits the REPL cleanly.\n\n";
}

int main() {
    std::string input_line;
    std::cout << "=======================================\n";
    std::cout << "      B+ Tree REPL Query Engine        \n";
    std::cout << "=======================================\n";
    std::cout << "Type HELP for a list of commands.\n\n";

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input_line)) break;

        std::stringstream ss(input_line);
        std::string command;
        ss >> command;

        if (command.empty()) continue;

        if (command == "EXIT") {
            std::cout << "Exiting query engine. Goodbye!\n";
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
            if (ss >> filename >> primary_col) {
                std::vector<std::vector<std::string>> valid_records;
                int pk_index = -1;

                if (CSVParser::validateAndParse(filename, primary_col, valid_records, pk_index)) {
                    BPlusTree tree(4);
                    
                    std::cout << "Building B+ tree index for " << valid_records.size() << " valid records...\n";
                    for (const auto& record : valid_records) {
                        std::string key = record[pk_index];
                        tree.insert(key, record); 
                    }
                    
                    StorageManager::saveIndex(tree, filename);
                }
            } else {
                std::cerr << "Usage Error: UPLOAD <filename.csv> <PRIMARY_COL>\n";
            }
        } 
        else if (command == "FIND") {
            std::string csv_name, key_value;
            if (ss >> csv_name >> key_value) {
                try {
                    BPlusTree* tree = StorageManager::loadIndex(csv_name, 4);
                    std::vector<std::string> result = tree->find(key_value);
                    
                    if (result.empty()) {
                        std::cout << "Error: Record not found for primary key '" << key_value << "'.\n";
                    } else {
                        std::cout << "Record Found:\n";
                        for (size_t i = 0; i < result.size(); ++i) {
                            std::cout << result[i] << (i < result.size() - 1 ? ", " : ""); 
                        }
                        std::cout << "\n";
                    }
                    
                    delete tree; 
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << "\n";
                }
            } else {
                std::cerr << "Usage Error: FIND <csv_name> <key_value>\n";
            }
        } 
        else {
            std::cerr << "Unrecognized command '" << command << "'. Type HELP for a list of valid commands.\n";
        }
    }
    return 0;
}