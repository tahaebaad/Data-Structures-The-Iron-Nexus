#include <iostream>
#include <cstring>
#include <fstream>
#include "structs.h"
#include "logger.h"
#include "coach.h"
#include "trainregistry.h"
#include "seating.h"
#include "network.h"
using namespace std;
#ifdef _WIN32
extern "C" {
    __declspec(dllimport) void* __stdcall GetStdHandle(unsigned long);
    __declspec(dllimport) int __stdcall GetConsoleMode(void*, unsigned long*);
    __declspec(dllimport) int __stdcall SetConsoleMode(void*, unsigned long);
}
#endif

Logger systemLogger;
TrainRegistry registry(&systemLogger);
RailwayNetwork network(&systemLogger);
UndoAction* undoStack = nullptr;
UndoAction* redoStack = nullptr;
bool suppressUndo = false;

struct TrainData {
    int trainId;
    CoachList* coaches;
    SeatingChart* seats;
    int seatCount;
    TrainData* next;
};
TrainData* allTrainData = nullptr;

TrainData* findTD(int id) {
    for (TrainData* t = allTrainData; t; t = t->next) {
        if (t->trainId == id) return t;
    }
    return nullptr;
}

TrainData* getOrCreateTD(int id) {
    TrainData* t = findTD(id);
    if (t) return t;
    t = new TrainData();
    t->trainId = id;
    t->coaches = new CoachList(id, &systemLogger);
    t->seats = nullptr;
    t->seatCount = 0;
    t->next = allTrainData;
    allTrainData = t;
    return t;
}

void enableColors() {
#ifdef _WIN32
    void* h = GetStdHandle((unsigned long)-11);
    unsigned long m = 0;
    GetConsoleMode(h, &m);
    SetConsoleMode(h, m | 0x0004);
#endif
}

void cls() {
    cout << "\033[2J\033[1;1H";
}

bool bufferDirty = true;

int getInt(const char* prompt = "") {
    if (prompt[0]) cout << prompt;
    int v;
    while (!(cin >> v)) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << RED << "  Invalid input. Enter a number: " << RST;
    }
    bufferDirty = true;
    return v;
}

void getStr(char* buf, int mx, const char* prompt = "") {
    if (prompt[0]) cout << prompt;
    if (bufferDirty) {
        cin.ignore(10000, '\n');
        bufferDirty = false;
    }
    cin.getline(buf, mx);
    stripLine(buf);
}

void pause() {
    cout << "\n  Press Enter to continue...";
    if (bufferDirty) {
        cin.ignore(10000, '\n');
        bufferDirty = false;
    }
    cin.get();
}

void hdr(const char* t) {
    cout << BLU << "\n  ================================================" << RST << "\n";
    cout << "    " << BLD << BCYN << t << RST << "\n";
    cout << BLU << "  ================================================" << RST << "\n";
}

void pushStack(UndoAction*& s, int ty, int a, int b, int c, const char* s1, const char* s2) {
    UndoAction* u = new UndoAction();
    u->type = ty;
    u->d1 = a;
    u->d2 = b;
    u->d3 = c;
    strncpy(u->s1, s1, 99);
    u->s1[99] = '\0';
    strncpy(u->s2, s2, 99);
    u->s2[99] = '\0';
    u->next = s;
    s = u;
}

void clearStack(UndoAction*& s) {
    while (s) {
        UndoAction* t = s;
        s = s->next;
        delete t;
    }
}

void record(int ty, int a, int b, int c, const char* s1, const char* s2) {
    if (suppressUndo) return;
    pushStack(undoStack, ty, a, b, c, s1, s2);
    clearStack(redoStack);
}

void doAction(int ty, int a, int b, int c, const char* s1, const char* s2) {
    suppressUndo = true;
    TrainData* td;
    switch (ty) {
        case 1:  registry.addTrain(a, s1, true); break;
        case 2:  registry.removeTrain(a, true); break;
        case 3:  network.addRoute(s1, s2, c, true); break;
        case 4:  network.removeRoute(s1, s2, true); break;
        case 5:  network.addStation(s1, true); break;
        case 6:  network.removeStation(s1, true); break;
        case 7:  td = findTD(a); if (td && td->coaches) td->coaches->attachEnd(b, s1, c, true); break;
        case 8:  td = findTD(a); if (td && td->coaches) td->coaches->deleteCoach(b, true); break;
        case 9:  td = findTD(a); if (td && td->seats) td->seats->bookSeat(b, s1, true); break;
        case 10: td = findTD(a); if (td && td->seats) td->seats->cancelBooking(b, true); break;
        case 11: td = findTD(a); if (td && td->coaches) td->coaches->attachFront(b, s1, c, true); break;
    }
    suppressUndo = false;
}

int reverseType(int t) {
    switch (t) {
        case 1:  return 2;
        case 2:  return 1;
        case 3:  return 4;
        case 4:  return 3;
        case 5:  return 6;
        case 6:  return 5;
        case 7:  return 8;
        case 8:  return 7;
        case 9:  return 10;
        case 10: return 9;
        case 11: return 8;
    }
    return 0;
}

void performUndo() {
    if (!undoStack) {
        cout << YLW << "\n  Nothing to undo.\n" << RST;
        return;
    }
    UndoAction* a = undoStack;
    undoStack = undoStack->next;
    int rev = reverseType(a->type);
    doAction(rev, a->d1, a->d2, a->d3, a->s1, a->s2);
    pushStack(redoStack, a->type, a->d1, a->d2, a->d3, a->s1, a->s2);
    cout << GRN << "\n  Undo successful.\n" << RST;
    delete a;
}

void performRedo() {
    if (!redoStack) {
        cout << YLW << "\n  Nothing to redo.\n" << RST;
        return;
    }
    UndoAction* a = redoStack;
    redoStack = redoStack->next;
    doAction(a->type, a->d1, a->d2, a->d3, a->s1, a->s2);
    pushStack(undoStack, a->type, a->d1, a->d2, a->d3, a->s1, a->s2);
    cout << GRN << "\n  Redo successful.\n" << RST;
    delete a;
}

void trainMenu() {
    int ch = 0;
    while (ch != 10) {
        cls();
        hdr("TRAIN REGISTRY (AVL Tree)");
        cout << "\n  Trains: " << BYLW << registry.getTotal() << RST << "\n";
        cout << "\n  " << BLD << "1." << RST << " Register    " << BLD << "2." << RST << " Decommission  " << BLD << "3." << RST << " Search\n";
        cout << "  " << BLD << "4." << RST << " Inorder     " << BLD << "5." << RST << " Preorder      " << BLD << "6." << RST << " Postorder\n";
        cout << "  " << BLD << "7." << RST << " Tree View   " << BLD << "8." << RST << " Save          " << BLD << "9." << RST << " Load\n";
        cout << "  " << BLD << "10." << RST << " Back\n";
        ch = getInt("\n  >> ");
        int id;
        char name[100];
        switch (ch) {
            case 1:
                id = getInt("\n  Train ID: ");
                getStr(name, 100, "  Train Name: ");
                if (registry.addTrain(id, name)) record(1, id, 0, 0, name, "");
                pause();
                break;
            case 2:
                id = getInt("\n  Train ID: ");
                {
                    Train* t = registry.findTrain(id);
                    char n[100] = "";
                    if (t) strncpy(n, t->name, 99);
                    if (registry.removeTrain(id)) record(2, id, 0, 0, n, "");
                }
                pause();
                break;
            case 3:  id = getInt("\n  Train ID: "); registry.searchTrain(id); pause(); break;
            case 4:  registry.displayInorder(); pause(); break;
            case 5:  registry.displayPreorder(); pause(); break;
            case 6:  registry.displayPostorder(); pause(); break;
            case 7:  registry.displayTree(); pause(); break;
            case 8:  registry.saveToFile("train_registry.dat"); pause(); break;
            case 9:  registry.loadFromFile("train_registry.dat"); pause(); break;
            case 10: break;
            default: cout << RED << "\n  Invalid.\n" << RST; pause();
        }
    }
}

void coachMenu() {
    int tid = getInt("\n  Enter Train ID: ");
    if (!registry.findTrain(tid)) {
        cout << RED << "\n  Train not found.\n" << RST;
        pause();
        return;
    }
    TrainData* td = getOrCreateTD(tid);
    CoachList* cl = td->coaches;
    int ch = 0;
    while (ch != 10) {
        cls();
        hdr("COACH MANAGEMENT (Doubly Linked List)");
        cout << "\n  Train: " << BYLW << tid << RST << "  Coaches: " << BYLW << cl->getSize() << RST << "\n";
        cout << "\n  " << BLD << "1." << RST << " Front  " << BLD << "2." << RST << " End  " << BLD << "3." << RST << " Position  " << BLD << "4." << RST << " Delete  " << BLD << "5." << RST << " Forward\n";
        cout << "  " << BLD << "6." << RST << " Backward  " << BLD << "7." << RST << " Reverse  " << BLD << "8." << RST << " Save  " << BLD << "9." << RST << " Load  " << BLD << "10." << RST << " Back\n";
        ch = getInt("\n  >> ");
        int cn, cap, pos;
        char type[50];
        char fn[100];
        switch (ch) {
            case 1:
                cn = getInt("\n  Coach#: "); getStr(type, 50, "  Type: "); cap = getInt("  Capacity: ");
                cl->attachFront(cn, type, cap); record(11, tid, cn, cap, type, ""); pause(); break;
            case 2:
                cn = getInt("\n  Coach#: "); getStr(type, 50, "  Type: "); cap = getInt("  Capacity: ");
                cl->attachEnd(cn, type, cap); record(7, tid, cn, cap, type, ""); pause(); break;
            case 3:
                pos = getInt("\n  Position: "); cn = getInt("  Coach#: "); getStr(type, 50, "  Type: "); cap = getInt("  Capacity: ");
                cl->insertAtPosition(pos, cn, type, cap); record(7, tid, cn, cap, type, ""); pause(); break;
            case 4:
                cn = getInt("\n  Coach# to delete: ");
                {
                    char ot[50]; int oc; bool got = cl->getCoachInfo(cn, ot, oc);
                    if (cl->deleteCoach(cn) && got) record(8, tid, cn, oc, ot, "");
                }
                pause(); break;
            case 5:  cl->traverseForward(); pause(); break;
            case 6:  cl->traverseBackward(); pause(); break;
            case 7:  cl->reverse(); pause(); break;
            case 8:
                buildFilename(fn, "coaches_", tid, ".dat");
                cl->saveToFile(fn); pause(); break;
            case 9:
                buildFilename(fn, "coaches_", tid, ".dat");
                cl->loadFromFile(fn); pause(); break;
            case 10: break;
            default: cout << RED << "\n  Invalid.\n" << RST; pause();
        }
    }
}

void networkMenu() {
    int ch = 0;
    while (ch != 13) {
        cls();
        hdr("RAILWAY NETWORK (Graph)");
        cout << "\n  Stations: " << BYLW << network.getStationCount() << "/15" << RST << "\n";
        cout << "\n  " << BLD << "1." << RST << " Add Station    " << BLD << "2." << RST << " Remove Station\n";
        cout << "  " << BLD << "3." << RST << " Add Route      " << BLD << "4." << RST << " Remove Route\n";
        cout << "  " << BLD << "5." << RST << " Stations       " << BLD << "6." << RST << " Matrix      " << BLD << "7." << RST << " Routes\n";
        cout << "  " << BLD << "8." << RST << " Dijkstra       " << BLD << "9." << RST << " BFS         " << BLD << "10." << RST << " DFS\n";
        cout << "  " << BLD << "11." << RST << " Save  " << BLD << "12." << RST << " Load  " << BLD << "13." << RST << " Back\n";
        ch = getInt("\n  >> ");
        char from[100], to[100];
        int w;
        switch (ch) {
            case 1:
                getStr(from, 100, "\n  Station name: ");
                if (network.addStation(from) >= 0) record(5, 0, 0, 0, from, "");
                pause(); break;
            case 2:
                getStr(from, 100, "\n  Station name: ");
                if (network.removeStation(from)) record(6, 0, 0, 0, from, "");
                pause(); break;
            case 3:
                getStr(from, 100, "\n  From: "); getStr(to, 100, "  To: "); w = getInt("  Distance(km): ");
                if (network.addRoute(from, to, w)) record(3, 0, 0, w, from, to);
                pause(); break;
            case 4:
                getStr(from, 100, "\n  From: "); getStr(to, 100, "  To: ");
                {
                    int f = -1, t2 = -1;
                    for (int i = 0; i < network.getStationCount(); i++) {
                        if (strcmp(network.getStationName(i), from) == 0) f = i;
                        if (strcmp(network.getStationName(i), to) == 0) t2 = i;
                    }
                    int ow = (f >= 0 && t2 >= 0) ? network.getRouteWeight(f, t2) : 0;
                    if (network.removeRoute(from, to)) record(4, 0, 0, ow, from, to);
                }
                pause(); break;
            case 5:  network.displayStations(); pause(); break;
            case 6:  network.displayMatrix(); pause(); break;
            case 7:  network.displayRoutes(); pause(); break;
            case 8:
                getStr(from, 100, "\n  Source: "); getStr(to, 100, "  Destination: ");
                network.dijkstra(from, to); pause(); break;
            case 9:  getStr(from, 100, "\n  Start station: "); network.bfs(from); pause(); break;
            case 10: getStr(from, 100, "\n  Start station: "); network.dfs(from); pause(); break;
            case 11: network.saveToFile("network.dat"); pause(); break;
            case 12: network.loadFromFile("network.dat"); pause(); break;
            case 13: break;
            default: cout << RED << "\n  Invalid.\n" << RST; pause();
        }
    }
}

void seatingMenu() {
    int tid = getInt("\n  Enter Train ID: ");
    if (!registry.findTrain(tid)) {
        cout << RED << "\n  Train not found.\n" << RST;
        pause();
        return;
    }
    TrainData* td = getOrCreateTD(tid);
    if (!td->seats) {
        int sc = getInt("  Total seats: ");
        if (sc <= 0) {
            cout << RED << "\n  Invalid.\n" << RST;
            pause();
            return;
        }
        td->seats = new SeatingChart(tid, sc, &systemLogger);
        td->seatCount = sc;
    }
    SeatingChart* chart = td->seats;
    int ch = 0;
    while (ch != 14) {
        cls();
        hdr("SEATING CHART (Hash + BST)");
        cout << "\n  Train: " << BYLW << tid << RST << "  Seats: " << BYLW << chart->getTotal() << RST << "\n";
        cout << "\n  " << BLD << "1." << RST << " Book       " << BLD << "2." << RST << " Cancel     " << BLD << "3." << RST << " Lookup(Hash)\n";
        cout << "  " << BLD << "4." << RST << " Hash Table " << BLD << "5." << RST << " Inorder    " << BLD << "6." << RST << " Preorder\n";
        cout << "  " << BLD << "7." << RST << " Postorder  " << BLD << "8." << RST << " BST View   " << BLD << "9." << RST << " Remove Seat\n";
        cout << "  " << BLD << "10." << RST << " Save      " << BLD << "11." << RST << " Load       " << BLD << "12." << RST << " Save BST Hierarchy\n";
        cout << "  " << BLD << "13." << RST << " Count     " << BLD << "14." << RST << " Back\n";
        ch = getInt("\n  >> ");
        int sn;
        char psg[100];
        char fn[100];
        switch (ch) {
            case 1:
                sn = getInt("\n  Seat#: "); getStr(psg, 100, "  Passenger: ");
                if (chart->bookSeat(sn, psg)) record(9, tid, sn, 0, psg, "");
                pause(); break;
            case 2:
                sn = getInt("\n  Seat#: ");
                {
                    const char* op = chart->getPassenger(sn);
                    char old[100] = "";
                    if (op) strncpy(old, op, 99);
                    if (chart->cancelBooking(sn)) record(10, tid, sn, 0, old, "");
                }
                pause(); break;
            case 3:  sn = getInt("\n  Seat#: "); chart->lookupSeat(sn); pause(); break;
            case 4:  chart->displayHashTable(); pause(); break;
            case 5:  chart->displayBSTInorder(); pause(); break;
            case 6:  chart->displayBSTPreorder(); pause(); break;
            case 7:  chart->displayBSTPostorder(); pause(); break;
            case 8:  chart->displayBSTStructure(); pause(); break;
            case 9:  sn = getInt("\n  Seat# to remove: "); chart->removeSeatPermanent(sn); pause(); break;
            case 10:
                buildFilename(fn, "seats_", tid, ".dat");
                chart->saveToFile(fn); pause(); break;
            case 11:
                buildFilename(fn, "seats_", tid, ".dat");
                chart->loadBookings(fn); pause(); break;
            case 12:
                buildFilename(fn, "seat_bst_", tid, ".txt");
                chart->saveHierarchical(fn); pause(); break;
            case 13: cout << "\n  Total seats: " << chart->getTotal() << "\n"; pause(); break;
            case 14: break;
            default: cout << RED << "\n  Invalid.\n" << RST; pause();
        }
    }
}

void logMenu() {
    int ch = 0;
    while (ch != 7) {
        cls();
        hdr("OPERATION LOG (Stack)");
        cout << "\n  Entries: " << BYLW << systemLogger.getCount() << RST << "\n";
        cout << "\n  " << BLD << "1." << RST << " View All  " << BLD << "2." << RST << " Delete Recent  " << BLD << "3." << RST << " Delete by Index\n";
        cout << "  " << BLD << "4." << RST << " Clear All " << BLD << "5." << RST << " Save  " << BLD << "6." << RST << " Load  " << BLD << "7." << RST << " Back\n";
        ch = getInt("\n  >> ");
        switch (ch) {
            case 1: systemLogger.displayLogs(); pause(); break;
            case 2: systemLogger.deleteRecent(); pause(); break;
            case 3: { int i = getInt("\n  Index: "); systemLogger.deleteByIndex(i); pause(); break; }
            case 4: systemLogger.clearLogs(); cout << GRN << "\n  All logs cleared.\n" << RST; pause(); break;
            case 5: systemLogger.saveToFile("logs.dat"); pause(); break;
            case 6: systemLogger.loadFromFile("logs.dat"); pause(); break;
            case 7: break;
            default: cout << RED << "\n  Invalid.\n" << RST; pause();
        }
    }
}

void saveAll() {
    cout << CYN << "\n  Saving all data...\n" << RST;
    registry.saveToFile("train_registry.dat");
    network.saveToFile("network.dat");
    systemLogger.saveToFile("logs.dat");
    for (TrainData* td = allTrainData; td; td = td->next) {
        char fn[100];
        if (td->coaches && td->coaches->getSize() > 0) {
            buildFilename(fn, "coaches_", td->trainId, ".dat");
            td->coaches->saveToFile(fn);
        }
        if (td->seats) {
            buildFilename(fn, "seats_", td->trainId, ".dat");
            td->seats->saveToFile(fn);
            buildFilename(fn, "seat_bst_", td->trainId, ".txt");
            td->seats->saveHierarchical(fn);
        }
    }
    cout << BGRN << "\n  All data saved successfully.\n" << RST;
}

void loadAll() {
    cout << CYN << "\n  Loading all data...\n" << RST;
    registry.loadFromFile("train_registry.dat");
    network.loadFromFile("network.dat");
    systemLogger.loadFromFile("logs.dat");
    int ids[10000];
    int cnt = registry.getAllTrainIds(ids, 10000);
    for (int i = 0; i < cnt; i++) {
        TrainData* td = getOrCreateTD(ids[i]);
        char fn[100];
        buildFilename(fn, "coaches_", ids[i], ".dat");
        {
            ifstream test(fn);
            if (test.is_open()) {
                test.close();
                td->coaches->loadFromFile(fn);
            }
        }
        buildFilename(fn, "seats_", ids[i], ".dat");
        {
            ifstream test(fn);
            if (test.is_open()) {
                int tId = 0, sc = 0;
                test >> tId >> sc;
                test.close();
                if (sc > 0) {
                    if (td->seats) delete td->seats;
                    td->seats = new SeatingChart(ids[i], sc, &systemLogger);
                    td->seatCount = sc;
                    td->seats->loadBookings(fn);
                }
            }
        }
    }
    cout << BGRN << "\n  All data loaded.\n" << RST;
}

void cleanupAll() {
    clearStack(undoStack);
    clearStack(redoStack);
    TrainData* td = allTrainData;
    while (td) {
        TrainData* nx = td->next;
        delete td->coaches;
        if (td->seats) delete td->seats;
        delete td;
        td = nx;
    }
}

int main() {
    enableColors();
    int ch = 0;
    while (ch != 9) {
        cls();
        cout << "\n" << BLU << "    ======================================================" << RST << "\n";
        cout << "    " << BLD << BCYN << "  THE IRON NEXUS" << RST << " " << CYN << "- Railway Management System" << RST << "\n";
        cout << BLU << "    ======================================================" << RST << "\n";
        cout << "\n  " << BLD << "1." << RST << " Train Registry       " << CYN << "(AVL Tree)" << RST << "\n";
        cout << "  " << BLD << "2." << RST << " Coach Management     " << CYN << "(Doubly Linked List)" << RST << "\n";
        cout << "  " << BLD << "3." << RST << " Railway Network      " << CYN << "(Graph + Dijkstra/BFS/DFS)" << RST << "\n";
        cout << "  " << BLD << "4." << RST << " Seating Chart        " << CYN << "(Hash Table + BST)" << RST << "\n";
        cout << "  " << BLD << "5." << RST << " Operation Logs       " << CYN << "(Stack)" << RST << "\n";
        cout << "  " << BLD << "6." << RST << " Save All Data\n";
        cout << "  " << BLD << "7." << RST << " Load All Data\n";
        cout << "  " << BLD << "8." << RST << " Undo / Redo\n";
        cout << "  " << BLD << "9." << RST << " Exit\n";
        ch = getInt("\n  >> ");
        switch (ch) {
            case 1: trainMenu(); break;
            case 2: coachMenu(); break;
            case 3: networkMenu(); break;
            case 4: seatingMenu(); break;
            case 5: logMenu(); break;
            case 6: saveAll(); pause(); break;
            case 7: loadAll(); pause(); break;
            case 8:
                cls();
                hdr("UNDO / REDO");
                cout << "\n  " << BLD << "1." << RST << " Undo   " << BLD << "2." << RST << " Redo   " << BLD << "3." << RST << " Back\n";
                {
                    int u = getInt("\n  >> ");
                    if (u == 1) performUndo();
                    else if (u == 2) performRedo();
                }
                pause();
                break;
            case 9:
                cleanupAll();
                cout << MAG << "\n  Shutting down The Iron Nexus...\n" << RST;
                break;
            default: cout << RED << "\n  Invalid.\n" << RST; pause();
        }
    }
    return 0;
}
