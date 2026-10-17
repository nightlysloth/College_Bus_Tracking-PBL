// =====================================================================
// File: CsvLoader.cpp
// Purpose: Loads stations.csv, buses.csv, and edges.csv.
//
// Simple line-by-line reading with ifstream and basic string splitting.
// No static functions or variables. All functions use call-by-value.
// =====================================================================
#include <iostream>
#include <fstream>
#include "CsvLoader.h"

// Check if character is whitespace or newline
bool isWhitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

// Remove spaces and newlines from both ends of string
string trimString(string text) {
    int start = 0;
    int end = (int)text.length() - 1;

    while (start <= end && isWhitespace(text[start])) {
        start++;
    }
    while (end >= start && isWhitespace(text[end])) {
        end--;
    }
    if (start > end) {
        return "";
    }
    return text.substr(start, end - start + 1);
}

// Split a line into parts by a separator character (e.g. ',' or ';')
int splitString(string line, char separator, string parts[], int maxParts) {
    int count = 0;
    string current = "";

    for (int i = 0; i < (int)line.length(); i++) {
        if (line[i] == separator) {
            if (count < maxParts) {
                parts[count] = trimString(current);
            }
            count++;
            current = "";
        } else {
            current += line[i];
        }
    }
    if (count < maxParts) {
        parts[count] = trimString(current);
    }
    count++;
    return count;
}

// ---------------------------------------------------------------------
// Load stations from stations.csv
// Format: station_id,station_name,buses_serving
// ---------------------------------------------------------------------
int loadStations(string path) {
    ifstream file(path.c_str());
    if (!file.is_open()) {
        return -1;
    }

    string line;
    int lineNo = 0;
    int loaded = 0;
    string parts[5];

    while (getline(file, line)) {
        lineNo++;
        line = trimString(line);
        // Skip header row and empty lines
        if (lineNo == 1 || line == "") {
            continue;
        }

        int fields = splitString(line, ',', parts, 5);
        if (fields != 3) {
            continue;
        }

        int id = parseInt(parts[0]);
        string name = parts[1];
        int serving = parseInt(parts[2]);

        if (id == -1 || serving == -1 || name == "") {
            continue;
        }

        if (sys.addStation(id, name, serving) != -1) {
            loaded++;
        }
    }
    file.close();
    return loaded;
}

// ---------------------------------------------------------------------
// Load buses from buses.csv
// Format: bus_no,registration,departure,driver,type,stop_count,stops
// ---------------------------------------------------------------------
int loadBuses(string path) {
    ifstream file(path.c_str());
    if (!file.is_open()) {
        return -1;
    }

    string line;
    int lineNo = 0;
    int loaded = 0;
    string parts[10];

    while (getline(file, line)) {
        lineNo++;
        line = trimString(line);
        // Skip header row and empty lines
        if (lineNo == 1 || line == "") {
            continue;
        }

        int fields = splitString(line, ',', parts, 10);
        if (fields < 7) {
            continue;
        }

        int busNo = parseInt(parts[0]);
        string registration = parts[1];
        int departure = parseTime(parts[2]);
        string driver = parts[3];
        char type = parts[4][0];
        int stopCountExpected = parseInt(parts[5]);
        string stopsField = parts[6];

        if (busNo == -1 || departure == -1 || (type != 'S' && type != 'M')) {
            continue;
        }

        // Split stops separated by ';'
        string stopNames[MAX_STOPS];
        int actualStops = splitString(stopsField, ';', stopNames, MAX_STOPS);

        if (actualStops != stopCountExpected) {
            // Stop count mismatch
        }

        int stopIndexes[MAX_STOPS];
        bool allStopsFound = true;
        for (int i = 0; i < actualStops; i++) {
            int stIdx = sys.findStation(stopNames[i]);
            if (stIdx == -1) {
                allStopsFound = false;
                break;
            }
            stopIndexes[i] = stIdx;
        }

        if (!allStopsFound) {
            continue;
        }

        if (sys.addBus(busNo, registration, departure, driver, type, stopIndexes, actualStops)) {
            loaded++;
        }
    }
    file.close();
    return loaded;
}

// ---------------------------------------------------------------------
// Load travel time overrides from edges.csv
// Format: from_station,to_station,minutes
// ---------------------------------------------------------------------
int loadEdges(string path) {
    ifstream file(path.c_str());
    if (!file.is_open()) {
        return 0; // Optional file
    }

    string line;
    int lineNo = 0;
    int loaded = 0;
    string parts[5];

    while (getline(file, line)) {
        lineNo++;
        line = trimString(line);
        if (lineNo == 1 || line == "") {
            continue;
        }

        int fields = splitString(line, ',', parts, 5);
        if (fields != 3) {
            continue;
        }

        int fromIdx = sys.findStation(parts[0]);
        int toIdx = sys.findStation(parts[1]);
        int minutes = parseInt(parts[2]);

        if (fromIdx == -1 || toIdx == -1 || minutes <= 0) {
            continue;
        }

        if (sys.addEdgeOverride(fromIdx, toIdx, minutes)) {
            loaded++;
        }
    }
    file.close();
    return loaded;
}

// ---------------------------------------------------------------------
// Master loader: loads stations, buses, edges and finalizes system setup
// ---------------------------------------------------------------------
bool loadAllData(string folder) {
    int stationCount = loadStations(folder + "stations.csv");
    if (stationCount <= 0) {
        cout << "Error: Could not load stations from " << folder << "stations.csv" << endl;
        return false;
    }

    int busCount = loadBuses(folder + "buses.csv");
    if (busCount <= 0) {
        cout << "Error: Could not load buses from " << folder << "buses.csv" << endl;
        return false;
    }

    int edgeCount = loadEdges(folder + "edges.csv");

    // Sort buses by number so Binary Search works
    sys.sortBusesByNumber();

    // Map stations to the buses serving them
    sys.buildStationBusLists();

    cout << "Loaded " << stationCount << " stations, "
         << busCount << " buses, "
         << edgeCount << " edge overrides." << endl;
    return true;
}

// Next phase stub
int verifyRouteStops(string path) {
    (void)path;
    return NOT_IMPLEMENTED;
}
