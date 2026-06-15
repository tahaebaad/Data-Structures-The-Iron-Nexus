#pragma once
#include "structs.h"
#include <ctime>
#include <cstring>
#include <iostream>
#include <fstream>
using namespace std;

class Logger {
private:
    LogEntry* top;
    int count;

    char* getCurrentTime() {
        static char buffer[30];
        time_t now = time(0);
        struct tm* t = localtime(&now);
        strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", t);
        return buffer;
    }

public:
    Logger() : top(nullptr), count(0) {}

    ~Logger() {
        while (top) {
            LogEntry* temp = top;
            top = top->next;
            delete temp;
        }
    }

    void log(const char* action) {
        char* ts = getCurrentTime();
        LogEntry* entry = new LogEntry(action, ts);
        entry->next = top;
        top = entry;
        count++;
    }

    void displayLogs() {
        if (!top) {
            cout << YLW << "\n  No logs recorded yet.\n" << RST;
            return;
        }
        cout << MAG << "\n  ===== OPERATION LOG (Most Recent First) =====\n\n" << RST;
        LogEntry* cur = top;
        int idx = 1;
        while (cur) {
            cout << "  " << BLD << idx << RST << ". "
                 << CYN << "[" << cur->timestamp << "]" << RST
                 << "  " << cur->action << "\n";
            cur = cur->next;
            idx++;
        }
        cout << "\n  Total entries: " << count << "\n";
        cout << MAG << "  =============================================\n" << RST;
    }

    void clearLogs() {
        while (top) {
            LogEntry* temp = top;
            top = top->next;
            delete temp;
        }
        count = 0;
    }

    bool deleteRecent() {
        if (!top) {
            cout << YLW << "\n  No logs to delete.\n" << RST;
            return false;
        }
        LogEntry* temp = top;
        cout << GRN << "\n  Deleted log: [" << temp->timestamp << "]  "
             << temp->action << "\n" << RST;
        top = top->next;
        delete temp;
        count--;
        return true;
    }

    bool deleteByIndex(int idx) {
        if (idx < 1 || idx > count) {
            cout << RED << "\n  Invalid index.\n" << RST;
            return false;
        }
        if (idx == 1) {
            return deleteRecent();
        }

        LogEntry* cur = top;
        for (int i = 1; i < idx - 1; i++) {
            cur = cur->next;
        }

        LogEntry* toDelete = cur->next;
        cur->next = toDelete->next;
        cout << GRN << "\n  Deleted log: [" << toDelete->timestamp << "]  "
             << toDelete->action << "\n" << RST;
        delete toDelete;
        count--;
        return true;
    }

    int getCount() { return count; }
    LogEntry* getTop() { return top; }

    void saveToFile(const char* filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error: Could not open file.\n" << RST;
            return;
        }
        LogEntry* cur = top;
        while (cur) {
            file << cur->timestamp << "|" << cur->action << "\n";
            cur = cur->next;
        }
        file.close();
        cout << GRN << "\n  Logs saved to " << filename << "\n" << RST;
    }

    void loadFromFile(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error: Could not open file.\n" << RST;
            return;
        }

        clearLogs();

        char lines[1000][300];
        int lineCount = 0;
        char buffer[300];
        while (file.getline(buffer, 300) && lineCount < 1000) {
            stripLine(buffer);
            strncpy(lines[lineCount], buffer, 299);
            lines[lineCount][299] = '\0';
            lineCount++;
        }
        file.close();

        for (int i = lineCount - 1; i >= 0; i--) {
            char parts[2][100];
            if (splitLine(lines[i], '|', parts, 2) < 2) {
                continue;
            }
            LogEntry* entry = new LogEntry(parts[1], parts[0]);
            entry->next = top;
            top = entry;
            count++;
        }
        cout << GRN << "\n  Logs loaded from " << filename
             << " (" << count << " entries)\n" << RST;
    }
};
