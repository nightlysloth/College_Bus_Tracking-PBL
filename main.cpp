#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include "CsvLoader.cpp"
#include "Algo.cpp"
using namespace std;

string readLine(string prompt) {
    cout << prompt;
    string line = "";
    if (!getline(cin, line)) exit(0);
    return trimString(line);
}

int readNumber(string prompt) {
    return parseInt(readLine(prompt));
}

int askForStation(string prompt) {
    string name = readLine(prompt);
    if (name.empty()) {
        cout << "Error: No station name entered." << endl;
        return -1;
    }
    int idx = sys.findStation(name);
    if (idx != -1) return idx;

    cout << "Station '" << name << "' not found." << endl;
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

void printBusReason(int b, int s) {
    int wait = sys.etaAt(b, s) - sys.now;
    string waitText = (wait <= 0) ? "Now" : intToString(wait) + " min";
    cout << "Bus " << sys.buses[b].busNo << " (arrives in " << waitText
         << ", " << sys.seatsFree(b) << " free seats, " << sys.seatStatus(b) << ")";
}

void featureSearchStation() {
    cout << "\n--- Search Station ---\n";
    int s = askForStation("Enter station name (full or prefix): ");
    if (s == -1) return;

    cout << "\nStation #" << sys.stations[s].id << ": " << sys.stations[s].name << endl;
    if (sys.busesAtStation[s].isEmpty()) {
        cout << "No bus serves this station." << endl;
        return;
    }
    int total = sys.busesAtStation[s].size();
    cout << "Buses serving this station (" << total << "): ";
    for (int i = 0; i < total; i++) {
        cout << sys.buses[sys.busesAtStation[s].getAt(i)].busNo;
        if (i < total - 1) cout << ", ";
    }
    cout << endl;
}

void featureSearchBus() {
    cout << "\n--- Search Bus ---\n";
    int busNo = readNumber("Enter bus number: ");
    if (busNo == -1) {
        cout << "Invalid input. Please enter a valid number." << endl;
        return;
    }
    int b = sys.findBus(busNo);
    if (b == -1) {
        cout << "Bus " << busNo << " not found in schedule." << endl;
        return;
    }

    Bus bus = sys.buses[b];
    int status = sys.getStatus(b);
    int cur = sys.currentStopIndex(b);

    cout << "\nBus " << bus.busNo << endl;
    cout << "  Registration : " << bus.registration << endl;
    cout << "  Driver       : " << bus.driver << endl;
    cout << "  Type         : " << (bus.type == 'S' ? "Standard Bus" : "Mini Bus")
         << " (" << bus.capacity << " seats)" << endl;
    cout << "  Departure    : " << formatTime(bus.departure) << " from " << sys.stations[bus.stops[0]].name << endl;
    cout << "  Status       : " << statusName(status) << " (time now: " << formatTime(sys.now) << ")" << endl;

    if (status == NOT_STARTED) {
        cout << "  Current stop : Not started yet (starts at " << sys.stations[bus.stops[0]].name << ")\n";
        cout << "  Next stop    : " << sys.stations[bus.stops[0]].name << endl;
    } else if (status == REACHED) {
        cout << "  Current stop : " << sys.stations[bus.stops[cur]].name << " (Destination)\n";
        cout << "  Next stop    : - (Trip completed)\n";
    } else {
        cout << "  Current stop : " << sys.stations[bus.stops[cur]].name << endl;
        cout << "  Next stop    : " << sys.stations[bus.stops[cur + 1]].name << endl;
    }
    cout << "  Stops left   : " << (bus.stopCount - 1 - cur) << endl;
    cout << "  Seats        : " << sys.seatsFree(b) << " free / " << bus.capacity
         << " (" << sys.seatStatus(b) << ")" << endl;
}

void featureSearchRoute() {
    cout << "\n--- Search Route ---\n";
    cout << " 1. View full route of a specific bus\n";
    cout << " 2. Find direct buses between two stations\n";
    int mode = readNumber("Choice (1-2): ");

    if (mode == 1) {
        int busNo = readNumber("Enter bus number: ");
        if (busNo == -1) return;
        int b = sys.findBus(busNo);
        if (b == -1) {
            cout << "Bus " << busNo << " not found." << endl;
            return;
        }
        Bus bus = sys.buses[b];
        int status = sys.getStatus(b);
        int cur = sys.currentStopIndex(b);

        cout << "\nRoute of Bus " << bus.busNo << " (" << statusName(status) << "):\n";
        cout << left << setw(4) << "#" << setw(28) << "Station" << "Scheduled Time\n";
        cout << "--------------------------------------------------------\n";
        for (int i = 0; i < bus.stopCount; i++) {
            cout << left << setw(4) << (i + 1)
                 << setw(28) << sys.stations[bus.stops[i]].name
                 << formatTime(bus.departure + sys.cumulativeTime(b, i));
            if (status == RUNNING && i == cur) cout << "   <-- [BUS IS HERE]";
            cout << endl;
        }
        cout << "Total Route Travel Time: " << sys.cumulativeTime(b, bus.stopCount - 1) << " minutes\n";
    } else if (mode == 2) {
        int src = askForStation("Enter origin station: ");
        if (src == -1) return;
        int dst = askForStation("Enter destination station: ");
        if (dst == -1) return;

        cout << "\nDirect buses from " << sys.stations[src].name << " to " << sys.stations[dst].name << ":\n";
        int foundCount = 0;
        for (int b = 0; b < sys.busCount; b++) {
            int posSrc = sys.stopPositionOf(b, src);
            int posDst = sys.stopPositionOf(b, dst);
            if (posSrc != -1 && posDst != -1 && posSrc < posDst) {
                if (foundCount == 0) {
                    cout << left << setw(8) << "Bus" << setw(10) << "Type" << setw(12) << "Departure"
                         << setw(14) << "ETA at Src" << setw(14) << "ETA at Dst"
                         << setw(12) << "Ride Time" << "Free Seats\n";
                    cout << "----------------------------------------------------------------------------------\n";
                }
                foundCount++;
                int etaSrc = sys.etaAt(b, src), etaDst = sys.etaAt(b, dst);
                cout << left << setw(8) << sys.buses[b].busNo
                     << setw(10) << (sys.buses[b].type == 'S' ? "Standard" : "Mini")
                     << setw(12) << formatTime(sys.buses[b].departure)
                     << setw(14) << formatTime(etaSrc)
                     << setw(14) << formatTime(etaDst)
                     << setw(12) << (intToString(etaDst - etaSrc) + " min")
                     << sys.seatsFree(b) << "/" << sys.buses[b].capacity << endl;
            }
        }
        if (foundCount == 0) cout << "No direct bus found between these stations.\n";
    }
}

void printArrivalBoardToStream(ostream& out, int s) {
    out << "\nStation: " << sys.stations[s].name << "                    Time: " << formatTime(sys.now) << endl;
    if (sys.busesAtStation[s].isEmpty()) {
        out << "No bus serves this station.\n";
        return;
    }
    IntQueue board = sys.buildArrivalBoard(s);
    if (board.isEmpty()) {
        out << "No more buses coming: every bus for this station has already passed.\n";
        return;
    }
    out << left << setw(4) << "#" << setw(6) << "Bus" << setw(10) << "Type"
        << setw(8) << "ETA" << setw(9) << "In" << setw(18) << "Seats(free/cap)" << "Status\n";
    out << "----------------------------------------------------------------\n";
    int rank = 1;
    while (!board.isEmpty()) {
        int b = board.dequeue();
        int eta = sys.etaAt(b, s), wait = eta - sys.now;
        out << left << setw(4) << rank++
            << setw(6) << sys.buses[b].busNo
            << setw(10) << (sys.buses[b].type == 'S' ? "Standard" : "Mini")
            << setw(8) << formatTime(eta)
            << setw(9) << (wait <= 0 ? "Now" : intToString(wait) + " min")
            << setw(18) << (intToString(sys.seatsFree(b)) + "/" + intToString(sys.buses[b].capacity))
            << sys.seatStatus(b) << endl;
    }
    Recommendation rec = sys.recommend(s);
    if (rec.eligibleCount > 0) {
        int best = rec.bestBus, wait = sys.etaAt(best, s) - sys.now;
        out << "----------------------------------------------------------------\n";
        out << "Recommended: Bus " << sys.buses[best].busNo << " (arrives in "
            << (wait <= 0 ? "now" : intToString(wait) + " min") << ", "
            << sys.seatsFree(best) << " free seats, " << sys.seatStatus(best) << ")\n";
    }
}

void featureArrivalBoard() {
    cout << "\n--- Arrival Board ---\n";
    int s = askForStation("Enter station name: ");
    if (s != -1) printArrivalBoardToStream(cout, s);
}

void featureRecommend() {
    cout << "\n--- Recommend a Bus ---\n";
    int s = askForStation("Enter your station: ");
    if (s == -1) return;
    if (sys.busesAtStation[s].isEmpty()) {
        cout << "No bus serves this station.\n";
        return;
    }
    Recommendation rec = sys.recommend(s);
    cout << "\nStation: " << sys.stations[s].name << "   Time: " << formatTime(sys.now) << endl;
    if (rec.eligibleCount == 0) {
        cout << "No bus to recommend: all buses have passed or are Full.\n";
        return;
    }
    cout << "Recommended : ";
    printBusReason(rec.bestBus, s);
    cout << endl;
    if (rec.secondBus != -1) {
        cout << "Second best : ";
        printBusReason(rec.secondBus, s);
        cout << endl;
    }
}

void showExactPathBuses(int path[], int pathLength) {
    int matchingBuses[MAX_BUSES], matchCount = 0;
    for (int b = 0; b < sys.busCount; b++) {
        for (int k = 0; k <= sys.buses[b].stopCount - pathLength; k++) {
            if (sys.buses[b].stops[k] == path[0]) {
                bool exact = true;
                for (int j = 1; j < pathLength; j++) {
                    if (sys.buses[b].stops[k + j] != path[j]) {
                        exact = false;
                        break;
                    }
                }
                if (exact) {
                    matchingBuses[matchCount++] = sys.buses[b].busNo;
                    break;
                }
            }
        }
    }
    if (matchCount > 0) {
        cout << "Bus following this exact path: ";
        for (int i = 0; i < matchCount; i++) {
            cout << "Bus " << matchingBuses[i] << (i < matchCount - 1 ? ", " : "\n");
        }
    }
}

void featureDijkstra() {
    cout << "\n--- Shortest Travel Time Route (Dijkstra) ---\n";
    int src = askForStation("Enter starting station: ");
    if (src == -1) return;
    int dst = askForStation("Enter destination station: ");
    if (dst == -1) return;
    if (src == dst) {
        cout << "Starting and destination stations are identical!\n";
        return;
    }

    int totalMinutes = 0, path[MAX_STATIONS], pathLength = 0;
    if (!dijkstraShortestPath(src, dst, totalMinutes, path, pathLength)) {
        cout << "No route found between " << sys.stations[src].name << " and " << sys.stations[dst].name << ".\n";
        return;
    }
    cout << "\nShortest Travel Time : " << totalMinutes << " minutes\n";
    cout << "Total Stations in Path: " << pathLength << endl;
    cout << "Path: ";
    for (int i = 0; i < pathLength; i++) {
        cout << sys.stations[path[i]].name << (i < pathLength - 1 ? " -> " : "\n");
    }
    showExactPathBuses(path, pathLength);
}

void featureBFS() {
    cout << "\n--- Fewest Stops Route (BFS) ---\n";
    int src = askForStation("Enter starting station: ");
    if (src == -1) return;
    int dst = askForStation("Enter destination station: ");
    if (dst == -1) return;
    if (src == dst) {
        cout << "Starting and destination stations are identical!\n";
        return;
    }

    int stopCount = 0, path[MAX_STATIONS], pathLength = 0;
    if (!bfsFewestStops(src, dst, stopCount, path, pathLength)) {
        cout << "No route found between " << sys.stations[src].name << " and " << sys.stations[dst].name << ".\n";
        return;
    }
    cout << "\nMinimum Stops : " << stopCount << " stops\nPath: ";
    for (int i = 0; i < pathLength; i++) {
        cout << sys.stations[path[i]].name << (i < pathLength - 1 ? " -> " : "\n");
    }
    showExactPathBuses(path, pathLength);
}

void featureDFS() {
    cout << "\n--- Station Reachability (DFS) ---\n";
    int src = askForStation("Enter starting station: ");
    if (src == -1) return;

    int reachable[MAX_STATIONS];
    int count = dfsReachableStations(src, reachable, MAX_STATIONS);
    cout << "\nReachable stations from " << sys.stations[src].name << " (" << count << " stations):\n";
    for (int i = 0; i < count; i++) {
        cout << "  " << setw(2) << (i + 1) << ". " << sys.stations[reachable[i]].name << endl;
    }
}

void featureSortingDemo() {
    cout << "\n--- Sort Bus Fleet (Merge Sort) ---\n";
    cout << "Select sorting key:\n 1. Bus Number\n 2. Departure Time\n 3. Free Seats (Active/Departed Buses Only)\n";
    int keyChoice = readNumber("Choice (1-3): ");
    if (keyChoice < 1 || keyChoice > 3) return;

    int busIndices[MAX_BUSES], keys[MAX_BUSES], n = 0;
    if (keyChoice == 3) {
        for (int i = 0; i < sys.busCount; i++) {
            if (sys.getStatus(i) != NOT_STARTED) {
                busIndices[n] = i;
                keys[n] = sys.seatsFree(i);
                n++;
            }
        }
        if (n == 0) {
            cout << "\nNo buses have departed yet at " << formatTime(sys.now) << ".\n";
            cout << "Tip: Use Option 11 (Advance clock) to advance time and start bus journeys!\n";
            return;
        }
    } else {
        n = sys.busCount;
        for (int i = 0; i < n; i++) {
            busIndices[i] = i;
            if (keyChoice == 1) keys[i] = sys.buses[i].busNo;
            else keys[i] = sys.buses[i].departure;
        }
    }

    int comp = 0, moves = 0;
    runMergeSort(busIndices, keys, n, comp, moves);

    cout << "\nSorted using Merge Sort (O(N log N)):\n";
    cout << "Total Buses Displayed: " << n << endl;
    cout << "Operations : " << comp << " comparisons, " << moves << " data moves.\n";
    cout << left << setw(6) << "#" << setw(8) << "Bus" << setw(12) << "Departure"
         << setw(16) << "Seats Free/Cap" << "Driver\n";
    cout << "--------------------------------------------------------\n";
    for (int i = 0; i < n; i++) {
        int b = busIndices[i];
        cout << left << setw(6) << (i + 1)
             << setw(8) << sys.buses[b].busNo
             << setw(12) << formatTime(sys.buses[b].departure)
             << setw(16) << (intToString(sys.seatsFree(b)) + "/" + intToString(sys.buses[b].capacity))
             << sys.buses[b].driver << endl;
    }
}

void featureDashboard() {
    int running = 0, notStarted = 0, reached = 0;
    for (int i = 0; i < sys.busCount; i++) {
        int st = sys.getStatus(i);
        if (st == RUNNING) running++;
        else if (st == NOT_STARTED) notStarted++;
        else reached++;
    }

    cout << "\n========================== SYSTEM DASHBOARD ==========================\n";
    cout << "Current Simulated Time: " << formatTime(sys.now) << endl;
    cout << "Overview: " << sys.busCount << " Buses | " << sys.stationCount << " Stations | "
         << sys.overrideCount << " Overrides\n";
    cout << "Status: " << running << " Running | " << notStarted << " Not Started | " << reached << " Reached\n";
    cout << "----------------------------------------------------------------------\n";

    if (running > 0) {
        cout << "Active Buses:\n";
        cout << left << setw(6) << "Bus" << setw(18) << "Driver" << setw(10) << "Dep"
             << setw(20) << "Current Stop" << setw(20) << "Next Stop" << "Seats Free\n";
        cout << "----------------------------------------------------------------------\n";
        for (int i = 0; i < sys.busCount; i++) {
            if (sys.getStatus(i) == RUNNING) {
                Bus b = sys.buses[i];
                int cur = sys.currentStopIndex(i);
                cout << left << setw(6) << b.busNo
                     << setw(18) << b.driver
                     << setw(10) << formatTime(b.departure)
                     << setw(20) << sys.stations[b.stops[cur]].name
                     << setw(20) << sys.stations[b.stops[cur + 1]].name
                     << sys.seatsFree(i) << "/" << b.capacity << endl;
            }
        }
    }

    cout << "----------------------------------------------------------------------\n";
    cout << "Major Hub Stations:\n";
    int hI[MAX_STATIONS], hB[MAX_STATIONS];
    for (int i = 0; i < sys.stationCount; i++) { hI[i] = i; hB[i] = sys.stations[i].busesServing; }
    for (int i = 0; i < sys.stationCount - 1; i++) {
        for (int j = 0; j < sys.stationCount - i - 1; j++) {
            if (hB[j] < hB[j + 1]) {
                int tb = hB[j]; hB[j] = hB[j + 1]; hB[j + 1] = tb;
                int ti = hI[j]; hI[j] = hI[j + 1]; hI[j + 1] = ti;
            }
        }
    }
    for (int i = 0; i < 5 && i < sys.stationCount; i++) {
        cout << "  " << (i + 1) << ". " << left << setw(20) << sys.stations[hI[i]].name
             << ": " << hB[i] << " buses\n";
    }
    cout << "======================================================================\n";
}

void featureClock() {
    cout << "\n--- Simulated Clock Controls ---\n";
    cout << "Current Time: " << formatTime(sys.now) << endl;
    cout << " 1. +1 min  2. +5 min  3. +10 min  4. Set time  5. Undo\n";
    int choice = readNumber("Choice (1-5): ");

    if (choice == 1) sys.advanceClock(1);
    else if (choice == 2) sys.advanceClock(5);
    else if (choice == 3) sys.advanceClock(10);
    else if (choice == 4) {
        int m = parseTime(readLine("Enter time (HH:MM): "));
        if (m != -1) sys.setClock(m);
        else { cout << "Invalid time format.\n"; return; }
    } else if (choice == 5) {
        if (!sys.undoClock()) { cout << "No clock change to undo.\n"; return; }
    } else return;
    cout << "Clock updated! Current time is now " << formatTime(sys.now) << endl;
}

void featureSaveReport() {
    cout << "\n--- Save Arrival Board Report ---\n";
    int s = askForStation("Enter station name: ");
    if (s == -1) return;

    ofstream reportFile("report.txt");
    if (!reportFile.is_open()) {
        cout << "Error: Unable to open 'report.txt'.\n";
        return;
    }
    reportFile << "================================================================\n";
    reportFile << "          COLLEGE BUS TRACKING SYSTEM - STATION REPORT          \n";
    reportFile << "================================================================\n";
    printArrivalBoardToStream(reportFile, s);
    reportFile << "================================================================\n";
    reportFile.close();
    cout << "Report successfully saved to 'report.txt'!\n";
}

void printMenu() {
    int running = 0, notStarted = 0, reached = 0;
    for (int i = 0; i < sys.busCount; i++) {
        int st = sys.getStatus(i);
        if (st == RUNNING) running++;
        else if (st == NOT_STARTED) notStarted++;
        else reached++;
    }

    cout << "\n=========== COLLEGE BUS TRACKER (Team ByteMe) ===========\n";
    cout << " Time: " << formatTime(sys.now)
         << "   Running: " << running
         << "   Not started: " << notStarted
         << "   Reached: " << reached << "\n";
    cout << "---------------------------------------------------------\n";
    cout << " 1. Search station (Hash Table)   6. Shortest time (Dijkstra)\n";
    cout << " 2. Search bus (Binary Search)    7. Fewest stops (BFS)\n";
    cout << " 3. Search route                  8. Connectivity (DFS)\n";
    cout << " 4. Arrival board (Queue)         9. Sort buses (Merge Sort)\n";
    cout << " 5. Recommend a bus              10. Dashboard\n";
    cout << "11. Advance clock (Stack)        12. Save report (File I/O)\n";
    cout << " 0. Exit\n";
}

int main(int argc, char* argv[]) {
    string folder = "data/";
    if (argc > 1) {
        folder = argv[1];
        if (!folder.empty() && folder[folder.length() - 1] != '/') folder += "/";
    } else {
        ifstream test("data/buses.csv");
        if (!test.is_open()) folder = "";
        test.close();
    }

    cout << "Loading data from '" << (folder.empty() ? "./" : folder) << "' ...\n";
    if (!loadAllData(folder)) {
        cout << "Fatal Error: Cannot start without data files. Exiting.\n";
        return 1;
    }

    bool running = true;
    while (running) {
        printMenu();
        int choice = readNumber("Choice: ");
        if (choice == -1) {
            cout << "Please enter a valid number from the menu.\n";
            continue;
        }

        switch (choice) {
            case 1:  featureSearchStation(); break;
            case 2:  featureSearchBus(); break;
            case 3:  featureSearchRoute(); break;
            case 4:  featureArrivalBoard(); break;
            case 5:  featureRecommend(); break;
            case 6:  featureDijkstra(); break;
            case 7:  featureBFS(); break;
            case 8:  featureDFS(); break;
            case 9:  featureSortingDemo(); break;
            case 10: featureDashboard(); break;
            case 11: featureClock(); break;
            case 12: featureSaveReport(); break;
            case 0:  running = false; break;
            default: cout << "Invalid choice. Please select from the menu.\n";
        }
    }
    cout << "Thank you for using College Bus Tracking System. Goodbye!\n";
    return 0;
}
