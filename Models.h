// =====================================================================
// File: Models.h
// Purpose: Core data models for College Bus Tracking System:
//          Station, Bus, Recommendation, Graph, and BusSystem.
//
// All methods use simple call-by-value (no reference parameters).
// No friend functions, no static methods/variables.
// =====================================================================
#ifndef MODELS_H
#define MODELS_H

#include <string>
#include "DataStructures.h"
using namespace std;

// ---------------------------- Constants ------------------------------
const int MAX_STATIONS = 200;          // max number of stations
const int MAX_BUSES = 100;             // max number of buses
const int MAX_STOPS = 30;              // max stops in one route
const int MAX_OVERRIDES = 50;          // travel time overrides
const int DEFAULT_EDGE_MINUTES = 4;    // default 4 minutes between consecutive stops
const int STANDARD_CAPACITY = 50;      // standard bus seats
const int MINI_CAPACITY = 32;          // mini bus seats
const int DEFAULT_START_TIME = 7 * 60; // 07:00 AM (420 minutes from midnight)
const int NOT_IMPLEMENTED = -999;      // stub return value

// Bus states
enum BusStatus { NOT_STARTED = 0, RUNNING = 1, REACHED = 2 };

// ---------------------------- Plain Data Structs ---------------------
struct Station {
    int id;            // station id (1 to 93)
    string name;       // station name (e.g. "Kargi Chowk")
    int busesServing;  // total buses that pass this station
};

struct Bus {
    int busNo;                 // bus number (e.g. 1, 2, 3...)
    string registration;       // vehicle registration number (e.g. "UK07PA1688")
    string driver;             // driver name
    int departure;             // departure time in minutes from midnight
    char type;                 // 'S' = standard, 'M' = mini
    int capacity;              // 50 or 32
    int stopCount;             // number of stops on this route
    int stops[MAX_STOPS];      // station indexes along the route in travel order
};

struct EdgeOverride {
    int fromStation;   // start station index
    int toStation;     // end station index
    int minutes;       // travel time in minutes
};

// Result for bus recommendation
struct Recommendation {
    int bestBus;        // index of best bus (-1 if none)
    int secondBus;      // index of second best bus (-1 if none)
    int eligibleCount;  // total eligible buses
};

// ---------------------------- Helper Functions -----------------------
string intToString(int number);
string formatTime(int minutes);
int parseTime(string text);       // returns minutes since midnight, or -1 if invalid
int parseInt(string text);         // returns positive number, or -1 if invalid
string statusName(int status);     // returns "Not Started", "Running", or "Reached"
void insertionSortByKey(int items[], int keys[], int n);

// ---------------------------------------------------------------------
// Graph Skeleton (for next phase)
// ---------------------------------------------------------------------
class Graph {
public:
    int vertexCount;
    IntList adjTo[MAX_STATIONS];
    IntList adjWeight[MAX_STATIONS];

    Graph();
    void buildFromRoutes();
    int shortestTime(int src, int dst, IntStack path);
    int fewestStops(int src, int dst, IntStack path);
    int reachableFrom(int src, IntList reachable);
};

// ---------------------------------------------------------------------
// BusSystem: owns all stations, buses, overrides, and simulated clock
// ---------------------------------------------------------------------
class BusSystem {
public:
    Station stations[MAX_STATIONS];
    int stationCount;

    Bus buses[MAX_BUSES];
    int busCount;

    EdgeOverride overrides[MAX_OVERRIDES];
    int overrideCount;

    HashTable stationIndex;               // maps station name -> station index
    IntList busesAtStation[MAX_STATIONS]; // bus indexes stopping at each station
    Graph graph;                          // graph for shortest path

    int now;                              // current time in minutes from midnight
    IntStack clockHistory;                // stack storing previous clock times for undo

    BusSystem();

    // Adding data
    int addStation(int id, string name, int busesServing);
    bool addBus(int busNo, string registration, int departure, string driver, char type, int stopIndexes[], int stopCount);
    bool addEdgeOverride(int fromStation, int toStation, int minutes);
    void sortBusesByNumber();
    void buildStationBusLists();

    // Searching
    int findStation(string name);
    int suggestStations(string prefix, int results[], int maxResults);
    int findBus(int busNo);
    int stopPositionOf(int busIdx, int stationIdx);

    // Simulation
    int edgeMinutes(int fromStation, int toStation);
    int cumulativeTime(int busIdx, int stopPos);
    int getStatus(int busIdx);
    int currentStopIndex(int busIdx);
    int etaAt(int busIdx, int stationIdx);
    bool hasPassed(int busIdx, int stationIdx);

    // Seats
    int seatsOccupied(int busIdx);
    int seatsFree(int busIdx);
    string seatStatus(int busIdx);

    // Arrival board & recommendation
    IntQueue buildArrivalBoard(int stationIdx);
    double scoreBus(int busIdx, int stationIdx);
    Recommendation recommend(int stationIdx);

    // Clock
    void advanceClock(int minutes);
    void setClock(int minutes);
    bool undoClock();

    // Next phase stubs
    int findBusesBetween(int fromStation, int toStation, IntList result);
    int sortBuses(int order[], int sortBy, int algorithm);
    int getStatusCounts();
    int busiestStations(int result[], int topN);
    bool saveReport(int stationIdx, string fileName);
};

// Global system instance accessible across the application
extern BusSystem sys;

#endif
