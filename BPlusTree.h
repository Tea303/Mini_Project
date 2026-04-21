#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include "BPlusTreeNodes.h" 
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

class BPlusTree {
private:
    Node* root;
    int max_capacity;
    std::vector<std::string> headers;

    // Traverses down to the correct leaf while recording the ancestry for upward propagation
    LeafNode* findLeaf(const std::string& key, std::vector<Node*>& path) {
        if (!root) return nullptr;
        
        Node* current = root;
        while (!current->isLeaf()) {
            path.push_back(current); // Track the path for the split propagation stack
            InternalNode* internal = static_cast<InternalNode*>(current);
            const auto& keys = internal->getKeys();
            const auto& children = internal->getChildren();
            
            size_t i = 0;
            while (i < keys.size() && key >= keys[i]) {
                i++;
            }
            current = children[i];
        }
        return static_cast<LeafNode*>(current);
    }

public:
    BPlusTree(int capacity = 3) : root(nullptr), max_capacity(capacity) {}

    // Required by StorageManager for O(N) contiguous serialization traversal
    LeafNode* getFirstLeaf() const {
        if (!root) return nullptr;
        Node* current = root;
        while (!current->isLeaf()) {
            current = static_cast<InternalNode*>(current)->getChildren()[0];
        }
        return static_cast<LeafNode*>(current);
    }

    // Search traversal
    std::vector<std::string> find(const std::string& key) {
        std::vector<Node*> dummy_path; 
        LeafNode* leaf = findLeaf(key, dummy_path);
        if (!leaf) return {}; 

        const auto& keys = leaf->getKeys();
        const auto& values = leaf->getValues();

        for (size_t i = 0; i < keys.size(); i++) {
            if (keys[i] == key) {
                return {values[i]}; 
            }
        }
        return {}; 
    }

    // Insert with iterative upward node splitting and full record consolidation
    void insert(const std::string& key, const std::vector<std::string>& record) {
        // Concatenate the entire record vector into a single string to prevent truncation
        std::string full_record = "";
        for (size_t j = 0; j < record.size(); ++j) {
            full_record += record[j];
            if (j < record.size() - 1) {
                full_record += "\t";
            }
        }

        if (!root) {
            root = new LeafNode(max_capacity);
            static_cast<LeafNode*>(root)->getKeys().push_back(key);
            static_cast<LeafNode*>(root)->getValues().push_back(full_record); 
            return;
        }

        std::vector<Node*> path;
        LeafNode* leaf = findLeaf(key, path);
        auto& keys = leaf->getKeys();
        auto& values = leaf->getValues();

        // Insert into sorted position
        size_t i = 0; // Changed from int to size_t
        while (i < keys.size() && key > keys[i]) i++;;
        
        keys.insert(keys.begin() + i, key);
        values.insert(values.begin() + i, full_record); // Insert the consolidated string

        // If the leaf is within its cardinality limits, we are done
        if (keys.size() <= static_cast<size_t>(max_capacity)) return; // Explicit cast
        // --- Trigger Leaf Node Bifurcation ---
        LeafNode* newLeaf = new LeafNode(max_capacity);
        int splitIndex = keys.size() / 2;
        
        // Transfer half the data to the right sibling
        std::vector<std::string> newKeys(keys.begin() + splitIndex, keys.end());
        std::vector<std::string> newValues(values.begin() + splitIndex, values.end());
        
        for (const auto& k : newKeys) newLeaf->getKeys().push_back(k);
        for (const auto& v : newValues) newLeaf->getValues().push_back(v);

        // Shrink the old left leaf
        keys.resize(splitIndex);
        values.resize(splitIndex);

        // Maintain the linked list
        newLeaf->setNextLeaf(leaf->getNextLeaf());
        leaf->setNextLeaf(newLeaf);

        // The key that pushes upward from a leaf is a copy of the new leaf's first key
        std::string splitKey = newLeaf->getKeys()[0];
        Node* leftNode = leaf;
        Node* rightNode = newLeaf;

        // --- Trigger Upward Propagation ---
        while (true) {
            // Base Case: We have reached the top and must spawn a new root
            if (path.empty()) {
                InternalNode* newRoot = new InternalNode(max_capacity);
                newRoot->getKeys().push_back(splitKey);
                newRoot->addChild(leftNode);
                newRoot->addChild(rightNode);
                root = newRoot;
                break;
            }

            // Pop the immediate parent from the ancestry stack
            InternalNode* parent = static_cast<InternalNode*>(path.back());
            path.pop_back();

            auto& pKeys = parent->getKeys();
            auto& pChildren = parent->getChildren();

            // Insert the propagated key and right child into the parent
            size_t j = 0; // Changed from int to size_t
            while (j < pKeys.size() && splitKey > pKeys[j]) j++;

            pKeys.insert(pKeys.begin() + j, splitKey);
            pChildren.insert(pChildren.begin() + j + 1, rightNode);

            // If the parent absorbs the key successfully without exceeding capacity, propagation halts
            if (pKeys.size() <= static_cast<size_t>(max_capacity)) break;
            // --- Trigger Internal Node Bifurcation ---
            InternalNode* newInternal = new InternalNode(max_capacity);
            int pSplitIndex = pKeys.size() / 2;
            
            // For internal nodes, the median key pushes up and is REMOVED from the current node
            std::string nextSplitKey = pKeys[pSplitIndex];

            std::vector<std::string> newPKeys(pKeys.begin() + pSplitIndex + 1, pKeys.end());
            std::vector<Node*> newPChildren(pChildren.begin() + pSplitIndex + 1, pChildren.end());

            for (const auto& k : newPKeys) newInternal->getKeys().push_back(k);
            for (const auto& c : newPChildren) newInternal->addChild(c);

            pKeys.resize(pSplitIndex);
            pChildren.resize(pSplitIndex + 1);

            // Prepare variables for the next loop iteration upward
            splitKey = nextSplitKey;
            leftNode = parent;
            rightNode = newInternal;
        }
    }

    void setHeaders(const std::vector<std::string>& h) { headers = h; }
    const std::vector<std::string>& getHeaders() const { return headers; }
};

#endif // BPLUS_TREE_H