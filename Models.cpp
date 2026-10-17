// =====================================================================
// File: Models.cpp
// Purpose: Implementations of BusSystem and helper utilities.
//
// Simple, step-by-step logic designed to be easy to understand and explain.
// All functions use call-by-value.
// =====================================================================
#include "Models.h"
#include <iostream>

// Global instance of the BusSystem
BusSystem sys;

// ---------------------------------------------------------------------
// HELPER FUNCTIONS
// ---------------------------------------------------------------------

// Convert integer to string without using any fancy libraries
string intToString(int number) {
    if (number == 0) {
        return "0";
    }
    bool isNegative = false;
    if (number < 0) {
        isNegative = true;
        number = -number;
    }
    string result = "";
    while (number > 0) {
        char digit = '0' + (number % 10);
        result = digit + result;
        number = number / 10;
    }
    if (isNegative) {
        result = "-" + result;
    }
    return result;
}

// Convert minutes since midnight (e.g. 455) to "07:35"
string formatTime(int minutes) {
    int hours = minutes / 60;
    int mins = minutes % 60;
    string result = "";
    if (hours < 10) {
        result += "0";
    }
    result += intToString(hours) + ":";
    if (mins < 10) {
        result += "0";
    }
    result += intToString(mins);
    return result;
}

// Convert string of digits to integer, returns -1 if invalid
int parseInt(string text) {
    if (text.length() == 0 || text.length() > 9) {
        return -1;
    }
    int result = 0;
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] < '0' || text[i] > '9') {
            return -1;
        }
        result = result * 10 + (text[i] - '0');
    }
    return result;
}

// Parse "HH:MM" format into minutes since midnight, returns -1 if invalid
int parseTime(string text) {
    int colonPos = -1;
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] == ':') {
            colonPos = i;
            break;
        }
    }
    if (colonPos < 1 || colonPos > 2) {
        return -1;
    }
    string hourPart = text.substr(0, colonPos);
    string minPart = text.substr(colonPos + 1);

    if (minPart.length() != 2) {
        return -1;
    }

    int h = parseInt(hourPart);
    int m = parseInt(minPart);

    if (h < 0 || h > 23 || m < 0 || m > 59) {
        return -1;
    }
    return h * 60 + m;
}

// Human-readable status name
string statusName(int status) {
    if (status == NOT_STARTED) return "Not Started";
    if (status == RUNNING) return "Running";
    return "Reached";
}

// Standard Insertion Sort on two parallel arrays
void insertionSortByKey(int items[], int keys[], int n) {
    for (int i = 1; i < n; i++) {
        int currentKey = keys[i];
        int currentItem = items[i];
        int j = i - 1;
        while (j >= 0 && keys[j] > currentKey) {
            keys[j + 1] = keys[j];
            items[j + 1] = items[j];
            j--;
        }
        keys[j + 1] = currentKey;
        items[j + 1] = currentItem;
    }
}

// ---------------------------------------------------------------------
// BUS SYSTEM CONSTRUCTOR & DATA LOADING
// ---------------------------------------------------------------------
BusSystem::BusSystem() {
    stationCount = 0;
    busCount = 0;
    overrideCount = 0;
    now = DEFAULT_START_TIME; // 07:00 AM
}

// Add a station from stations.csv
int BusSystem::addStation(int id, string name, int busesServing) {
    if (stationCount >= MAX_STATIONS) {
        return -1;
    }
    // Check if station already exists
    if (stationIndex.get(name) != -1) {
        return -1;
    }
    stations[stationCount].id = id;
    stations[stationCount].name = name;
    stations[stationCount].busesServing = busesServing;

    // Save in hash table for fast lookup by name
    stationIndex.insert(name, stationCount);

    int newIndex = stationCount;
    stationCount++;
    return newIndex;
}

// Add a bus from buses.csv
bool BusSystem::addBus(int busNo, string registration, int departure,
                       string driver, char type, int stopIndexes[], int stopCount) {
    if (busCount >= MAX_BUSES || stopCount < 1 || stopCount > MAX_STOPS) {
        return false;
    }
    // Check for duplicate bus number
    for (int i = 0; i < busCount; i++) {
        if (buses[i].busNo == busNo) {
            return false;
        }
    }
    buses[busCount].busNo = busNo;
    buses[busCount].registration = registration;
    buses[busCount].departure = departure;
    buses[busCount].driver = driver;
    buses[busCount].type = type;

    if (type == 'M') {
        buses[busCount].capacity = MINI_CAPACITY;
    } else {
        buses[busCount].capacity = STANDARD_CAPACITY;
    }

    buses[busCount].stopCount = stopCount;
    for (int i = 0; i < stopCount; i++) {
        buses[busCount].stops[i] = stopIndexes[i];
    }
    busCount++;
    return true;
}

// Add travel time override from edges.csv
bool BusSystem::addEdgeOverride(int fromStation, int toStation, int minutes) {
    if (overrideCount >= MAX_OVERRIDES) {
        return false;
    }
    overrides[overrideCount].fromStation = fromStation;
    overrides[overrideCount].toStation = toStation;
    overrides[overrideCount].minutes = minutes;
    overrideCount++;
    return true;
}

// Insertion Sort to keep buses sorted by bus number (needed for binary search)
void BusSystem::sortBusesByNumber() {
    for (int i = 1; i < busCount; i++) {
        Bus keyBus = buses[i];
        int j = i - 1;
        while (j >= 0 && buses[j].busNo > keyBus.busNo) {
            buses[j + 1] = buses[j];
            j--;
        }
        buses[j + 1] = keyBus;
    }
}

// For each station, build the list of bus indexes that stop at it
void BusSystem::buildStationBusLists() {
    for (int s = 0; s < stationCount; s++) {
        busesAtStation[s].clear();
    }
    for (int b = 0; b < busCount; b++) {
        for (int k = 0; k < buses[b].stopCount; k++) {
            int stIdx = buses[b].stops[k];
            busesAtStation[stIdx].pushBack(b);
        }
    }
}

// ---------------------------------------------------------------------
// SEARCHING
// ---------------------------------------------------------------------

// Hash table lookup: returns station index or -1
int BusSystem::findStation(string name) {
    return stationIndex.get(name);
}

// Linear search: finds stations starting with prefix (case-insensitive)
int BusSystem::suggestStations(string prefix, int results[], int maxResults) {
    string lowerPrefix = toLowerCase(prefix);
    int found = 0;
    for (int i = 0; i < stationCount && found < maxResults; i++) {
        string lowerName = toLowerCase(stations[i].name);
        if (lowerName.length() >= lowerPrefix.length()) {
            if (lowerName.substr(0, lowerPrefix.length()) == lowerPrefix) {
                results[found] = i;
                found++;
            }
        }
    }
    return found;
}

// Binary search on sorted buses array: returns bus index or -1
int BusSystem::findBus(int busNo) {
    int low = 0;
    int high = busCount - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (buses[mid].busNo == busNo) {
            return mid;
        } else if (buses[mid].busNo < busNo) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

// Linear search in a bus's route to find which stop number it is
int BusSystem::stopPositionOf(int busIdx, int stationIdx) {
    for (int i = 0; i < buses[busIdx].stopCount; i++) {
        if (buses[busIdx].stops[i] == stationIdx) {
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------
// SIMULATION: TIME, POSITION, ETA
// ---------------------------------------------------------------------

// Travel time in minutes between consecutive stations
int BusSystem::edgeMinutes(int fromStation, int toStation) {
    for (int i = 0; i < overrideCount; i++) {
        if (overrides[i].fromStation == fromStation && overrides[i].toStation == toStation) {
            return overrides[i].minutes;
        }
    }
    return DEFAULT_EDGE_MINUTES; // 4 minutes default
}

// Minutes elapsed from first stop to stop number stopPos
int BusSystem::cumulativeTime(int busIdx, int stopPos) {
    int total = 0;
    for (int k = 1; k <= stopPos; k++) {
        int stA = buses[busIdx].stops[k - 1];
        int stB = buses[busIdx].stops[k];
        total += edgeMinutes(stA, stB);
    }
    return total;
}

// Status of bus: NOT_STARTED, RUNNING, or REACHED
int BusSystem::getStatus(int busIdx) {
    if (now < buses[busIdx].departure) {
        return NOT_STARTED;
    }
    int elapsed = now - buses[busIdx].departure;
    int totalTripTime = cumulativeTime(busIdx, buses[busIdx].stopCount - 1);
    if (elapsed >= totalTripTime) {
        return REACHED;
    }
    return RUNNING;
}

// Index of current stop the bus is at or just left
int BusSystem::currentStopIndex(int busIdx) {
    if (now < buses[busIdx].departure) {
        return 0;
    }
    int elapsed = now - buses[busIdx].departure;
    int index = 0;
    for (int i = 1; i < buses[busIdx].stopCount; i++) {
        if (cumulativeTime(busIdx, i) <= elapsed) {
            index = i;
        } else {
            break;
        }
    }
    return index;
}

// Estimated arrival time (minutes from midnight) at given station
int BusSystem::etaAt(int busIdx, int stationIdx) {
    int pos = stopPositionOf(busIdx, stationIdx);
    if (pos == -1) {
        return -1;
    }
    return buses[busIdx].departure + cumulativeTime(busIdx, pos);
}

// Has the bus already departed from this station?
bool BusSystem::hasPassed(int busIdx, int stationIdx) {
    int eta = etaAt(busIdx, stationIdx);
    if (eta == -1) {
        return false;
    }
    return eta < now;
}

// ---------------------------------------------------------------------
// SIMULATION: SEATS
// ---------------------------------------------------------------------

// Occupied seats formula from PRD:
// occupied = 5 + (busNo * 13) % 10 + 3 * stopsTravelled
int BusSystem::seatsOccupied(int busIdx) {
    int stopsTravelled = 0;
    if (getStatus(busIdx) != NOT_STARTED) {
        stopsTravelled = currentStopIndex(busIdx);
    }
    int occupied = 5 + (buses[busIdx].busNo * 13) % 10 + 3 * stopsTravelled;
    if (occupied > buses[busIdx].capacity) {
        occupied = buses[busIdx].capacity;
    }
    return occupied;
}

// Free seats
int BusSystem::seatsFree(int busIdx) {
    return buses[busIdx].capacity - seatsOccupied(busIdx);
}

// Plenty (> 40% free), Filling (10% - 40% free), Full (< 10% free)
string BusSystem::seatStatus(int busIdx) {
    int freeCount = seatsFree(busIdx);
    int cap = buses[busIdx].capacity;
    if (freeCount * 100 > 40 * cap) {
        return "Plenty";
    }
    if (freeCount * 100 >= 10 * cap) {
        return "Filling";
    }
    return "Full";
}

// ---------------------------------------------------------------------
// ARRIVAL BOARD & RECOMMENDATION
// ---------------------------------------------------------------------

// Build queue of upcoming buses sorted by ETA (earliest first)
IntQueue BusSystem::buildArrivalBoard(int stationIdx) {
    IntQueue board;
    int busList[MAX_BUSES];
    int etaList[MAX_BUSES];
    int n = 0;

    for (int i = 0; i < busesAtStation[stationIdx].size(); i++) {
        int busIdx = busesAtStation[stationIdx].getAt(i);
        if (!hasPassed(busIdx, stationIdx)) {
            busList[n] = busIdx;
            etaList[n] = etaAt(busIdx, stationIdx);
            n++;
        }
    }

    insertionSortByKey(busList, etaList, n);

    for (int i = 0; i < n; i++) {
        board.enqueue(busList[i]);
    }
    return board;
}

// Score formula: waitMinutes + 0.5 * rideMinutes + seatPenalty (lower is better)
double BusSystem::scoreBus(int busIdx, int stationIdx) {
    int pos = stopPositionOf(busIdx, stationIdx);
    if (pos == -1 || hasPassed(busIdx, stationIdx)) {
        return -1.0;
    }
    string status = seatStatus(busIdx);
    if (status == "Full") {
        return -1.0;
    }
    int wait = etaAt(busIdx, stationIdx) - now;
    int ride = cumulativeTime(busIdx, buses[busIdx].stopCount - 1) - cumulativeTime(busIdx, pos);
    int penalty = 0;
    if (status == "Filling") {
        penalty = 5;
    }
    return wait + 0.5 * ride + penalty;
}

// Recommend best and second best bus
Recommendation BusSystem::recommend(int stationIdx) {
    Recommendation rec;
    rec.bestBus = -1;
    rec.secondBus = -1;
    rec.eligibleCount = 0;

    double bestScore = 999999;
    double secondScore = 999999;

    for (int i = 0; i < busesAtStation[stationIdx].size(); i++) {
        int busIdx = busesAtStation[stationIdx].getAt(i);
        double score = scoreBus(busIdx, stationIdx);
        if (score < 0) {
            continue;
        }
        rec.eligibleCount++;
        if (rec.bestBus == -1 || score < bestScore) {
            rec.secondBus = rec.bestBus;
            secondScore = bestScore;
            rec.bestBus = busIdx;
            bestScore = score;
        } else if (rec.secondBus == -1 || score < secondScore) {
            rec.secondBus = busIdx;
            secondScore = score;
        }
    }
    return rec;
}

// ---------------------------------------------------------------------
// CLOCK CONTROLS
// ---------------------------------------------------------------------
void BusSystem::advanceClock(int minutes) {
    clockHistory.push(now); // push to stack for undo
    now = now + minutes;
    if (now > 23 * 60 + 59) {
        now = 23 * 60 + 59;
    }
}

void BusSystem::setClock(int minutes) {
    clockHistory.push(now);
    now = minutes;
}

bool BusSystem::undoClock() {
    if (clockHistory.isEmpty()) {
        return false;
    }
    now = clockHistory.pop(); // pop previous time from stack
    return true;
}
