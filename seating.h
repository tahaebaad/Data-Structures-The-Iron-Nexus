#pragma once
#include "structs.h"
#include "logger.h"
#include <iostream>
#include <cstring>
#include <fstream>
using namespace std;

class SeatHashTable {
private:
    HashNode* table;
    int capacity;
    int count;

    int hashFunction(int seatNum) {
        unsigned int key = ((unsigned int)(seatNum) * 2654435761u) >> 16;
        return (int)(key % (unsigned int)capacity);
    }

    int probe(int seatNum) {
        int idx = hashFunction(seatNum);
        for (int i = 0; i < capacity; i++) {
            int pos = (idx + i) % capacity;
            if (!table[pos].occupied && !table[pos].deleted) return pos;
            if (table[pos].occupied && table[pos].data.seatNumber == seatNum) return pos;
        }
        return -1;
    }

    int find(int seatNum) {
        int idx = hashFunction(seatNum);
        for (int i = 0; i < capacity; i++) {
            int pos = (idx + i) % capacity;
            if (!table[pos].occupied && !table[pos].deleted) return -1;
            if (table[pos].occupied && table[pos].data.seatNumber == seatNum) return pos;
        }
        return -1;
    }

public:
    SeatHashTable(int cap = 997) : capacity(cap), count(0) {
        table = new HashNode[capacity];
    }

    ~SeatHashTable() { delete[] table; }

    bool insertSeat(int seatNum) {
        if (count >= (int)(capacity * 0.75)) return false;
        int pos = probe(seatNum);
        if (pos == -1) return false;
        if (table[pos].occupied && table[pos].data.seatNumber == seatNum) return false;
        table[pos].data = Seat(seatNum);
        table[pos].occupied = true;
        table[pos].deleted = false;
        count++;
        return true;
    }

    bool bookSeat(int seatNum, const char* passenger) {
        int pos = find(seatNum);
        if (pos == -1) {
            cout << RED << "\n  Seat " << seatNum << " does not exist.\n" << RST;
            return false;
        }
        if (table[pos].data.booked) {
            cout << YLW << "\n  Seat " << seatNum << " already booked by "
                 << table[pos].data.passengerName << ".\n" << RST;
            return false;
        }
        table[pos].data.booked = true;
        strncpy(table[pos].data.passengerName, passenger, 99);
        table[pos].data.passengerName[99] = '\0';
        return true;
    }

    bool cancelBooking(int seatNum) {
        int pos = find(seatNum);
        if (pos == -1) {
            cout << RED << "\n  Seat " << seatNum << " does not exist.\n" << RST;
            return false;
        }
        if (!table[pos].data.booked) {
            cout << YLW << "\n  Seat " << seatNum << " not booked.\n" << RST;
            return false;
        }
        table[pos].data.booked = false;
        table[pos].data.passengerName[0] = '\0';
        return true;
    }

    bool removeSeat(int seatNum) {
        int pos = find(seatNum);
        if (pos == -1) return false;
        table[pos].occupied = false;
        table[pos].deleted = true;
        count--;
        return true;
    }

    bool lookupSeat(int seatNum) {
        int pos = find(seatNum);
        if (pos == -1) {
            cout << RED << "\n  Seat " << seatNum << " does not exist.\n" << RST;
            return false;
        }
        cout << CYN << "\n  ===== SEAT LOOKUP =====" << RST << "\n";
        cout << "  Seat Number : " << table[pos].data.seatNumber << "\n";
        cout << "  Hash Index  : " << pos << "\n";
        if (table[pos].data.booked) {
            cout << "  Status      : " << RED << "BOOKED" << RST << "\n";
            cout << "  Passenger   : " << table[pos].data.passengerName << "\n";
        }
        else {
            cout << "  Status      : " << GRN << "AVAILABLE" << RST << "\n";
        }
        cout << CYN << "  ========================" << RST << "\n";
        return true;
    }

    void displayAll() {
        if (count == 0) {
            cout << YLW << "\n  No seats.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== HASH TABLE STATUS =====\n\n" << RST;
        cout << "  " << BLD;
        printPadded("Index", 7);
        cout << " ";
        printPadded("Seat", 6);
        cout << " ";
        printPadded("Status", 11);
        cout << " Passenger" << RST << "\n";
        cout << "  ------- ------ ----------- ----------\n";

        for (int i = 0; i < capacity; i++) {
            if (table[i].occupied) {
                cout << "  ";
                printPaddedInt(i, 7);
                cout << " ";
                printPaddedInt(table[i].data.seatNumber, 6);
                cout << " ";
                if (table[i].data.booked) {
                    cout << RED;
                    printPadded("BOOKED", 11);
                    cout << RST << " " << table[i].data.passengerName;
                }
                else {
                    cout << GRN;
                    printPadded("AVAILABLE", 11);
                    cout << RST << " -";
                }
                cout << "\n";
            }
        }
        cout << "\n  Seats: " << count << " / " << capacity << "\n";
        cout << CYN << "  =============================" << RST << "\n";
    }

    int getCount() { return count; }
    int getCapacity() { return capacity; }

    Seat* getSeatData(int seatNum) {
        int pos = find(seatNum);
        if (pos != -1) {
            return &table[pos].data;
        }
        return nullptr;
    }

    void resetAllBookings() {
        for (int i = 0; i < capacity; i++) {
            if (table[i].occupied) {
                table[i].data.booked = false;
                table[i].data.passengerName[0] = '\0';
            }
        }
    }
};

class SeatBST {
private:
    SeatBSTNode* root;
    int count;

    SeatBSTNode* insert(SeatBSTNode* n, Seat seat) {
        if (!n) {
            count++;
            return new SeatBSTNode(seat);
        }
        if (seat.seatNumber < n->data.seatNumber) {
            n->left = insert(n->left, seat);
        }
        else if (seat.seatNumber > n->data.seatNumber) {
            n->right = insert(n->right, seat);
        }
        else {
            n->data = seat;
        }
        return n;
    }

    SeatBSTNode* getMin(SeatBSTNode* n) {
        while (n->left) {
            n = n->left;
        }
        return n;
    }

    SeatBSTNode* remove(SeatBSTNode* n, int seatNum, bool& found) {
        if (!n) {
            found = false;
            return nullptr;
        }
        if (seatNum < n->data.seatNumber) {
            n->left = remove(n->left, seatNum, found);
        }
        else if (seatNum > n->data.seatNumber) {
            n->right = remove(n->right, seatNum, found);
        }
        else {
            found = true;
            if (!n->left) {
                SeatBSTNode* temp = n->right;
                delete n;
                count--;
                return temp;
            }
            if (!n->right) {
                SeatBSTNode* temp = n->left;
                delete n;
                count--;
                return temp;
            }
            SeatBSTNode* suc = getMin(n->right);
            n->data = suc->data;
            n->right = remove(n->right, suc->data.seatNumber, found);
        }
        return n;
    }

    void inorder(SeatBSTNode* n) {
        if (!n) return;
        inorder(n->left);
        cout << "  " << CYN << "|" << RST << " ";
        printPaddedInt(n->data.seatNumber, 6);
        cout << " " << CYN << "|" << RST << " ";
        if (n->data.booked) {
            cout << RED;
            printPadded("BOOKED", 11);
        }
        else {
            cout << GRN;
            printPadded("AVAILABLE", 11);
        }
        cout << RST << " " << CYN << "|" << RST << " ";
        if (n->data.booked) {
            cout << n->data.passengerName;
        }
        else {
            cout << "-";
        }
        cout << "\n";
        inorder(n->right);
    }

    void preorder(SeatBSTNode* n) {
        if (!n) return;
        cout << "  " << CYN << "|" << RST << " ";
        printPaddedInt(n->data.seatNumber, 6);
        cout << " " << CYN << "|" << RST << " ";
        if (n->data.booked) {
            cout << RED;
            printPadded("BOOKED", 11);
        }
        else {
            cout << GRN;
            printPadded("AVAILABLE", 11);
        }
        cout << RST << " " << CYN << "|" << RST << " ";
        if (n->data.booked) {
            cout << n->data.passengerName;
        }
        else {
            cout << "-";
        }
        cout << "\n";
        preorder(n->left);
        preorder(n->right);
    }

    void postorder(SeatBSTNode* n) {
        if (!n) return;
        postorder(n->left);
        postorder(n->right);
        cout << "  " << CYN << "|" << RST << " ";
        printPaddedInt(n->data.seatNumber, 6);
        cout << " " << CYN << "|" << RST << " ";
        if (n->data.booked) {
            cout << RED;
            printPadded("BOOKED", 11);
        }
        else {
            cout << GRN;
            printPadded("AVAILABLE", 11);
        }
        cout << RST << " " << CYN << "|" << RST << " ";
        if (n->data.booked) {
            cout << n->data.passengerName;
        }
        else {
            cout << "-";
        }
        cout << "\n";
    }

    void destroyTree(SeatBSTNode* n) {
        if (!n) return;
        destroyTree(n->left);
        destroyTree(n->right);
        delete n;
    }

    void printTree(SeatBSTNode* n, const char* prefix, bool isLeft) {
        if (!n) return;

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
        cout << BYLW << n->data.seatNumber << RST;
        if (n->data.booked) {
            cout << " " << RED << "[" << n->data.passengerName << "]" << RST;
        }
        cout << "\n";
        printTree(n->left, newPrefix, true);
    }

    void saveNode(ofstream& file, SeatBSTNode* n) {
        if (!n) {
            file << "#\n";
            return;
        }
        file << n->data.seatNumber << "|" << (n->data.booked ? 1 : 0) << "|";
        if (n->data.booked) {
            file << n->data.passengerName;
        }
        file << "\n";
        saveNode(file, n->left);
        saveNode(file, n->right);
    }

    SeatBSTNode* loadNode(ifstream& file) {
        char buffer[200];
        if (!file.getline(buffer, 200)) return nullptr;
        stripLine(buffer);
        if (buffer[0] == '#') return nullptr;

        char parts[3][100];
        int numParts = splitLine(buffer, '|', parts, 3);
        if (numParts < 2) return nullptr;

        int seatNum = strToInt(parts[0]);
        Seat s(seatNum);

        int bookedFlag = strToInt(parts[1]);
        if (bookedFlag && numParts >= 3 && parts[2][0]) {
            s.booked = true;
            strncpy(s.passengerName, parts[2], 99);
            s.passengerName[99] = '\0';
        }

        SeatBSTNode* node = new SeatBSTNode(s);
        count++;
        node->left = loadNode(file);
        node->right = loadNode(file);
        return node;
    }

    void writeHierarchical(ofstream& file, SeatBSTNode* n, const char* prefix, bool isLeft) {
        if (!n) return;

        char newPrefix[200];
        strcpy(newPrefix, prefix);
        if (isLeft) {
            strcat(newPrefix, "|   ");
        }
        else {
            strcat(newPrefix, "    ");
        }

        writeHierarchical(file, n->right, newPrefix, false);
        file << prefix;
        if (isLeft) {
            file << "\\-- ";
        }
        else {
            file << "/-- ";
        }
        file << n->data.seatNumber;
        if (n->data.booked) {
            file << " [" << n->data.passengerName << "]";
        }
        file << "\n";
        writeHierarchical(file, n->left, newPrefix, true);
    }

public:
    SeatBST() : root(nullptr), count(0) {}
    ~SeatBST() { destroyTree(root); }

    void insertSeat(Seat seat) { root = insert(root, seat); }

    bool removeSeat(int seatNum) {
        bool found = true;
        root = remove(root, seatNum, found);
        return found;
    }

    void updateSeat(Seat seat) { root = insert(root, seat); }

    void displayInorder() {
        if (!root) {
            cout << YLW << "\n  BST is empty.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== SEAT BST (Inorder - Sorted) =====\n\n" << RST;
        cout << "  " << BLD << "| ";
        printPadded("Seat", 6);
        cout << " | ";
        printPadded("Status", 11);
        cout << " | Passenger" << RST << "\n";
        cout << "  |--------|-------------|----------\n";
        inorder(root);
        cout << "\n  Total: " << count << " seats\n";
        cout << CYN << "  =======================================" << RST << "\n";
    }

    void displayPreorder() {
        if (!root) {
            cout << YLW << "\n  BST is empty.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== SEAT BST (Preorder) =====\n\n" << RST;
        cout << "  " << BLD << "| ";
        printPadded("Seat", 6);
        cout << " | ";
        printPadded("Status", 11);
        cout << " | Passenger" << RST << "\n";
        cout << "  |--------|-------------|----------\n";
        preorder(root);
        cout << "\n  Total: " << count << " seats\n";
        cout << CYN << "  ==============================" << RST << "\n";
    }

    void displayPostorder() {
        if (!root) {
            cout << YLW << "\n  BST is empty.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== SEAT BST (Postorder) =====\n\n" << RST;
        cout << "  " << BLD << "| ";
        printPadded("Seat", 6);
        cout << " | ";
        printPadded("Status", 11);
        cout << " | Passenger" << RST << "\n";
        cout << "  |--------|-------------|----------\n";
        postorder(root);
        cout << "\n  Total: " << count << " seats\n";
        cout << CYN << "  ================================" << RST << "\n";
    }

    void displayTree() {
        if (!root) {
            cout << YLW << "\n  BST is empty.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== BST TREE STRUCTURE =====\n\n" << RST;
        printTree(root, "", false);
        cout << CYN << "\n  ==============================" << RST << "\n";
    }

    int getCount() { return count; }

    void saveToFile(ofstream& file) {
        file << "BST " << count << "\n";
        saveNode(file, root);
    }

    void loadFromFile(ifstream& file) {
        destroyTree(root);
        root = nullptr;
        count = 0;
        root = loadNode(file);
    }

    void saveHierarchical(const char* filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error opening file.\n" << RST;
            return;
        }
        file << "========== SEAT BST HIERARCHICAL STRUCTURE ==========\n\n";
        writeHierarchical(file, root, "", false);
        file << "\n=====================================================\n";
        file << "Total seats: " << count << "\n";
        file.close();
        cout << GRN << "\n  BST hierarchy saved to " << filename << "\n" << RST;
    }

    void resetAll() {
        destroyTree(root);
        root = nullptr;
        count = 0;
    }
};

class SeatingChart {
private:
    SeatHashTable hashTable;
    SeatBST bst;
    int trainId;
    int totalSeats;
    Logger* logger;

public:
    SeatingChart(int tId, int seatCount, Logger* log)
        : hashTable(seatCount * 2 + 7),
          trainId(tId), totalSeats(seatCount), logger(log) {
        for (int i = 1; i <= seatCount; i++) {
            hashTable.insertSeat(i);
            bst.insertSeat(Seat(i));
        }
        char msg[256];
        char numBuf[20];
        strcpy(msg, "Seating chart: Train ");
        intToStr(tId, numBuf);
        strcat(msg, numBuf);
        strcat(msg, ", ");
        intToStr(seatCount, numBuf);
        strcat(msg, numBuf);
        strcat(msg, " seats");
        logger->log(msg);
    }

    bool bookSeat(int seatNum, const char* passenger, bool silent = false) {
        if (seatNum < 1 || seatNum > totalSeats) {
            if (!silent) {
                cout << RED << "\n  Invalid seat. Range: 1-" << totalSeats << "\n" << RST;
            }
            return false;
        }
        if (hashTable.bookSeat(seatNum, passenger)) {
            Seat s(seatNum);
            s.booked = true;
            strncpy(s.passengerName, passenger, 99);
            s.passengerName[99] = '\0';
            bst.updateSeat(s);
            if (!silent) {
                char msg[256];
                char numBuf[20];
                strcpy(msg, "Seat ");
                intToStr(seatNum, numBuf);
                strcat(msg, numBuf);
                strcat(msg, " booked for ");
                strcat(msg, passenger);
                strcat(msg, ", Train ");
                intToStr(trainId, numBuf);
                strcat(msg, numBuf);
                logger->log(msg);
                cout << GRN << "\n  Seat " << seatNum << " booked for "
                     << passenger << ".\n" << RST;
            }
            return true;
        }
        return false;
    }

    bool cancelBooking(int seatNum, bool silent = false) {
        if (seatNum < 1 || seatNum > totalSeats) {
            if (!silent) {
                cout << RED << "\n  Invalid seat.\n" << RST;
            }
            return false;
        }
        if (hashTable.cancelBooking(seatNum)) {
            bst.removeSeat(seatNum);
            bst.insertSeat(Seat(seatNum));
            if (!silent) {
                char msg[256];
                char numBuf[20];
                strcpy(msg, "Seat ");
                intToStr(seatNum, numBuf);
                strcat(msg, numBuf);
                strcat(msg, " cancelled, Train ");
                intToStr(trainId, numBuf);
                strcat(msg, numBuf);
                logger->log(msg);
                cout << GRN << "\n  Seat " << seatNum << " booking cancelled.\n" << RST;
            }
            return true;
        }
        return false;
    }

    bool removeSeatPermanent(int seatNum) {
        if (seatNum < 1 || seatNum > totalSeats) {
            cout << RED << "\n  Invalid seat.\n" << RST;
            return false;
        }
        if (hashTable.removeSeat(seatNum) && bst.removeSeat(seatNum)) {
            char msg[256];
            char numBuf[20];
            strcpy(msg, "Seat ");
            intToStr(seatNum, numBuf);
            strcat(msg, numBuf);
            strcat(msg, " permanently removed, Train ");
            intToStr(trainId, numBuf);
            strcat(msg, numBuf);
            logger->log(msg);
            cout << GRN << "\n  Seat " << seatNum
                 << " permanently removed (tombstone set).\n" << RST;
            return true;
        }
        cout << RED << "\n  Seat " << seatNum << " not found.\n" << RST;
        return false;
    }

    const char* getPassenger(int seatNum) {
        Seat* s = hashTable.getSeatData(seatNum);
        if (s && s->booked) {
            return s->passengerName;
        }
        return nullptr;
    }

    void lookupSeat(int seatNum) { hashTable.lookupSeat(seatNum); }
    void displayHashTable() { hashTable.displayAll(); }
    void displayBSTInorder() { bst.displayInorder(); }
    void displayBSTPreorder() { bst.displayPreorder(); }
    void displayBSTPostorder() { bst.displayPostorder(); }
    void displayBSTStructure() { bst.displayTree(); }
    int getTotal() { return totalSeats; }
    int getTrainId() { return trainId; }

    void saveToFile(const char* filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error opening file.\n" << RST;
            return;
        }
        file << trainId << " " << totalSeats << "\n";
        for (int i = 1; i <= totalSeats; i++) {
            Seat* s = hashTable.getSeatData(i);
            if (s) {
                file << s->seatNumber << "|" << (s->booked ? 1 : 0) << "|";
                if (s->booked) {
                    file << s->passengerName;
                }
                file << "\n";
            }
        }
        file.close();
        cout << GRN << "\n  Seating saved to " << filename << "\n" << RST;
    }

    void loadBookings(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error opening file.\n" << RST;
            return;
        }
        int tId = 0;
        int seats = 0;
        file >> tId >> seats;
        file.get(); // consume newline

        hashTable.resetAllBookings();
        bst.resetAll();
        for (int i = 1; i <= totalSeats; i++) {
            bst.insertSeat(Seat(i));
        }

        char buffer[200];
        while (file.getline(buffer, 200)) {
            stripLine(buffer);
            char parts[3][100];
            int numParts = splitLine(buffer, '|', parts, 3);
            if (numParts >= 2) {
                int num = strToInt(parts[0]);
                int booked = strToInt(parts[1]);
                if (booked && numParts >= 3 && num >= 1 && num <= totalSeats) {
                    bookSeat(num, parts[2], true);
                }
            }
        }
        file.close();
        cout << GRN << "\n  Bookings loaded from " << filename << "\n" << RST;
    }

    void saveHierarchical(const char* filename) { bst.saveHierarchical(filename); }
};
