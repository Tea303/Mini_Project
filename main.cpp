#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iomanip>
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
    std::string inputLine;
    BPlusTree* activeTree = nullptr;
    std::string currentDataset = "";

    std::cout << "===============================================\n";
    std::cout << "        B+ Tree REPL Query Engine              \n";
    std::cout << "===============================================\n";
    std::cout << "Type HELP for syntax instructions.\n\n";

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, inputLine)) break;

        std::stringstream ss(inputLine);
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
            std::string filename, primaryCol;
            if (ss >> filename) {
                std::getline(ss >> std::ws, primaryCol); 
                
                if (primaryCol.empty()) {
                    std::cerr << "Usage Error: UPLOAD <filename.csv> <PRIMARY_COL>\n";
                    continue;
                }

                std::vector<std::vector<std::string>> validRecords;
                int pkIndex = -1;

                if (CSVParser::validateAndParse(filename, primaryCol, validRecords, pkIndex)) {
                    BPlusTree tree(4);
                    
                    std::ifstream file(filename);
                    std::string headerLine;
                    if (std::getline(file, headerLine)) {
                        tree.setHeaders(CSVParser::parseLine(headerLine));
                    }
                    file.close();

                    std::cout << "Indexing " << validRecords.size() << " records using [" << primaryCol << "]...\n";
                    for (const auto& record : validRecords) {
                        tree.insert(record[pkIndex], record); 
                    }
                    
                    StorageManager::saveIndex(tree, filename);
                    
                    if (activeTree) {
                        delete activeTree;
                        activeTree = nullptr;
                        currentDataset = "";
                    }
                }
            } else {
                std::cerr << "Usage Error: UPLOAD <filename.csv> <PRIMARY_COL>\n";
            }
        } 
        else if (command == "FIND") {
            std::string csvName, keyValue;
            if (ss >> csvName) {
                std::getline(ss >> std::ws, keyValue);

                if (!keyValue.empty()) {
                    size_t last = keyValue.find_last_not_of(" \n\r\t");
                    if (last != std::string::npos) {
                        keyValue = keyValue.substr(0, last + 1);
                    }
                }

                if (keyValue.empty()) {
                    std::cerr << "Usage Error: FIND <csv_name> <key_value>\n";
                    continue;
                }

                try {
                    if (activeTree == nullptr || csvName != currentDataset) {
                        std::cout << "Loading index [" << csvName << "] into memory... (Please wait)\n";
                        if (activeTree) delete activeTree;
                        
                        activeTree = StorageManager::loadIndex(csvName, 4);
                        currentDataset = csvName;
                        std::cout << "Index loaded successfully.\n";
                    }

                    std::vector<std::string> result = activeTree->find(keyValue);
                    
                    if (result.empty()) {
                        std::cout << "Error: Key [" << keyValue << "] not found.\n";
                    } else {
                        std::vector<std::string> fields;
                        std::stringstream rss(result[0]);
                        std::string segment;
                        while (std::getline(rss, segment, '\t')) {
                            fields.push_back(segment);
                        }

                        const std::vector<std::string>& headers = activeTree->getHeaders();

                        std::cout << "\n+--------------------------------+----------------------------------------------------+\n";
                        std::cout << "| SEARCH                         | Result for: " << std::left << std::setw(39) << keyValue << "|\n";
                        std::cout << "+--------------------------------+----------------------------------------------------+\n";
                        std::cout << "| CATEGORY NAME                  | FIELD VALUE                                        |\n";
                        std::cout << "+--------------------------------+----------------------------------------------------+\n";
                        
                        for (size_t i = 0; i < fields.size(); ++i) {
                            std::string label = (i < headers.size()) ? headers[i] : "Field " + std::to_string(i);
                            
                            std::cout << "| " << std::left << std::setw(30) << label << " | " 
                                      << std::left << std::setw(50) << fields[i] << " |\n";
                        }
                        
                        std::cout << "+--------------------------------+----------------------------------------------------+\n\n";
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Engine Error: " << e.what() << "\n";
                    activeTree = nullptr;
                    currentDataset = "";
                }
            } else {
                std::cerr << "Usage Error: FIND <csv_name> <key_value>\n";
            }
        } 
        else {
            std::cerr << "Unrecognized command. Type HELP for syntax.\n";
        }
    }

    if (activeTree) {
        delete activeTree;
    }

    return 0;
}