#pragma once
#include "structs.h"
#include "logger.h"
#include <iostream>
#include <cstring>
#include <fstream>
using namespace std;

class TrainRegistry {
private:
    Train* root;
    int totalTrains;
    Logger* logger;
    int cachedId;
    Train* cachedNode;

    int getHeight(Train* n) { return n ? n->height : 0; }

    int getBalance(Train* n) {
        if (!n) return 0;
        return getHeight(n->left) - getHeight(n->right);
    }

    void updateHeight(Train* n) {
        int lh = getHeight(n->left);
        int rh = getHeight(n->right);
        if (lh > rh) {
            n->height = 1 + lh;
        }
        else {
            n->height = 1 + rh;
        }
    }

    Train* rotateRight(Train* y) {
        Train* x = y->left;
        Train* T2 = x->right;
        x->right = y;
        y->left = T2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    Train* rotateLeft(Train* x) {
        Train* y = x->right;
        Train* T2 = y->left;
        y->left = x;
        x->right = T2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    Train* balance(Train* n) {
        updateHeight(n);
        int bf = getBalance(n);
        if (bf > 1 && getBalance(n->left) >= 0) {
            return rotateRight(n);
        }
        if (bf > 1 && getBalance(n->left) < 0) {
            n->left = rotateLeft(n->left);
            return rotateRight(n);
        }
        if (bf < -1 && getBalance(n->right) <= 0) {
            return rotateLeft(n);
        }
        if (bf < -1 && getBalance(n->right) > 0) {
            n->right = rotateRight(n->right);
            return rotateLeft(n);
        }
        return n;
    }

    Train* insert(Train* n, int id, const char* name) {
        if (!n) {
            totalTrains++;
            return new Train(id, name);
        }
        if (id < n->id) {
            n->left = insert(n->left, id, name);
        }
        else if (id > n->id) {
            n->right = insert(n->right, id, name);
        }
        else {
            return n;
        }
        return balance(n);
    }

    Train* getMinNode(Train* n) {
        while (n->left) {
            n = n->left;
        }
        return n;
    }

    Train* remove(Train* n, int id, bool& found) {
        if (!n) {
            found = false;
            return nullptr;
        }
        if (id < n->id) {
            n->left = remove(n->left, id, found);
        }
        else if (id > n->id) {
            n->right = remove(n->right, id, found);
        }
        else {
            found = true;
            if (!n->left || !n->right) {
                Train* temp = n->left ? n->left : n->right;
                if (!temp) {
                    delete n;
                    return nullptr;
                }
                else {
                    Train* del = n;
                    n = temp;
                    delete del;
                }
            }
            else {
                Train* suc = getMinNode(n->right);
                n->id = suc->id;
                strncpy(n->name, suc->name, 99);
                n->name[99] = '\0';
                n->right = remove(n->right, suc->id, found);
            }
        }
        return balance(n);
    }

    Train* search(Train* n, int id) {
        if (!n) return nullptr;
        if (id == n->id) return n;
        if (id < n->id) return search(n->left, id);
        return search(n->right, id);
    }

    void inorder(Train* n) {
        if (!n) return;
        inorder(n->left);
        cout << "  " << CYN << "|" << RST << " ";
        printPaddedInt(n->id, 8);
        cout << " " << CYN << "|" << RST << " ";
        printPadded(n->name, 22);
        cout << " " << CYN << "|" << RST << "\n";
        inorder(n->right);
    }

    void preorder(Train* n) {
        if (!n) return;
        cout << "  " << CYN << "|" << RST << " ";
        printPaddedInt(n->id, 8);
        cout << " " << CYN << "|" << RST << " ";
        printPadded(n->name, 22);
        cout << " " << CYN << "|" << RST << "\n";
        preorder(n->left);
        preorder(n->right);
    }

    void postorder(Train* n) {
        if (!n) return;
        postorder(n->left);
        postorder(n->right);
        cout << "  " << CYN << "|" << RST << " ";
        printPaddedInt(n->id, 8);
        cout << " " << CYN << "|" << RST << " ";
        printPadded(n->name, 22);
        cout << " " << CYN << "|" << RST << "\n";
    }

    void destroyTree(Train* n) {
        if (!n) return;
        destroyTree(n->left);
        destroyTree(n->right);
        delete n;
    }

    void saveNode(ofstream& file, Train* n) {
        if (!n) {
            file << "#\n";
            return;
        }
        file << n->id << "|" << n->name << "\n";
        saveNode(file, n->left);
        saveNode(file, n->right);
    }

    Train* loadNode(ifstream& file) {
        char buffer[200];
        if (!file.getline(buffer, 200)) return nullptr;
        stripLine(buffer);
        if (buffer[0] == '#') return nullptr;

        char parts[2][100];
        if (splitLine(buffer, '|', parts, 2) < 2) return nullptr;

        Train* n = new Train(strToInt(parts[0]), parts[1]);
        totalTrains++;
        n->left = loadNode(file);
        n->right = loadNode(file);
        updateHeight(n);
        return n;
    }

    void printTree(Train* n, const char* prefix, bool isLeft) {
        if (!n) return;

        // Build new prefix for children
        char newPrefix[200];
        strcpy(newPrefix, prefix);
        if (isLeft) {
            strcat(newPrefix, "  |   ");
        }
        else {
            strcat(newPrefix, "      ");
        }

        printTree(n->right, newPrefix, false);
        cout << "  " << prefix;
        if (isLeft) {
            cout << "  \\-- ";
        }
        else {
            cout << "  /-- ";
        }
        cout << BYLW << n->id << RST << " (" << n->name << ")\n";
        printTree(n->left, newPrefix, true);
    }

    void collectIds(Train* n, int* ids, int& idx) {
        if (!n) return;
        collectIds(n->left, ids, idx);
        ids[idx] = n->id;
        idx++;
        collectIds(n->right, ids, idx);
    }

public:
    TrainRegistry(Logger* log)
        : root(nullptr), totalTrains(0), logger(log), cachedId(-1), cachedNode(nullptr) {}

    ~TrainRegistry() { destroyTree(root); }

    bool addTrain(int id, const char* name, bool silent = false) {
        int before = totalTrains;
        root = insert(root, id, name);
        cachedId = -1;
        cachedNode = nullptr;
        if (totalTrains > before) {
            if (!silent) {
                char msg[256];
                strcpy(msg, "Train ");
                char numBuf[20];
                intToStr(id, numBuf);
                strcat(msg, numBuf);
                strcat(msg, " (");
                strcat(msg, name);
                strcat(msg, ") registered");
                logger->log(msg);
                cout << GRN << "\n  Train " << id << " (" << name << ") registered.\n" << RST;
            }
            return true;
        }
        if (!silent) {
            cout << YLW << "\n  Train ID " << id << " already exists.\n" << RST;
        }
        return false;
    }

    bool removeTrain(int id, bool silent = false) {
        bool found = true;
        root = remove(root, id, found);
        cachedId = -1;
        cachedNode = nullptr;
        if (found) {
            totalTrains--;
            if (!silent) {
                char msg[256];
                buildLogMsg(msg, "Train ", id, " decommissioned");
                logger->log(msg);
                cout << GRN << "\n  Train " << id << " removed.\n" << RST;
            }
            return true;
        }
        if (!silent) {
            cout << RED << "\n  Train " << id << " not found.\n" << RST;
        }
        return false;
    }

    void searchTrain(int id) {
        Train* r = findTrain(id);
        if (r) {
            cout << GRN << "\n  ===== TRAIN FOUND =====" << RST << "\n";
            cout << "  ID:   " << r->id << "\n  Name: " << r->name << "\n";
            cout << GRN << "  ======================" << RST << "\n";
        }
        else {
            cout << RED << "\n  Train " << id << " not found.\n" << RST;
        }
    }

    Train* findTrain(int id) {
        if (id == cachedId && cachedNode) return cachedNode;
        Train* r = search(root, id);
        if (r) {
            cachedId = id;
            cachedNode = r;
        }
        return r;
    }

    void displayInorder() {
        if (!root) {
            cout << YLW << "\n  Registry is empty.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== TRAIN REGISTRY (Inorder) =====\n\n" << RST;
        cout << "  " << BLD << "| ";
        printPadded("ID", 8);
        cout << " | ";
        printPadded("Name", 22);
        cout << " |" << RST << "\n";
        cout << "  |----------|------------------------|\n";
        inorder(root);
        cout << "\n  Total: " << totalTrains << " trains\n";
        cout << CYN << "  ====================================\n" << RST;
    }

    void displayPreorder() {
        if (!root) {
            cout << YLW << "\n  Registry is empty.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== TRAIN REGISTRY (Preorder) =====\n\n" << RST;
        cout << "  " << BLD << "| ";
        printPadded("ID", 8);
        cout << " | ";
        printPadded("Name", 22);
        cout << " |" << RST << "\n";
        cout << "  |----------|------------------------|\n";
        preorder(root);
        cout << "\n  Total: " << totalTrains << " trains\n";
        cout << CYN << "  =====================================\n" << RST;
    }

    void displayPostorder() {
        if (!root) {
            cout << YLW << "\n  Registry is empty.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== TRAIN REGISTRY (Postorder) =====\n\n" << RST;
        cout << "  " << BLD << "| ";
        printPadded("ID", 8);
        cout << " | ";
        printPadded("Name", 22);
        cout << " |" << RST << "\n";
        cout << "  |----------|------------------------|\n";
        postorder(root);
        cout << "\n  Total: " << totalTrains << " trains\n";
        cout << CYN << "  ======================================\n" << RST;
    }

    void displayTree() {
        if (!root) {
            cout << YLW << "\n  Registry is empty.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== AVL TREE STRUCTURE =====\n\n" << RST;
        printTree(root, "", false);
        cout << CYN << "\n  ==============================\n" << RST;
    }

    int getTotal() { return totalTrains; }

    int getAllTrainIds(int* ids, int maxCount) {
        int cnt = 0;
        collectIds(root, ids, cnt);
        return cnt;
    }

    void saveToFile(const char* filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error opening file.\n" << RST;
            return;
        }
        file << totalTrains << "\n";
        saveNode(file, root);
        file.close();
        cout << GRN << "\n  Registry saved to " << filename << "\n" << RST;
    }

    void loadFromFile(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error opening file.\n" << RST;
            return;
        }
        destroyTree(root);
        root = nullptr;
        totalTrains = 0;
        cachedId = -1;
        cachedNode = nullptr;
        int sc = 0;
        file >> sc;
        file.get(); // consume newline
        root = loadNode(file);
        file.close();
        cout << GRN << "\n  Registry loaded (" << totalTrains << " trains)\n" << RST;
    }
};
