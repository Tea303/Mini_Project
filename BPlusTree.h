#include "BPlusTreeNodes.h" // Assumes the previous node definitions are here
#include <iostream>
#include <algorithm>

class BPlusTree {
private:
    Node* root;
    int max_capacity;

    // Helper to traverse down to the correct leaf
    LeafNode* findLeaf(const std::string& key) {
        if (!root) return nullptr;
        
        Node* current = root;
        while (!current->isLeaf()) {
            InternalNode* internal = static_cast<InternalNode*>(current);
            const auto& keys = internal->getKeys();
            const auto& children = internal->getChildren();
            
            int i = 0;
            // Linear scan for the correct branch (could be binary search for large capacities)
            while (i < keys.size() && key >= keys[i]) {
                i++;
            }
            current = children[i];
        }
        return static_cast<LeafNode*>(current);
    }

    // Handles the upward propagation of splits
    void insertInternal(const std::string& key, Node* cursor, Node* child) {
        InternalNode* internal = static_cast<InternalNode*>(cursor);
        
        // Find correct insertion index
        auto& keys = internal->getKeys(); // Note: Assumes Node class allows modifying keys for simplicity in this snippet
        auto& children = internal->getChildren();
        
        int i = 0;
        while (i < keys.size() && key > keys[i]) {
            i++;
        }
        
        keys.insert(keys.begin() + i, key);
        children.insert(children.begin() + i + 1, child);

        // Check if this internal node now exceeds its capacity
        if (keys.size() > max_capacity) {
            InternalNode* newInternal = new InternalNode(max_capacity);
            int splitIndex = keys.size() / 2;
            std::string splitKey = keys[splitIndex];
            
            // Move half the keys and children to the new node
            std::vector<std::string> newKeys(keys.begin() + splitIndex + 1, keys.end());
            std::vector<Node*> newChildren(children.begin() + splitIndex + 1, children.end());
            
            // Update the new internal node
            for (const auto& k : newKeys) newInternal->getKeys().push_back(k); // Assumes mutator exists
            for (const auto& c : newChildren) newInternal->addChild(c);
            
            // Shrink the old internal node
            keys.resize(splitIndex);
            children.resize(splitIndex + 1);

            // If we split the root, we must create a new root
            if (cursor == root) {
                InternalNode* newRoot = new InternalNode(max_capacity);
                newRoot->getKeys().push_back(splitKey);
                newRoot->addChild(cursor);
                newRoot->addChild(newInternal);
                root = newRoot;
            } else {
                // Find parent and propagate upwards (Requires parent pointers or a recursive implementation)
                // For a fully robust single-threaded engine, tracking the traversal path via a stack is standard.
                std::cout << "Upward propagation to parent triggered for key: " << splitKey << "\n";
            }
        }
    }

public:
    BPlusTree(int capacity = 3) : root(nullptr), max_capacity(capacity) {}

    // Search traversal
    std::vector<std::string> find(const std::string& key) {
        LeafNode* leaf = findLeaf(key);
        if (!leaf) return {}; // Tree is empty

        const auto& keys = leaf->getKeys();
        const auto& values = leaf->getValues();

        for (int i = 0; i < keys.size(); i++) {
            if (keys[i] == key) {
                // In this implementation, the value is stored as a single vector of strings per key
                return values; // Returning the raw record
            }
        }
        return {}; // Clear not-found case
    }

    // Insert with node splitting
    void insert(const std::string& key, const std::vector<std::string>& record) {
        if (!root) {
            root = new LeafNode(max_capacity);
            static_cast<LeafNode*>(root)->getKeys().push_back(key);
            static_cast<LeafNode*>(root)->getValues().push_back(record[0]); // Simplified to push the whole record string
            return;
        }

        LeafNode* leaf = findLeaf(key);
        auto& keys = leaf->getKeys();
        auto& values = leaf->getValues();

        // Insert into sorted position
        int i = 0;
        while (i < keys.size() && key > keys[i]) i++;
        
        keys.insert(keys.begin() + i, key);
        values.insert(values.begin() + i, record[0]);

        // Trigger bifurcation if the leaf is over capacity
        if (keys.size() > max_capacity) {
            LeafNode* newLeaf = new LeafNode(max_capacity);
            int splitIndex = keys.size() / 2;
            
            // Transfer half the data to the new leaf
            std::vector<std::string> newKeys(keys.begin() + splitIndex, keys.end());
            std::vector<std::string> newValues(values.begin() + splitIndex, values.end());
            
            for (const auto& k : newKeys) newLeaf->getKeys().push_back(k);
            for (const auto& v : newValues) newLeaf->getValues().push_back(v);

            // Shrink the old leaf
            keys.resize(splitIndex);
            values.resize(splitIndex);

            // Maintain the linked list
            newLeaf->setNextLeaf(leaf->getNextLeaf());
            leaf->setNextLeaf(newLeaf);

            // The split key to propagate upwards is the first key of the new right-hand leaf
            std::string splitKey = newLeaf->getKeys()[0];

            if (leaf == root) {
                InternalNode* newRoot = new InternalNode(max_capacity);
                newRoot->getKeys().push_back(splitKey);
                newRoot->addChild(leaf);
                newRoot->addChild(newLeaf);
                root = newRoot;
            } else {
                insertInternal(splitKey, root, newLeaf); // Simplified: actual implementation requires parent tracking
            }
        }
    }
};