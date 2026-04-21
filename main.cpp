#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iomanip> // Required for formatting the table output

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
            if (ss >> filename >> primary_col) {
                std::vector<std::vector<std::string>> valid_records;
                int pk_index = -1;

                if (CSVParser::validateAndParse(filename, primary_col, valid_records, pk_index)) {
                    BPlusTree tree(4);
                    std::cout << "Indexing " << valid_records.size() << " records...\n";
                    for (const auto& record : valid_records) {
                        tree.insert(record[pk_index], record); 
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
                    // Load the index file from disk as required 
                    BPlusTree* tree = StorageManager::loadIndex(csv_name, 4);
                    std::vector<std::string> result = tree->find(key_value);
                    
                    if (result.empty()) {
                        std::cout << "Error: Key '" << key_value << "' not found.\n";
                    } else {
                        // Structured Table Output for better readability
                        std::vector<std::string> fields = CSVParser::parseLine(result[0]);
                        const std::vector<std::string>& headers = tree->getHeaders();

                        std::cout << "\n+------------+------------------------------------------------------+\n";
                        std::cout << "| SEARCH     | Result for: " << std::left << std::setw(41) << key_value << "|\n";
                        std::cout << "+------------+------------------------------------------------------+\n";
                        std::cout << "| COLUMN ID  | FIELD VALUE                                          |\n";
                        std::cout << "+------------+------------------------------------------------------+\n";
                        
                        for (size_t i = 0; i < fields.size(); ++i) {
                            // If headers exist, use them; otherwise, fall back to "Field i"
                            std::string label = (i < headers.size()) ? headers[i] : "Field " + std::to_string(i);
                            
                            std::cout << "| " << std::left << std::setw(10) << label << " | " 
                                    << std::left << std::setw(52) << fields[i] << " |\n";
                        }
                        
                        std::cout << "+------------+------------------------------------------------------+\n\n";
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
            std::cerr << "Unrecognized command. Type HELP for syntax.\n";
        }
    }
    return 0;
}