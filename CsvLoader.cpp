#include <iostream>
#include <fstream>
#include "CsvLoader.h"

bool isWhitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

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
        string stopsField = parts[6];

        if (busNo == -1 || departure == -1 || (type != 'S' && type != 'M')) {
            continue;
        }

        string stopNames[MAX_STOPS];
        int actualStops = splitString(stopsField, ';', stopNames, MAX_STOPS);

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

int loadEdges(string path) {
    ifstream file(path.c_str());
    if (!file.is_open()) {
        return 0;
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

    sys.sortBusesByNumber();
    sys.buildStationBusLists();

    cout << "Loaded " << stationCount << " stations, "
         << busCount << " buses, "
         << edgeCount << " edge overrides." << endl;
    return true;
}
