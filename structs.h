#pragma once
#include <iostream>
#include <cstring>
using namespace std;

// ==================== ANSI COLOR CODES ====================
#define RST  "\033[0m"
#define BLD  "\033[1m"
#define RED  "\033[31m"
#define GRN  "\033[32m"
#define YLW  "\033[33m"
#define BLU  "\033[34m"
#define MAG  "\033[35m"
#define CYN  "\033[36m"
#define BRED "\033[91m"
#define BGRN "\033[92m"
#define BYLW "\033[93m"
#define BCYN "\033[96m"

// ==================== HELPER: STRING TO INT ====================
// Replaces atoi / stoi. Handles optional leading '-' sign.
inline int strToInt(const char* str) {
    int result = 0;
    int i = 0;
    bool negative = false;

    if (str[0] == '-') {
        negative = true;
        i = 1;
    }

    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    if (negative) {
        result = -result;
    }
    return result;
}

// ==================== HELPER: INT TO STRING ====================
// Replaces snprintf / ostringstream for integer-to-char[] conversion.
inline void intToStr(int num, char* buffer) {
    if (num == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    bool negative = false;
    if (num < 0) {
        negative = true;
        num = -num;
    }

    char temp[20];
    int tempLen = 0;

    while (num > 0) {
        temp[tempLen] = '0' + (num % 10);
        tempLen++;
        num = num / 10;
    }

    int pos = 0;
    if (negative) {
        buffer[pos] = '-';
        pos++;
    }

    // Reverse the digits into the buffer
    for (int i = tempLen - 1; i >= 0; i--) {
        buffer[pos] = temp[i];
        pos++;
    }
    buffer[pos] = '\0';
}

// ==================== HELPER: PRINT LEFT-PADDED STRING ====================
// Prints text left-aligned in a field of the given width (like %-Xs).
inline void printPadded(const char* text, int width) {
    int textLen = strlen(text);
    cout << text;
    for (int i = textLen; i < width; i++) {
        cout << ' ';
    }
}

// ==================== HELPER: PRINT LEFT-PADDED INT ====================
// Prints an integer left-aligned in a field of the given width (like %-Xd).
inline void printPaddedInt(int num, int width) {
    char numStr[20];
    intToStr(num, numStr);
    printPadded(numStr, width);
}

// ==================== HELPER: PRINT RIGHT-PADDED INT ====================
// Prints an integer right-aligned in a field of the given width (like %Xd).
inline void printRightInt(int num, int width) {
    char numStr[20];
    intToStr(num, numStr);
    int numLen = strlen(numStr);
    for (int i = numLen; i < width; i++) {
        cout << ' ';
    }
    cout << numStr;
}

// ==================== HELPER: PRINT RIGHT-PADDED STRING ====================
// Prints a string right-aligned in a field of the given width (like %Xs).
inline void printRightStr(const char* text, int width) {
    int textLen = strlen(text);
    for (int i = textLen; i < width; i++) {
        cout << ' ';
    }
    cout << text;
}

// ==================== HELPER: BUILD LOG MESSAGE ====================
// Builds a log message by concatenating: prefix + number + suffix
// into a destination buffer. Used to replace ostringstream for logging.
inline void buildLogMsg(char* dest, const char* part1, int num, const char* part2) {
    char numStr[20];
    intToStr(num, numStr);
    strcpy(dest, part1);
    strcat(dest, numStr);
    strcat(dest, part2);
}

// ==================== HELPER: BUILD FILENAME ====================
// Builds a filename like "coaches_101.dat" into the dest buffer.
inline void buildFilename(char* dest, const char* prefix, int num, const char* suffix) {
    char numStr[20];
    intToStr(num, numStr);
    strcpy(dest, prefix);
    strcat(dest, numStr);
    strcat(dest, suffix);
}

// ==================== LINE PARSING UTILITIES ====================
inline int splitLine(const char* str, char delim, char parts[][100], int maxParts) {
    int p = 0;
    int c = 0;
    for (int i = 0; str[i] && p < maxParts; i++) {
        if (str[i] == delim) {
            parts[p][c] = '\0';
            p++;
            c = 0;
        }
        else if (str[i] != '\r' && str[i] != '\n') {
            if (c < 99) {
                parts[p][c] = str[i];
                c++;
            }
        }
    }
    parts[p][c] = '\0';
    return p + 1;
}

inline void stripLine(char* buf) {
    int len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        len--;
        buf[len] = '\0';
    }
}

// ==================== DATA STRUCTURES ====================
struct Train {
    int id;
    char name[100];
    Train* left;
    Train* right;
    int height;

    Train() : id(0), left(nullptr), right(nullptr), height(1) {
        name[0] = '\0';
    }

    Train(int _id, const char* _name) : id(_id), left(nullptr), right(nullptr), height(1) {
        strncpy(name, _name, 99);
        name[99] = '\0';
    }
};

struct Coach {
    int coachNumber;
    char type[50];
    int capacity;
    Coach* next;
    Coach* prev;

    Coach() : coachNumber(0), capacity(0), next(nullptr), prev(nullptr) {
        type[0] = '\0';
    }

    Coach(int num, const char* t, int cap) : coachNumber(num), capacity(cap), next(nullptr), prev(nullptr) {
        strncpy(type, t, 49);
        type[49] = '\0';
    }
};

struct LogEntry {
    char action[256];
    char timestamp[30];
    LogEntry* next;

    LogEntry() : next(nullptr) {
        action[0] = '\0';
        timestamp[0] = '\0';
    }

    LogEntry(const char* act, const char* ts) : next(nullptr) {
        strncpy(action, act, 255);
        action[255] = '\0';
        strncpy(timestamp, ts, 29);
        timestamp[29] = '\0';
    }
};

struct Seat {
    int seatNumber;
    bool booked;
    char passengerName[100];

    Seat() : seatNumber(0), booked(false) {
        passengerName[0] = '\0';
    }

    Seat(int num) : seatNumber(num), booked(false) {
        passengerName[0] = '\0';
    }
};

struct SeatBSTNode {
    Seat data;
    SeatBSTNode* left;
    SeatBSTNode* right;

    SeatBSTNode() : left(nullptr), right(nullptr) {}
    SeatBSTNode(Seat s) : data(s), left(nullptr), right(nullptr) {}
};

struct HashNode {
    Seat data;
    bool occupied;
    bool deleted;

    HashNode() : occupied(false), deleted(false) {}
};

struct Station {
    char name[100];

    Station() { name[0] = '\0'; }
    Station(const char* n) {
        strncpy(name, n, 99);
        name[99] = '\0';
    }
};

struct UndoAction {
    int type;
    int d1, d2, d3;
    char s1[100];
    char s2[100];
    UndoAction* next;

    UndoAction() : type(0), d1(0), d2(0), d3(0), next(nullptr) {
        s1[0] = '\0';
        s2[0] = '\0';
    }
};
