#pragma once
#include <string>
#include "DataStructures.cpp"
using namespace std;

const int MAX_STATIONS = 200;
const int MAX_BUSES = 100;
const int MAX_STOPS = 30;
const int MAX_OVERRIDES = 50;
const int DEFAULT_EDGE_MINUTES = 4;
const int STANDARD_CAPACITY = 50;
const int MINI_CAPACITY = 32;
const int DEFAULT_START_TIME = 7 * 60;

enum BusStatus { NOT_STARTED = 0, RUNNING = 1, REACHED = 2 };

struct Station {
    int id;
    string name;
    int busesServing;
};

struct Bus {
    int busNo;
    string registration;
    string driver;
    int departure;
    char type;
    int capacity;
    int stopCount;
    int stops[MAX_STOPS];
};

struct EdgeOverride {
    int fromStation;
    int toStation;
    int minutes;
};

struct Recommendation {
    int bestBus;
    int secondBus;
    int eligibleCount;
};

inline string intToString(int number) {
    if (number == 0) return "0";
    bool neg = number < 0;
    if (neg) number = -number;
    string res = "";
    while (number > 0) {
        res = (char)('0' + (number % 10)) + res;
        number /= 10;
    }
    return neg ? "-" + res : res;
}

inline string formatTime(int minutes) {
    int h = minutes / 60, m = minutes % 60;
    return (h < 10 ? "0" : "") + intToString(h) + ":" + (m < 10 ? "0" : "") + intToString(m);
}

inline int parseInt(string text) {
    if (text.empty() || text.length() > 9) return -1;
    int res = 0;
    for (char c : text) {
        if (c < '0' || c > '9') return -1;
        res = res * 10 + (c - '0');
    }
    return res;
}

inline int parseTime(string text) {
    int colon = text.find(':');
    if (colon == (int)string::npos || colon < 1 || colon > 2) return -1;
    string hp = text.substr(0, colon), mp = text.substr(colon + 1);
    if (mp.length() != 2) return -1;
    int h = parseInt(hp), m = parseInt(mp);
    if (h < 0 || h > 23 || m < 0 || m > 59) return -1;
    return h * 60 + m;
}

inline string statusName(int status) {
    if (status == NOT_STARTED) return "Not Started";
    if (status == RUNNING) return "Running";
    return "Reached";
}

inline void insertionSortByKey(int items[], int keys[], int n) {
    for (int i = 1; i < n; i++) {
        int ck = keys[i], ci = items[i], j = i - 1;
        while (j >= 0 && keys[j] > ck) {
            keys[j + 1] = keys[j];
            items[j + 1] = items[j];
            j--;
        }
        keys[j + 1] = ck;
        items[j + 1] = ci;
    }
}

struct BusSystem {
    Station stations[MAX_STATIONS];
    int stationCount;
    Bus buses[MAX_BUSES];
    int busCount;
    EdgeOverride overrides[MAX_OVERRIDES];
    int overrideCount;
    HashTable stationIndex;
    IntList busesAtStation[MAX_STATIONS];
    int now;
    IntStack clockHistory;

    BusSystem() {
        stationCount = busCount = overrideCount = 0;
        now = DEFAULT_START_TIME;
    }

    int addStation(int id, string name, int busesServing) {
        if (stationCount >= MAX_STATIONS || stationIndex.get(name) != -1) return -1;
        stations[stationCount] = {id, name, busesServing};
        stationIndex.insert(name, stationCount);
        return stationCount++;
    }

    bool addBus(int busNo, string registration, int departure, string driver, char type, int stopIndexes[], int stopCount) {
        if (busCount >= MAX_BUSES || stopCount < 1 || stopCount > MAX_STOPS) return false;
        for (int i = 0; i < busCount; i++) {
            if (buses[i].busNo == busNo) return false;
        }
        buses[busCount].busNo = busNo;
        buses[busCount].registration = registration;
        buses[busCount].departure = departure;
        buses[busCount].driver = driver;
        buses[busCount].type = type;
        buses[busCount].capacity = (type == 'M' ? MINI_CAPACITY : STANDARD_CAPACITY);
        buses[busCount].stopCount = stopCount;
        for (int i = 0; i < stopCount; i++) buses[busCount].stops[i] = stopIndexes[i];
        busCount++;
        return true;
    }

    bool addEdgeOverride(int fromStation, int toStation, int minutes) {
        if (overrideCount >= MAX_OVERRIDES) return false;
        overrides[overrideCount++] = {fromStation, toStation, minutes};
        return true;
    }

    void sortBusesByNumber() {
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

    void buildStationBusLists() {
        for (int s = 0; s < stationCount; s++) busesAtStation[s].clear();
        for (int b = 0; b < busCount; b++) {
            for (int k = 0; k < buses[b].stopCount; k++) {
                busesAtStation[buses[b].stops[k]].pushBack(b);
            }
        }
    }

    int findStation(string name) {
        return stationIndex.get(name);
    }

    int suggestStations(string prefix, int results[], int maxResults) {
        string lp = toLowerCase(prefix);
        int found = 0;
        for (int i = 0; i < stationCount && found < maxResults; i++) {
            string ln = toLowerCase(stations[i].name);
            if (ln.length() >= lp.length() && ln.substr(0, lp.length()) == lp) {
                results[found++] = i;
            }
        }
        return found;
    }

    int findBus(int busNo) {
        int low = 0, high = busCount - 1;
        while (low <= high) {
            int mid = (low + high) / 2;
            if (buses[mid].busNo == busNo) return mid;
            if (buses[mid].busNo < busNo) low = mid + 1;
            else high = mid - 1;
        }
        return -1;
    }

    int stopPositionOf(int busIdx, int stationIdx) {
        for (int i = 0; i < buses[busIdx].stopCount; i++) {
            if (buses[busIdx].stops[i] == stationIdx) return i;
        }
        return -1;
    }

    int edgeMinutes(int fromStation, int toStation) {
        for (int i = 0; i < overrideCount; i++) {
            if (overrides[i].fromStation == fromStation && overrides[i].toStation == toStation) {
                return overrides[i].minutes;
            }
        }
        return DEFAULT_EDGE_MINUTES;
    }

    int cumulativeTime(int busIdx, int stopPos) {
        int total = 0;
        for (int k = 1; k <= stopPos; k++) {
            total += edgeMinutes(buses[busIdx].stops[k - 1], buses[busIdx].stops[k]);
        }
        return total;
    }

    int getStatus(int busIdx) {
        if (now < buses[busIdx].departure) return NOT_STARTED;
        int elapsed = now - buses[busIdx].departure;
        int totalTripTime = cumulativeTime(busIdx, buses[busIdx].stopCount - 1);
        return elapsed >= totalTripTime ? REACHED : RUNNING;
    }

    int currentStopIndex(int busIdx) {
        if (now < buses[busIdx].departure) return 0;
        int elapsed = now - buses[busIdx].departure, idx = 0;
        for (int i = 1; i < buses[busIdx].stopCount; i++) {
            if (cumulativeTime(busIdx, i) <= elapsed) idx = i;
            else break;
        }
        return idx;
    }

    int etaAt(int busIdx, int stationIdx) {
        int pos = stopPositionOf(busIdx, stationIdx);
        return pos == -1 ? -1 : buses[busIdx].departure + cumulativeTime(busIdx, pos);
    }

    bool hasPassed(int busIdx, int stationIdx) {
        int eta = etaAt(busIdx, stationIdx);
        return eta != -1 && eta < now;
    }

    int seatsOccupied(int busIdx) {
        if (now < buses[busIdx].departure) return 0;
        int stops = currentStopIndex(busIdx);
        if (stops == 0) return 0;
        int occ = 5 + (buses[busIdx].busNo * 13) % 10 + 3 * stops;
        return occ > buses[busIdx].capacity ? buses[busIdx].capacity : occ;
    }

    int seatsFree(int busIdx) {
        return buses[busIdx].capacity - seatsOccupied(busIdx);
    }

    string seatStatus(int busIdx) {
        int freeCount = seatsFree(busIdx), cap = buses[busIdx].capacity;
        if (freeCount * 100 > 40 * cap) return "Plenty";
        if (freeCount * 100 >= 10 * cap) return "Filling";
        return "Full";
    }

    IntQueue buildArrivalBoard(int stationIdx) {
        IntQueue board;
        int busList[MAX_BUSES], etaList[MAX_BUSES], n = 0;
        for (int i = 0; i < busesAtStation[stationIdx].size(); i++) {
            int b = busesAtStation[stationIdx].getAt(i);
            if (!hasPassed(b, stationIdx)) {
                busList[n] = b;
                etaList[n] = etaAt(b, stationIdx);
                n++;
            }
        }
        insertionSortByKey(busList, etaList, n);
        for (int i = 0; i < n; i++) board.enqueue(busList[i]);
        return board;
    }

    double scoreBus(int busIdx, int stationIdx) {
        int pos = stopPositionOf(busIdx, stationIdx);
        if (pos == -1 || hasPassed(busIdx, stationIdx)) return -1.0;
        string st = seatStatus(busIdx);
        if (st == "Full") return -1.0;
        int wait = etaAt(busIdx, stationIdx) - now;
        int ride = cumulativeTime(busIdx, buses[busIdx].stopCount - 1) - cumulativeTime(busIdx, pos);
        return wait + 0.5 * ride + (st == "Filling" ? 5 : 0);
    }

    Recommendation recommend(int stationIdx) {
        Recommendation rec = {-1, -1, 0};
        double bestScore = 999999, secondScore = 999999;
        for (int i = 0; i < busesAtStation[stationIdx].size(); i++) {
            int b = busesAtStation[stationIdx].getAt(i);
            double s = scoreBus(b, stationIdx);
            if (s < 0) continue;
            rec.eligibleCount++;
            if (rec.bestBus == -1 || s < bestScore) {
                rec.secondBus = rec.bestBus; secondScore = bestScore;
                rec.bestBus = b; bestScore = s;
            } else if (rec.secondBus == -1 || s < secondScore) {
                rec.secondBus = b; secondScore = s;
            }
        }
        return rec;
    }

    void advanceClock(int minutes) {
        clockHistory.push(now);
        now += minutes;
        if (now > 1439) now = 1439;
    }

    void setClock(int minutes) {
        clockHistory.push(now);
        now = minutes;
    }

    bool undoClock() {
        if (clockHistory.isEmpty()) return false;
        now = clockHistory.pop();
        return true;
    }
};

inline BusSystem& getSystem() {
    static BusSystem instance;
    return instance;
}
#define sys (getSystem())
