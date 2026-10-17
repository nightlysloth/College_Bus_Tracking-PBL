// =====================================================================
// File: main.cpp
// Purpose: Interactive terminal menu for College Bus Tracking System.
//
// Simple, beginner-friendly C++ code.
// No friend functions, no static variables/functions, no reference parameters.
// All functions use call-by-value and access the global 'sys' object.
//
// Compile:  g++ *.cpp -o main
// Run:      ./main data/
// =====================================================================
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include "DataStructures.h"
#include "Models.h"
#include "CsvLoader.h"
using namespace std;

// ---------------------------------------------------------------------
// Basic Input Helpers (Call by value)
// ---------------------------------------------------------------------

// Prints a prompt and reads one full line of input
string readLine(string prompt) {
    cout << prompt;
    string line = "";
    if (!getline(cin, line)) {
        cout << endl << "Input closed. Goodbye!" << endl;
        exit(0);
    }
    return trimString(line);
}

// Reads an integer number from the user, returns -1 if invalid
int readNumber(string prompt) {
    string line = readLine(prompt);
    int value = parseInt(line);
    return value;
}

// Asks the user for a station name.
// Returns the station index or -1 if not found.
// Shows up to 5 prefix suggestions if not found.
int askForStation(string prompt) {
    string name = readLine(prompt);
    if (name == "") {
        cout << "No name entered." << endl;
        return -1;
    }

    // Exact search using Hash Table
    int index = sys.findStation(name);
    if (index != -1) {
        return index;
    }

    cout << "Station '" << name << "' not found." << endl;

    // Linear search for prefix suggestions
    int suggestions[5];
    int found = sys.suggestStations(name, suggestions, 5);
    if (found > 0) {
        cout << "Did you mean:" << endl;
        for (int i = 0; i < found; i++) {
            cout << "   - " << sys.stations[suggestions[i]].name << endl;
        }
    }
    return -1;
}

// Prints a path stored in an IntStack: A -> B -> C
void printPath(IntStack path) {
    cout << "Path: ";
    bool isFirst = true;
    while (!path.isEmpty()) {
        if (!isFirst) {
            cout << " -> ";
        }
        int stIdx = path.pop();
        cout << sys.stations[stIdx].name;
        isFirst = false;
    }
    cout << endl;
}

// Prints message for features reserved for the next phase
void printNotReady(string what, string step) {
    cout << "[TODO] " << what << " is not implemented yet (next phase, " << step << ")." << endl;
}

// ---------------------------------------------------------------------
// Feature 1: Search Station (Exact lookup + Prefix suggestions)
// ---------------------------------------------------------------------
void featureSearchStation() {
    int s = askForStation("Enter station name (full or first letters): ");
    if (s == -1) {
        return;
    }

    cout << endl << "Station #" << sys.stations[s].id << ": " << sys.stations[s].name << endl;

    if (sys.busesAtStation[s].isEmpty()) {
        cout << "No bus serves this station." << endl;
        return;
    }

    int totalServing = sys.busesAtStation[s].size();
    cout << "Buses serving it (" << totalServing << "): ";
    for (int i = 0; i < totalServing; i++) {
        int busIdx = sys.busesAtStation[s].getAt(i);
        cout << sys.buses[busIdx].busNo;
        if (i < totalServing - 1) {
            cout << ", ";
        }
    }
    cout << endl;
}

// ---------------------------------------------------------------------
// Feature 2: Search Bus by Bus Number (Binary Search)
// ---------------------------------------------------------------------
void featureSearchBus() {
    int busNo = readNumber("Enter bus number: ");
    if (busNo == -1) {
        cout << "Please type a valid number." << endl;
        return;
    }

    int b = sys.findBus(busNo); // Binary Search
    if (b == -1) {
        cout << "Bus " << busNo << " not found." << endl;
        return;
    }

    Bus bus = sys.buses[b];
    int status = sys.getStatus(b);
    int cur = sys.currentStopIndex(b);

    cout << endl << "Bus " << bus.busNo << endl;
    cout << "  Registration : " << bus.registration << endl;
    cout << "  Driver       : " << bus.driver << endl;
    cout << "  Type         : " << (bus.type == 'S' ? "Standard Bus" : "Mini Bus")
         << " (" << bus.capacity << " seats)" << endl;
    cout << "  Departure    : " << formatTime(bus.departure) << " from " << sys.stations[bus.stops[0]].name << endl;
    cout << "  Status       : " << statusName(status) << "  (time now " << formatTime(sys.now) << ")" << endl;

    if (status == NOT_STARTED) {
        cout << "  Current stop : not started yet (will begin at " << sys.stations[bus.stops[0]].name << ")" << endl;
    } else {
        cout << "  Current stop : " << sys.stations[bus.stops[cur]].name << endl;
    }

    if (status == REACHED) {
        cout << "  Next stop    : - (journey finished)" << endl;
    } else if (status == NOT_STARTED) {
        cout << "  Next stop    : " << sys.stations[bus.stops[0]].name << endl;
    } else {
        cout << "  Next stop    : " << sys.stations[bus.stops[cur + 1]].name << endl;
    }

    cout << "  Stops left   : " << (bus.stopCount - 1 - cur) << endl;
    cout << "  Seats        : " << sys.seatsFree(b) << " free / " << bus.capacity
         << " (" << sys.seatStatus(b) << ")" << endl;
}

// ---------------------------------------------------------------------
// Feature 3: Route Search
// ---------------------------------------------------------------------
void featureSearchRoute() {
    cout << " 1. Route of a bus number" << endl;
    cout << " 2. Buses from station A to station B" << endl;
    int choice = readNumber("Choice: ");

    if (choice == 1) {
        int busNo = readNumber("Enter bus number: ");
        int b = sys.findBus(busNo);
        if (b == -1) {
            cout << "Bus not found." << endl;
            return;
        }

        Bus bus = sys.buses[b];
        int status = sys.getStatus(b);
        int cur = sys.currentStopIndex(b);

        cout << endl << "Route of bus " << bus.busNo << " (" << statusName(status) << ")" << endl;
        cout << left << setw(5) << "#" << setw(24) << "Station" << "Scheduled" << endl;

        for (int i = 0; i < bus.stopCount; i++) {
            int scheduledTime = bus.departure + sys.cumulativeTime(b, i);
            cout << left << setw(5) << (i + 1)
                 << setw(24) << sys.stations[bus.stops[i]].name
                 << formatTime(scheduledTime);

            if (status == RUNNING && i == cur) {
                cout << "   <-- bus is here";
            }
            cout << endl;
        }
        cout << "Total travel time: " << sys.cumulativeTime(b, bus.stopCount - 1) << " min" << endl;

    } else if (choice == 2) {
        int from = askForStation("From station: ");
        if (from == -1) return;
        int to = askForStation("To station: ");
        if (to == -1) return;

        printNotReady("Route search between two stations", "Step 1");
    } else {
        cout << "Invalid choice." << endl;
    }
}

// ---------------------------------------------------------------------
// Feature 4: Arrival Board (Using Queue & Earliest ETA first)
// ---------------------------------------------------------------------
void printArrivalBoard(int s) {
    cout << endl << "Station: " << sys.stations[s].name << "      Time: " << formatTime(sys.now) << endl;

    if (sys.busesAtStation[s].isEmpty()) {
        cout << "No bus serves this station." << endl;
        return;
    }

    IntQueue board = sys.buildArrivalBoard(s);
    if (board.isEmpty()) {
        cout << "No more buses today: every bus has already passed." << endl;
        return;
    }

    cout << left << setw(4) << "#" << setw(6) << "Bus" << setw(10) << "Type" << setw(8) << "ETA"
         << setw(9) << "In" << setw(18) << "Seats(free/cap)" << "Status" << endl;
    cout << "----------------------------------------------------------------" << endl;

    int rank = 1;
    while (!board.isEmpty()) {
        int b = board.dequeue();
        int eta = sys.etaAt(b, s);
        int wait = eta - sys.now;
        string inText = (wait == 0) ? "Now" : intToString(wait) + " min";
        string seats = intToString(sys.seatsFree(b)) + "/" + intToString(sys.buses[b].capacity);

        cout << left << setw(4) << rank
             << setw(6) << sys.buses[b].busNo
             << setw(10) << (sys.buses[b].type == 'S' ? "Standard" : "Mini")
             << setw(8) << formatTime(eta)
             << setw(9) << inText
             << setw(18) << seats
             << sys.seatStatus(b) << endl;
        rank++;
    }
}

void featureArrivalBoard() {
    int s = askForStation("Enter station name: ");
    if (s == -1) return;
    printArrivalBoard(s);
}

// ---------------------------------------------------------------------
// Feature 5: Recommend a Bus
// ---------------------------------------------------------------------
void printReason(int b, int s) {
    int wait = sys.etaAt(b, s) - sys.now;
    cout << "Bus " << sys.buses[b].busNo << " (arrives in " << wait << " min, "
         << sys.seatsFree(b) << " free seats, " << sys.seatStatus(b) << ")";
}

void featureRecommend() {
    int s = askForStation("Enter your station: ");
    if (s == -1) return;

    if (sys.busesAtStation[s].isEmpty()) {
        cout << "No bus serves this station." << endl;
        return;
    }

    Recommendation rec = sys.recommend(s);

    cout << endl << "Station: " << sys.stations[s].name << "   Time: " << formatTime(sys.now) << endl;
    if (rec.eligibleCount == 0) {
        cout << "No bus to recommend: all buses have passed or are Full." << endl;
        return;
    }

    cout << "Recommended : ";
    printReason(rec.bestBus, s);
    cout << endl;

    if (rec.secondBus != -1) {
        cout << "Second best : ";
        printReason(rec.secondBus, s);
        cout << endl;
    }
}

// ---------------------------------------------------------------------
// Features 6, 7, 8: Graph Stubs
// ---------------------------------------------------------------------
void featureShortestTime() {
    int from = askForStation("From station: ");
    if (from == -1) return;
    int to = askForStation("To station: ");
    if (to == -1) return;
    printNotReady("Shortest time (Dijkstra)", "Step 3");
}

void featureFewestStops() {
    int from = askForStation("From station: ");
    if (from == -1) return;
    int to = askForStation("To station: ");
    if (to == -1) return;
    printNotReady("Fewest stops (BFS)", "Step 4");
}

void featureConnectivity() {
    int from = askForStation("Start station: ");
    if (from == -1) return;
    printNotReady("Connectivity (DFS)", "Step 5");
}

// ---------------------------------------------------------------------
// Feature 9: Sorting Demo Stub
// ---------------------------------------------------------------------
void featureSortingDemo() {
    cout << "Sort by: 1) Bus number  2) Free seats  3) Arrival at Geu" << endl;
    int sortBy = readNumber("Choice: ");
    cout << "Algorithm: 1) Bubble  2) Insertion  3) Selection" << endl;
    int algorithm = readNumber("Choice: ");
    if (sortBy < 1 || sortBy > 3 || algorithm < 1 || algorithm > 3) {
        cout << "Invalid choice." << endl;
        return;
    }
    printNotReady("Sorting demo", "Step 6");
}

// ---------------------------------------------------------------------
// Feature 10: Dashboard Stub
// ---------------------------------------------------------------------
void featureDashboard() {
    printNotReady("Dashboard", "Step 7");
}

// ---------------------------------------------------------------------
// Feature 11: Advance Clock (Stack gives UNDO capability)
// ---------------------------------------------------------------------
void featureClock() {
    cout << "Time now: " << formatTime(sys.now) << endl;
    cout << " 1. +1 minute   2. +5 minutes   3. +10 minutes" << endl;
    cout << " 4. Set time    5. Undo last clock change" << endl;
    int choice = readNumber("Choice: ");

    if (choice == 1) {
        sys.advanceClock(1);
    } else if (choice == 2) {
        sys.advanceClock(5);
    } else if (choice == 3) {
        sys.advanceClock(10);
    } else if (choice == 4) {
        string timeStr = readLine("New time (HH:MM): ");
        int minutes = parseTime(timeStr);
        if (minutes != -1) {
            sys.setClock(minutes);
        } else {
            cout << "Invalid time. Use HH:MM, for example 07:45." << endl;
            return;
        }
    } else if (choice == 5) {
        if (!sys.undoClock()) {
            cout << "Nothing to undo." << endl;
            return;
        }
    } else {
        cout << "Invalid choice." << endl;
        return;
    }
    cout << "Time is now " << formatTime(sys.now) << endl;
}

// ---------------------------------------------------------------------
// Feature 12: Save Report Stub
// ---------------------------------------------------------------------
void featureSaveReport() {
    int s = askForStation("Station for the report: ");
    if (s == -1) return;
    printNotReady("Save report", "Step 8");
}

// ---------------------------------------------------------------------
// Menu Display
// ---------------------------------------------------------------------
void printMenu() {
    cout << endl;
    cout << "=========== COLLEGE BUS TRACKER (Team ByteMe) ===========" << endl;
    cout << " Time: " << formatTime(sys.now) << "   Buses: " << sys.busCount
         << "   Stations: " << sys.stationCount << endl;
    cout << "---------------------------------------------------------" << endl;
    cout << "  1. Search station          6. Shortest time (Dijkstra)  [TODO]" << endl;
    cout << "  2. Search bus              7. Fewest stops (BFS)        [TODO]" << endl;
    cout << "  3. Search route            8. Connectivity (DFS)        [TODO]" << endl;
    cout << "  4. Arrival board           9. Sorting demo              [TODO]" << endl;
    cout << "  5. Recommend a bus        10. Dashboard                 [TODO]" << endl;
    cout << " 11. Advance clock          12. Save report              [TODO]" << endl;
    cout << "  0. Exit" << endl;
}

// ---------------------------------------------------------------------
// Main Function
// ---------------------------------------------------------------------
int main(int argc, char* argv[]) {
    string folder = "data/";
    if (argc > 1) {
        folder = argv[1];
        if (folder.length() > 0 && folder[folder.length() - 1] != '/') {
            folder += "/";
        }
    } else {
        ifstream test("data/buses.csv");
        if (!test.is_open()) {
            folder = "";
        }
        test.close();
    }

    cout << "Loading data from '" << (folder == "" ? "./" : folder) << "' ..." << endl;
    if (!loadAllData(folder)) {
        cout << "Cannot start without data files. Exiting." << endl;
        return 1;
    }

    bool running = true;
    while (running) {
        printMenu();
        int choice = readNumber("Choice: ");
        if (choice == -1) {
            cout << "Please enter a number from the menu." << endl;
            continue;
        }
        switch (choice) {
            case 1:  featureSearchStation(); break;
            case 2:  featureSearchBus(); break;
            case 3:  featureSearchRoute(); break;
            case 4:  featureArrivalBoard(); break;
            case 5:  featureRecommend(); break;
            case 6:  featureShortestTime(); break;
            case 7:  featureFewestStops(); break;
            case 8:  featureConnectivity(); break;
            case 9:  featureSortingDemo(); break;
            case 10: featureDashboard(); break;
            case 11: featureClock(); break;
            case 12: featureSaveReport(); break;
            case 0:  running = false; break;
            default: cout << "Invalid choice. Try again." << endl;
        }
    }
    cout << "Goodbye!" << endl;
    return 0;
}
