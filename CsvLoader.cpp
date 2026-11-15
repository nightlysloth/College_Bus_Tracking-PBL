#pragma once
#include <iostream>
#include <fstream>
#include "Graph.cpp"

inline string trimString(string text) {
    int s = 0, e = (int)text.length() - 1;
    while (s <= e && (text[s] == ' ' || text[s] == '\t' || text[s] == '\r' || text[s] == '\n')) s++;
    while (e >= s && (text[e] == ' ' || text[e] == '\t' || text[e] == '\r' || text[e] == '\n')) e--;
    return s > e ? "" : text.substr(s, e - s + 1);
}

inline int splitString(string line, char sep, string parts[], int maxParts) {
    int count = 0;
    string cur = "";
    for (char c : line) {
        if (c == sep) {
            if (count < maxParts) parts[count] = trimString(cur);
            count++;
            cur = "";
        } else cur += c;
    }
    if (count < maxParts) parts[count] = trimString(cur);
    return ++count;
}

inline int loadStations(string path) {
    ifstream file(path.c_str());
    if (!file.is_open()) return -1;
    string line, parts[5];
    int lineNo = 0, loaded = 0;
    while (getline(file, line)) {
        lineNo++;
        line = trimString(line);
        if (lineNo == 1 || line.empty()) continue;
        if (splitString(line, ',', parts, 5) != 3) continue;
        int id = parseInt(parts[0]), serving = parseInt(parts[2]);
        if (id != -1 && serving != -1 && !parts[1].empty()) {
            if (sys.addStation(id, parts[1], serving) != -1) loaded++;
        }
    }
    return loaded;
}

inline int loadBuses(string path) {
    ifstream file(path.c_str());
    if (!file.is_open()) return -1;
    string line, parts[10];
    int lineNo = 0, loaded = 0;
    while (getline(file, line)) {
        lineNo++;
        line = trimString(line);
        if (lineNo == 1 || line.empty()) continue;
        if (splitString(line, ',', parts, 10) < 7) continue;

        int busNo = parseInt(parts[0]), dep = parseTime(parts[2]);
        char type = parts[4][0];
        if (busNo == -1 || dep == -1 || (type != 'S' && type != 'M')) continue;

        string stopNames[MAX_STOPS];
        int actualStops = splitString(parts[6], ';', stopNames, MAX_STOPS);
        int stopIdx[MAX_STOPS];
        bool ok = true;
        for (int i = 0; i < actualStops; i++) {
            stopIdx[i] = sys.findStation(stopNames[i]);
            if (stopIdx[i] == -1) { ok = false; break; }
        }
        if (ok && sys.addBus(busNo, parts[1], dep, parts[3], type, stopIdx, actualStops)) loaded++;
    }
    return loaded;
}

inline int loadEdges(string path) {
    ifstream file(path.c_str());
    if (!file.is_open()) return 0;
    string line, parts[5];
    int lineNo = 0, loaded = 0;
    while (getline(file, line)) {
        lineNo++;
        line = trimString(line);
        if (lineNo == 1 || line.empty()) continue;
        if (splitString(line, ',', parts, 5) != 3) continue;
        int u = sys.findStation(parts[0]), v = sys.findStation(parts[1]), m = parseInt(parts[2]);
        if (u != -1 && v != -1 && m > 0) {
            if (sys.addEdgeOverride(u, v, m)) loaded++;
        }
    }
    return loaded;
}

inline bool loadAllData(string folder) {
    int sc = loadStations(folder + "stations.csv");
    if (sc <= 0) return false;
    int bc = loadBuses(folder + "buses.csv");
    if (bc <= 0) return false;
    int ec = loadEdges(folder + "edges.csv");
    sys.sortBusesByNumber();
    sys.buildStationBusLists();
    buildGraphFromRoutes();
    cout << "Loaded " << sc << " stations, " << bc << " buses, " << ec << " edge overrides." << endl;
    return true;
}
