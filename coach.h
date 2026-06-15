#pragma once
#include "structs.h"
#include "logger.h"
#include <iostream>
#include <cstring>
#include <fstream>
using namespace std;

class CoachList {
private:
    Coach* head;
    Coach* tail;
    int size;
    int trainId;
    Logger* logger;

public:
    CoachList(int tId, Logger* log)
        : head(nullptr), tail(nullptr), size(0), trainId(tId), logger(log) {}

    ~CoachList() {
        Coach* cur = head;
        while (cur) {
            Coach* temp = cur;
            cur = cur->next;
            delete temp;
        }
    }

    void attachFront(int coachNum, const char* type, int cap, bool silent = false) {
        Coach* nc = new Coach(coachNum, type, cap);
        if (!head) {
            head = nc;
            tail = nc;
        }
        else {
            nc->next = head;
            head->prev = nc;
            head = nc;
        }
        size++;

        if (!silent) {
            char msg[256];
            strcpy(msg, "Coach #");
            char numBuf[20];
            intToStr(coachNum, numBuf);
            strcat(msg, numBuf);
            strcat(msg, " attached front, Train ");
            intToStr(trainId, numBuf);
            strcat(msg, numBuf);
            logger->log(msg);
            cout << GRN << "\n  Coach #" << coachNum << " attached at front.\n" << RST;
        }
    }

    void attachEnd(int coachNum, const char* type, int cap, bool silent = false) {
        Coach* nc = new Coach(coachNum, type, cap);
        if (!tail) {
            head = nc;
            tail = nc;
        }
        else {
            nc->prev = tail;
            tail->next = nc;
            tail = nc;
        }
        size++;

        if (!silent) {
            char msg[256];
            strcpy(msg, "Coach #");
            char numBuf[20];
            intToStr(coachNum, numBuf);
            strcat(msg, numBuf);
            strcat(msg, " attached end, Train ");
            intToStr(trainId, numBuf);
            strcat(msg, numBuf);
            logger->log(msg);
            cout << GRN << "\n  Coach #" << coachNum << " attached at end.\n" << RST;
        }
    }

    void insertAtPosition(int pos, int coachNum, const char* type, int cap) {
        if (pos < 1 || pos > size + 1) {
            cout << RED << "\n  Invalid position. Range: 1 to " << size + 1 << "\n" << RST;
            return;
        }
        if (pos == 1) {
            attachFront(coachNum, type, cap);
            return;
        }
        if (pos == size + 1) {
            attachEnd(coachNum, type, cap);
            return;
        }

        Coach* nc = new Coach(coachNum, type, cap);
        Coach* cur = head;
        for (int i = 1; i < pos; i++) {
            cur = cur->next;
        }
        nc->next = cur;
        nc->prev = cur->prev;
        cur->prev->next = nc;
        cur->prev = nc;
        size++;

        char msg[256];
        strcpy(msg, "Coach #");
        char numBuf[20];
        intToStr(coachNum, numBuf);
        strcat(msg, numBuf);
        strcat(msg, " inserted pos ");
        intToStr(pos, numBuf);
        strcat(msg, numBuf);
        strcat(msg, ", Train ");
        intToStr(trainId, numBuf);
        strcat(msg, numBuf);
        logger->log(msg);
        cout << GRN << "\n  Coach #" << coachNum << " inserted at position " << pos << ".\n" << RST;
    }

    bool deleteCoach(int coachNum, bool silent = false) {
        if (!head) {
            if (!silent) cout << YLW << "\n  No coaches.\n" << RST;
            return false;
        }
        Coach* cur = head;
        while (cur) {
            if (cur->coachNumber == coachNum) {
                if (cur == head && cur == tail) {
                    head = nullptr;
                    tail = nullptr;
                }
                else if (cur == head) {
                    head = head->next;
                    head->prev = nullptr;
                }
                else if (cur == tail) {
                    tail = tail->prev;
                    tail->next = nullptr;
                }
                else {
                    cur->prev->next = cur->next;
                    cur->next->prev = cur->prev;
                }
                delete cur;
                size--;

                if (!silent) {
                    char msg[256];
                    strcpy(msg, "Coach #");
                    char numBuf[20];
                    intToStr(coachNum, numBuf);
                    strcat(msg, numBuf);
                    strcat(msg, " removed, Train ");
                    intToStr(trainId, numBuf);
                    strcat(msg, numBuf);
                    logger->log(msg);
                    cout << GRN << "\n  Coach #" << coachNum << " removed.\n" << RST;
                }
                return true;
            }
            cur = cur->next;
        }
        if (!silent) {
            cout << RED << "\n  Coach #" << coachNum << " not found.\n" << RST;
        }
        return false;
    }

    bool getCoachInfo(int coachNum, char* outType, int& outCap) {
        Coach* cur = head;
        while (cur) {
            if (cur->coachNumber == coachNum) {
                strncpy(outType, cur->type, 49);
                outType[49] = '\0';
                outCap = cur->capacity;
                return true;
            }
            cur = cur->next;
        }
        return false;
    }

    void traverseForward() {
        if (!head) {
            cout << YLW << "\n  No coaches.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== COACHES (Front to Rear) =====\n\n" << RST;
        cout << "  " << MAG << "ENGINE" << RST << " <-> ";
        Coach* cur = head;
        while (cur) {
            cout << BLD << "[#" << cur->coachNumber << " " << cur->type
                 << " (" << cur->capacity << " seats)]" << RST;
            if (cur->next) {
                cout << " <-> ";
            }
            cur = cur->next;
        }
        cout << " <-> " << MAG << "CABOOSE" << RST << "\n";
        cout << "\n  Total: " << size << " coaches\n";
        cout << CYN << "  ===================================\n" << RST;
    }

    void traverseBackward() {
        if (!tail) {
            cout << YLW << "\n  No coaches.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== COACHES (Rear to Front) =====\n\n" << RST;
        cout << "  " << MAG << "CABOOSE" << RST << " <-> ";
        Coach* cur = tail;
        while (cur) {
            cout << BLD << "[#" << cur->coachNumber << " " << cur->type
                 << " (" << cur->capacity << " seats)]" << RST;
            if (cur->prev) {
                cout << " <-> ";
            }
            cur = cur->prev;
        }
        cout << " <-> " << MAG << "ENGINE" << RST << "\n";
        cout << "\n  Total: " << size << " coaches\n";
        cout << CYN << "  ===================================\n" << RST;
    }

    void reverse() {
        if (!head || head == tail) {
            cout << YLW << "\n  Nothing to reverse.\n" << RST;
            return;
        }
        Coach* cur = head;
        while (cur) {
            Coach* temp = cur->prev;
            cur->prev = cur->next;
            cur->next = temp;
            cur = cur->prev;
        }
        Coach* temp = head;
        head = tail;
        tail = temp;

        char msg[256];
        buildLogMsg(msg, "Coaches reversed, Train ", trainId, "");
        logger->log(msg);
        cout << GRN << "\n  Train orientation reversed.\n" << RST;
    }

    int getSize() { return size; }
    int getTrainId() { return trainId; }

    void saveToFile(const char* filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error opening file.\n" << RST;
            return;
        }
        file << trainId << " " << size << "\n";
        Coach* cur = head;
        while (cur) {
            file << cur->coachNumber << "|" << cur->type << "|" << cur->capacity << "\n";
            cur = cur->next;
        }
        file.close();
        cout << GRN << "\n  Coaches saved to " << filename << "\n" << RST;
    }

    void loadFromFile(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error opening file.\n" << RST;
            return;
        }

        // Clear existing coaches
        while (head) {
            Coach* temp = head;
            head = head->next;
            delete temp;
        }
        head = nullptr;
        tail = nullptr;
        size = 0;

        int tId = 0;
        int coachCount = 0;
        file >> tId >> coachCount;
        file.get(); // consume newline

        for (int i = 0; i < coachCount; i++) {
            char buffer[200];
            if (!file.getline(buffer, 200)) {
                break;
            }
            stripLine(buffer);
            char parts[3][100];
            int numParts = splitLine(buffer, '|', parts, 3);
            if (numParts >= 3) {
                int coachNum = strToInt(parts[0]);
                int capacity = strToInt(parts[2]);
                Coach* nc = new Coach(coachNum, parts[1], capacity);
                if (!tail) {
                    head = nc;
                    tail = nc;
                }
                else {
                    nc->prev = tail;
                    tail->next = nc;
                    tail = nc;
                }
                size++;
            }
        }
        file.close();
        cout << GRN << "\n  Coaches loaded (" << size << " coaches)\n" << RST;
    }
};
