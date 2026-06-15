#pragma once
#include "structs.h"
#include "logger.h"
#include <iostream>
#include <cstring>
#include <fstream>
using namespace std;

class RailwayNetwork {
private:
    static const int MAX = 15;
    static const int INF = 99999;
    char stations[MAX][100];
    int matrix[MAX][MAX];
    int count;
    Logger* logger;

    int findStation(const char* name) {
        for (int i = 0; i < count; i++) {
            if (strcmp(stations[i], name) == 0) {
                return i;
            }
        }
        return -1;
    }

public:
    RailwayNetwork(Logger* log) : count(0), logger(log) {
        for (int i = 0; i < MAX; i++) {
            for (int j = 0; j < MAX; j++) {
                matrix[i][j] = 0;
            }
        }
    }

    int getStationCount() { return count; }

    const char* getStationName(int idx) {
        if (idx >= 0 && idx < count) return stations[idx];
        return nullptr;
    }

    int getRouteWeight(int from, int to) {
        if (from >= 0 && from < count && to >= 0 && to < count) {
            return matrix[from][to];
        }
        return 0;
    }

    int addStation(const char* name, bool silent = false) {
        if (count >= MAX) {
            if (!silent) cout << RED << "\n  Maximum stations (15) reached.\n" << RST;
            return -1;
        }
        if (findStation(name) != -1) {
            if (!silent) cout << YLW << "\n  Station already exists.\n" << RST;
            return -1;
        }
        strncpy(stations[count], name, 99);
        stations[count][99] = '\0';
        for (int i = 0; i <= count; i++) {
            matrix[count][i] = 0;
            matrix[i][count] = 0;
        }
        count++;
        if (!silent) {
            char msg[256];
            strcpy(msg, "Station '");
            strcat(msg, name);
            strcat(msg, "' added to network");
            logger->log(msg);
            cout << GRN << "\n  Station '" << name << "' added.\n" << RST;
        }
        return count - 1;
    }

    bool removeStation(const char* name, bool silent = false) {
        int idx = findStation(name);
        if (idx == -1) {
            if (!silent) cout << RED << "\n  Station not found.\n" << RST;
            return false;
        }
        int tempMatrix[MAX][MAX];
        char tempNames[MAX][100];
        int ni = 0;
        for (int i = 0; i < count; i++) {
            if (i == idx) continue;
            strcpy(tempNames[ni], stations[i]);
            int nj = 0;
            for (int j = 0; j < count; j++) {
                if (j == idx) continue;
                tempMatrix[ni][nj] = matrix[i][j];
                nj++;
            }
            ni++;
        }
        count--;
        for (int i = 0; i < count; i++) {
            strcpy(stations[i], tempNames[i]);
            for (int j = 0; j < count; j++) {
                matrix[i][j] = tempMatrix[i][j];
            }
        }
        if (!silent) {
            char msg[256];
            strcpy(msg, "Station '");
            strcat(msg, name);
            strcat(msg, "' removed");
            logger->log(msg);
            cout << GRN << "\n  Station '" << name << "' removed.\n" << RST;
        }
        return true;
    }

    bool addRoute(const char* from, const char* to, int weight, bool silent = false) {
        int f = findStation(from);
        int t = findStation(to);
        if (f == -1 || t == -1) {
            if (!silent) cout << RED << "\n  One or both stations not found.\n" << RST;
            return false;
        }
        if (f == t) {
            if (!silent) cout << RED << "\n  Cannot create self-loop.\n" << RST;
            return false;
        }
        matrix[f][t] = weight;
        matrix[t][f] = weight;
        if (!silent) {
            char msg[256];
            char numBuf[20];
            strcpy(msg, "Route: ");
            strcat(msg, from);
            strcat(msg, " <-> ");
            strcat(msg, to);
            strcat(msg, " (");
            intToStr(weight, numBuf);
            strcat(msg, numBuf);
            strcat(msg, " km)");
            logger->log(msg);
            cout << GRN << "\n  Route: " << from << " <-> " << to
                 << " (" << weight << " km) added.\n" << RST;
        }
        return true;
    }

    bool removeRoute(const char* from, const char* to, bool silent = false) {
        int f = findStation(from);
        int t = findStation(to);
        if (f == -1 || t == -1) {
            if (!silent) cout << RED << "\n  Station not found.\n" << RST;
            return false;
        }
        if (matrix[f][t] == 0) {
            if (!silent) cout << YLW << "\n  No route exists.\n" << RST;
            return false;
        }
        matrix[f][t] = 0;
        matrix[t][f] = 0;
        if (!silent) {
            char msg[256];
            strcpy(msg, "Route removed: ");
            strcat(msg, from);
            strcat(msg, " <-> ");
            strcat(msg, to);
            logger->log(msg);
            cout << GRN << "\n  Route removed.\n" << RST;
        }
        return true;
    }

    void displayStations() {
        if (count == 0) {
            cout << YLW << "\n  No stations.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== RAILWAY STATIONS =====\n\n" << RST;
        for (int i = 0; i < count; i++) {
            cout << "  " << BLD << i << RST << ". " << stations[i] << "\n";
        }
        cout << "\n  Total: " << count << " / " << MAX << " stations\n";
        cout << CYN << "  ============================\n" << RST;
    }

    void displayMatrix() {
        if (count == 0) {
            cout << YLW << "\n  No stations.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== ADJACENCY MATRIX =====\n\n" << RST;

        // Print header row
        cout << "  " << BLD;
        printRightStr("", 8);
        for (int i = 0; i < count; i++) {
            char shortName[6];
            strncpy(shortName, stations[i], 5);
            shortName[5] = '\0';
            printRightStr(shortName, 6);
        }
        cout << RST << "\n";

        // Print each row
        for (int i = 0; i < count; i++) {
            char shortName[6];
            strncpy(shortName, stations[i], 5);
            shortName[5] = '\0';
            cout << "  " << BLD;
            printRightStr(shortName, 6);
            cout << RST << "  ";
            for (int j = 0; j < count; j++) {
                if (matrix[i][j] > 0) {
                    cout << GRN;
                    printRightInt(matrix[i][j], 5);
                    cout << RST << " ";
                }
                else {
                    cout << "    0 ";
                }
            }
            cout << "\n";
        }
        cout << CYN << "\n  =============================\n" << RST;
    }

    void displayRoutes() {
        if (count == 0) {
            cout << YLW << "\n  No stations.\n" << RST;
            return;
        }
        cout << CYN << "\n  ===== ALL ROUTES =====\n\n" << RST;
        bool found = false;
        for (int i = 0; i < count; i++) {
            for (int j = i + 1; j < count; j++) {
                if (matrix[i][j] > 0) {
                    cout << "  " << stations[i] << " <--" << CYN << "["
                         << matrix[i][j] << " km]" << RST << "--> " << stations[j] << "\n";
                    found = true;
                }
            }
        }
        if (!found) {
            cout << YLW << "  No routes established.\n" << RST;
        }
        cout << CYN << "\n  ======================\n" << RST;
    }

    void bfs(const char* startName) {
        int start = findStation(startName);
        if (start == -1) {
            cout << RED << "\n  Station not found.\n" << RST;
            return;
        }

        bool visited[MAX] = {};
        int queue[MAX];
        int front = 0;
        int rear = 0;

        visited[start] = true;
        queue[rear] = start;
        rear++;

        cout << BGRN << "\n  ===== BFS TRAVERSAL =====" << RST << "\n";
        cout << "  Starting from: " << BLD << stations[start] << RST << "\n\n";

        int step = 1;
        while (front < rear) {
            int u = queue[front];
            front++;
            cout << "  " << CYN << step << RST << ". " << BLD << stations[u] << RST << "\n";
            step++;

            for (int v = 0; v < count; v++) {
                if (matrix[u][v] > 0 && !visited[v]) {
                    visited[v] = true;
                    queue[rear] = v;
                    rear++;
                }
            }
        }

        bool allVisited = true;
        for (int i = 0; i < count; i++) {
            if (!visited[i]) {
                allVisited = false;
                break;
            }
        }
        if (!allVisited) {
            cout << YLW << "\n  Warning: Some stations are unreachable from "
                 << startName << "\n" << RST;
        }
        cout << BGRN << "\n  =========================\n" << RST;
    }

    void dfs(const char* startName) {
        int start = findStation(startName);
        if (start == -1) {
            cout << RED << "\n  Station not found.\n" << RST;
            return;
        }

        bool visited[MAX] = {};
        int stack[MAX];
        int top = 0;

        stack[top] = start;
        top++;

        cout << BGRN << "\n  ===== DFS TRAVERSAL =====" << RST << "\n";
        cout << "  Starting from: " << BLD << stations[start] << RST << "\n\n";

        int step = 1;
        while (top > 0) {
            top--;
            int u = stack[top];
            if (visited[u]) continue;
            visited[u] = true;
            cout << "  " << CYN << step << RST << ". " << BLD << stations[u] << RST << "\n";
            step++;

            for (int v = count - 1; v >= 0; v--) {
                if (matrix[u][v] > 0 && !visited[v]) {
                    stack[top] = v;
                    top++;
                }
            }
        }
        cout << BGRN << "\n  =========================\n" << RST;
    }

    void dijkstra(const char* srcName, const char* destName) {
        int src = findStation(srcName);
        int dest = findStation(destName);
        if (src == -1 || dest == -1) {
            cout << RED << "\n  Station not found.\n" << RST;
            return;
        }
        if (src == dest) {
            cout << YLW << "\n  Source and destination are the same.\n" << RST;
            return;
        }

        int dist[MAX];
        bool visited[MAX] = {};
        int parent[MAX];

        for (int i = 0; i < count; i++) {
            dist[i] = INF;
            parent[i] = -1;
        }
        dist[src] = 0;

        for (int iter = 0; iter < count; iter++) {
            int u = -1;
            int minD = INF;
            for (int j = 0; j < count; j++) {
                if (!visited[j] && dist[j] < minD) {
                    minD = dist[j];
                    u = j;
                }
            }
            if (u == -1) break;
            visited[u] = true;
            for (int v = 0; v < count; v++) {
                if (!visited[v] && matrix[u][v] > 0 && dist[u] + matrix[u][v] < dist[v]) {
                    dist[v] = dist[u] + matrix[u][v];
                    parent[v] = u;
                }
            }
        }

        if (dist[dest] == INF) {
            cout << RED << "\n  No path exists between " << srcName
                 << " and " << destName << ".\n" << RST;
            return;
        }

        int path[MAX];
        int pathLen = 0;
        for (int c = dest; c != -1; c = parent[c]) {
            path[pathLen] = c;
            pathLen++;
        }
        // Reverse the path array
        for (int i = 0; i < pathLen / 2; i++) {
            int temp = path[i];
            path[i] = path[pathLen - 1 - i];
            path[pathLen - 1 - i] = temp;
        }

        cout << BGRN << "\n  ===== SHORTEST PATH FOUND =====" << RST << "\n";
        cout << "  From: " << BLD;
        printPadded(srcName, 15);
        cout << RST << "  To: " << BLD << destName << RST << "\n";
        cout << "  Total Distance: " << BYLW << dist[dest] << " km" << RST << "\n\n";

        for (int i = 0; i < pathLen - 1; i++) {
            cout << "  " << GRN;
            printPadded(stations[path[i]], 15);
            cout << RST << " " << CYN << "--[" << matrix[path[i]][path[i + 1]]
                 << " km]-->" << RST << " " << GRN << stations[path[i + 1]] << RST << "\n";
        }

        cout << BGRN << "\n  ================================\n" << RST;

        char msg[256];
        char numBuf[20];
        strcpy(msg, "Dijkstra: ");
        strcat(msg, srcName);
        strcat(msg, " -> ");
        strcat(msg, destName);
        strcat(msg, " = ");
        intToStr(dist[dest], numBuf);
        strcat(msg, numBuf);
        strcat(msg, " km");
        logger->log(msg);
    }

    void saveToFile(const char* filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error: Could not open file.\n" << RST;
            return;
        }
        file << count << "\n";
        for (int i = 0; i < count; i++) {
            file << stations[i] << "\n";
        }
        for (int i = 0; i < count; i++) {
            for (int j = 0; j < count; j++) {
                file << matrix[i][j] << " ";
            }
            file << "\n";
        }
        file.close();
        cout << GRN << "\n  Network saved to " << filename << "\n" << RST;
    }

    void loadFromFile(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << RED << "\n  Error: Could not open file.\n" << RST;
            return;
        }
        file >> count;
        file.get(); // consume newline
        for (int i = 0; i < count; i++) {
            file.getline(stations[i], 100);
            stripLine(stations[i]);
        }
        for (int i = 0; i < count; i++) {
            for (int j = 0; j < count; j++) {
                file >> matrix[i][j];
            }
        }
        file.close();
        cout << GRN << "\n  Network loaded (" << count << " stations)\n" << RST;
    }
};
