# The Iron Nexus: Railway Management System

The Iron Nexus is a command-line railway infrastructure simulation built entirely in C++. Developed as a comprehensive Data Structures project, this system models the complete lifecycle of a railway network—from train registration and coach management to route optimization and individual seat booking.

The core engineering constraint of this project was a strict **No STL (Standard Template Library)** rule. Every data structure was implemented from scratch using raw pointers, custom memory management, and standard character arrays.

### 🚂 System Modules & Data Structures

* **Train Registry (AVL Tree):** Manages a fleet of up to 10,000 trains using a self-balancing AVL Tree. Ensures $O(\log n)$ search time with an added LRU-1 cache for instant repeat lookups. Includes Preorder, Inorder, and Postorder tree visualizations.
* **Coach Management (Doubly Linked List):** Models the physical structure of a train. Supports $O(1)$ front/end attachments, specific position insertions, and bidirectional traversal (Engine to Caboose). Includes an in-place reversal algorithm.
* **Route Management (Graph):** Maps the railway network using an Adjacency Matrix. Implements **Dijkstra’s Algorithm** to calculate the shortest path between stations, alongside BFS and DFS for unreachability detection.
* **Seating Chart (Dual-Layer Hash + BST):** * *Layer 1:* A Hash Table using Knuth Multiplicative Hashing and linear probing for $O(1)$ instant seat booking/cancellations. 
    * *Layer 2:* A Binary Search Tree (BST) that mirrors the hash table to allow for structured, ordered display of seats.
* **Operation Logs (Stack):** A custom Linked-List based stack that records all system events with timestamps for LIFO retrieval.

### 🛠️ Technical Highlights

* **Zero STL Dependencies:** Built exclusively using `<iostream>`, `<fstream>`, `<cstring>`, and `<ctime>`. 
* **Memory Management:** Complete manual memory handling. Custom destructors and cleanup functions ensure zero memory leaks across all interconnecting nodes.
* **Global Undo/Redo:** A dual-stack architecture that tracks 11 different action types across all modules, allowing users to reverse network changes or seating bookings.
* **Robust File I/O:** Serializes the entire system state to pipe-delimited `.dat` files. Includes a bonus feature that saves the Seating BST in a visual, hierarchical ASCII format to `.txt` files.
* **Crash-Proof UI:** Features custom input-buffer clearing to prevent infinite loops on bad user input, wrapped in a fully ANSI color-coded terminal interface.

### 🚀 How to Run

1. Clone the repository: `git clone https://github.com/tahaebaad/The-Iron-Nexus.git`
2. Open your terminal in the project directory.
3. Compile using g++ (MinGW/MSYS2): `g++ main.cpp -o 6.exe`
4. Run the executable: `./6.exe`

### 👥 Author
* **Taha Ebaad (24I-0516)**
* Built for Data Structures, Spring 2026.