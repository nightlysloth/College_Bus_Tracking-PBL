#ifndef MODELS_H
#define MODELS_H

#include <string>
#include "DataStructures.h"
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

string intToString(int number);
string formatTime(int minutes);
int parseTime(string text);
int parseInt(string text);
string statusName(int status);
void insertionSortByKey(int items[], int keys[], int n);

class BusSystem {
public:
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

    BusSystem();

    int addStation(int id, string name, int busesServing);
    bool addBus(int busNo, string registration, int departure, string driver, char type, int stopIndexes[], int stopCount);
    bool addEdgeOverride(int fromStation, int toStation, int minutes);
    void sortBusesByNumber();
    void buildStationBusLists();

    int findStation(string name);
    int suggestStations(string prefix, int results[], int maxResults);
    int findBus(int busNo);
    int stopPositionOf(int busIdx, int stationIdx);

    int edgeMinutes(int fromStation, int toStation);
    int cumulativeTime(int busIdx, int stopPos);
    int getStatus(int busIdx);
    int currentStopIndex(int busIdx);
    int etaAt(int busIdx, int stationIdx);
    bool hasPassed(int busIdx, int stationIdx);

    int seatsOccupied(int busIdx);
    int seatsFree(int busIdx);
    string seatStatus(int busIdx);

    IntQueue buildArrivalBoard(int stationIdx);
    double scoreBus(int busIdx, int stationIdx);
    Recommendation recommend(int stationIdx);

    void advanceClock(int minutes);
    void setClock(int minutes);
    bool undoClock();
};

extern BusSystem sys;

#endif
