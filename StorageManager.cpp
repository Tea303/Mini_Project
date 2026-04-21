#include "StorageManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::string StorageManager::formatFilename(const std::string& csv_filename) {
    size_t dot_pos = csv_filename.find_last_of(".");
    std::string base_name = (dot_pos == std::string::npos) ? csv_filename : csv_filename.substr(0, dot_pos);
    return base_name + "_pi.db";
}

void StorageManager::saveIndex(BPlusTree& tree, const std::string& csv_filename) {
    std::string db_filename = formatFilename(csv_filename);
    std::ofstream out(db_filename, std::ios::trunc);

    out << "HEADER|"; 
    for (size_t i = 0; i < tree.getHeaders().size(); ++i) {
        out << tree.getHeaders()[i] << (i < tree.getHeaders().size() - 1 ? "," : "");
    }
    out << "\n";

    if (!out.is_open()) {
        std::cerr << "Error: Could not open " << db_filename << " for writing.\n";
        return;
    }

    LeafNode* current_leaf = tree.getFirstLeaf();
    int record_count = 0;

    while (current_leaf != nullptr) {
        const auto& keys = current_leaf->getKeys();
        const auto& values = current_leaf->getValues();

        for (size_t i = 0; i < keys.size(); ++i) {
            out << keys[i] << "|" << values[i] << "\n";
            record_count++;
        }
        current_leaf = current_leaf->getNextLeaf();
    }

    out.close();
    std::cout << "Successfully serialized " << record_count << " records to " << db_filename << "\n";
}

BPlusTree* StorageManager::loadIndex(const std::string& dataset_name, int capacity) {
    std::string db_filename = formatFilename(dataset_name);
    std::ifstream in(db_filename);

    if (!in.is_open()) {
        throw std::runtime_error("Index file " + db_filename + " does not exist. Run UPLOAD first.");
    }

    BPlusTree* tree = new BPlusTree(capacity);
    std::string line;
    std::string line;

    if (std::getline(in, line)) {
        if (line.substr(0, 7) == "HEADER|") {
            std::string header_str = line.substr(7);
            tree->setHeaders(CSVParser::parseLine(header_str));
        }
    }
    
    while (std::getline(in, line)) {
        size_t delim_pos = line.find("|");
        if (delim_pos != std::string::npos) {
            std::string key = line.substr(0, delim_pos);
            std::string record = line.substr(delim_pos + 1);
            
            tree->insert(key, {record}); 
        }
    }

    in.close();
    return tree;
}