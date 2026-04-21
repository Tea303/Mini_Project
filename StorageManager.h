#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "BPlusTree.h"
#include <string>
#include <vector>

class StorageManager {
private:
    static std::string formatFilename(const std::string& csv_filename);

public:
    static void saveIndex(BPlusTree& tree, const std::string& csv_filename);
    static BPlusTree* loadIndex(const std::string& dataset_name, int capacity = 3);
};

#endif // STORAGE_MANAGER_H