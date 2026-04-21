#ifndef BPLUS_TREE_NODES_H
#define BPLUS_TREE_NODES_H

#include <string>
#include <vector>

/**
 * Base Node Class
 * Establishes the foundational properties shared by both internal and leaf nodes.
 */
class Node {
protected:
    bool is_leaf;
    int max_capacity;
    std::vector<std::string> keys;

public:
    Node(bool leaf_status, int capacity) 
        : is_leaf(leaf_status), max_capacity(capacity) {}
    
    virtual ~Node() = default;

    // Accessors
    bool isLeaf() const { 
        return is_leaf; 
    }
    
    int getMaxCapacity() const { 
        return max_capacity; 
    }
    
    int getCurrentKeyCount() const { 
        return keys.size(); 
    }
    
    const std::vector<std::string>& getKeys() const { 
        return keys; 
    }
};

/**
 * Leaf Node Class
 * Stores the actual key-value pairs where the value represents the CSV record.
 */
class LeafNode : public Node {
private:
    std::vector<std::string> values; // The string representation of the dataset row
    LeafNode* next_leaf;             // Pointer to maintain the linked-list property at the leaf level

public:
    LeafNode(int capacity) 
        : Node(true, capacity), next_leaf(nullptr) {}
    
    ~LeafNode() override = default;

    // Accessors
    const std::vector<std::string>& getValues() const { 
        return values; 
    }
    
    LeafNode* getNextLeaf() const { 
        return next_leaf; 
    }

    // Mutators
    void setNextLeaf(LeafNode* neighbor) { 
        next_leaf = neighbor; 
    }
};

/**
 * Internal Node Class
 * Acts as the routing mechanism, containing keys and polymorphic pointers to child nodes.
 */
class InternalNode : public Node {
private:
    std::vector<Node*> children;

public:
    InternalNode(int capacity) 
        : Node(false, capacity) {}
    
    ~InternalNode() override = default;

    // Accessors
    const std::vector<Node*>& getChildren() const { 
        return children; 
    }

    // Mutators
    // Note: Foundational setup. Advanced insertion logic (splitting) will manage these later.
    void addChild(Node* child_node) { 
        children.push_back(child_node); 
    }
};

#endif // BPLUS_TREE_NODES_H