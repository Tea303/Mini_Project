# B+ Tree REPL Query Engine

## CPTS 223 -- Advanced Data Structures

**Author:** Yuta Raqueno  
**Institution:** Washington State University  

## Overview
This repository contains a single-threaded query engine backed by an in-memory B+ tree index. The final deliverable is not a one-off zipcode lookup tool. It is a general-purpose engine that works on any well-formed CSV file. You should be able to point it at a completely new dataset and have it work without modifying your code.

## Core Architecture
* **Robust CSV Parsing**: Designed to safely handle real-world, intentionally messy datasets by gracefully skipping rows with missing fields or inconsistent formatting without crashing.
* **Hierarchical B+ Tree**: Built with strict encapsulation using polymorphic node classes. Features recursive upward propagation for node bifurcation to maintain perfectly balanced access paths.
* **Disk Serialization**: Decouples tree logic from file I/O by persisting the leaf-node data layer directly to disk.

## Compilation
To compile the source code in a standard Linux/Ubuntu environment, run:
```bash
make