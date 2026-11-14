#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include "DataStructures.h"
#include "Models.h"
#include "CsvLoader.h"
using namespace std;

string readLine(string prompt) {
    cout << prompt;
    string line = "";
    if (!getline(cin, line)) {
        cout << endl << "Input closed. Goodbye!" << endl;
        exit(0);
    }
    return trimString(line);
}

int readNumber(string prompt) {
    string line = readLine(prompt);
    int value = parseInt(line);
    return value;
}

int askForStation(string prompt) {
    string name = readLine(prompt);
    if (name == "") {
        cout << "No name entered." << endl;
        return -1;
    }

    int index = sys.findStation(name);
    if (index != -1) {
        return index;
    }

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

void featureSearchBus() {
    int busNo = readNumber("Enter bus number: ");
    if (busNo == -1) {
        cout << "Please type a valid number." << endl;
        return;
    }

    int b = sys.findBus(busNo);
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

void featureSearchRoute() {
    int busNo = readNumber("Enter bus number: ");
    if (busNo == -1) {
        cout << "Please enter a valid bus number." << endl;
        return;
    }

    int b = sys.findBus(busNo);
    if (b == -1) {
        cout << "Bus " << busNo << " not found." << endl;
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
}

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

// ==========================================
// SORTING DEMONSTRATION
// ==========================================

// Bubble Sort
void bubbleSortDemo(int items[], int keys[], int n, int &comparisons, int &moves) {
    comparisons = 0;
    moves = 0;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            comparisons++;
            if (keys[j] > keys[j + 1]) {
                int temp = keys[j];
                keys[j] = keys[j + 1];
                keys[j + 1] = temp;
                temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
                moves++;
            }
        }
    }
}

// Insertion Sort
void insertionSortDemo(int items[], int keys[], int n, int &comparisons, int &moves) {
    comparisons = 0;
    moves = 0;
    for (int i = 1; i < n; i++) {
        int currentKey = keys[i];
        int currentItem = items[i];
        int j = i - 1;
        while (j >= 0) {
            comparisons++;
            if (keys[j] > currentKey) {
                keys[j + 1] = keys[j];
                items[j + 1] = items[j];
                moves++;
                j--;
            } else {
                break;
            }
        }
        keys[j + 1] = currentKey;
        items[j + 1] = currentItem;
        moves++;
    }
}

// Selection Sort
void selectionSortDemo(int items[], int keys[], int n, int &comparisons, int &moves) {
    comparisons = 0;
    moves = 0;
    for (int i = 0; i < n - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < n; j++) {
            comparisons++;
            if (keys[j] < keys[minIndex]) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            int temp = keys[i];
            keys[i] = keys[minIndex];
            keys[minIndex] = temp;
            temp = items[i];
            items[i] = items[minIndex];
            items[minIndex] = temp;
            moves++;
        }
    }
}

// Main Sorting Demo Feature
void featureSortingDemo() {
    if (sys.busCount == 0) {
        cout << "No buses available for sorting." << endl;
        return;
    }
    int items[MAX_BUSES];
    int keys[MAX_BUSES];
    int n = sys.busCount;
    cout << "\n========== SORTING DEMONSTRATION ==========\n";
    cout << "\nChoose sorting criteria:\n";
    cout << "1. Sort by Bus Number\n";
    cout << "2. Sort by ETA at Geu\n";
    cout << "3. Sort by Free Seats\n";
    int criteria = readNumber("Enter choice: ");
    if (criteria < 1 || criteria > 3) {
        cout << "Invalid criteria choice.\n";
        return;
    }
    
    // Store bus indexes and their corresponding keys
    for (int i = 0; i < n; i++) {
        items[i] = i;
        if (criteria == 1) {
            keys[i] = sys.buses[i].busNo;
        }
        else if (criteria == 2) {
            int lastStop = sys.buses[i].stops[sys.buses[i].stopCount - 1];
            keys[i] = sys.etaAt(i, lastStop);
        }
        else {
            keys[i] = sys.seatsFree(i);
        }
    }
    cout << "\nChoose Sorting Algorithm:\n";
    cout << "1. Bubble Sort\n";
    cout << "2. Insertion Sort\n";
    cout << "3. Selection Sort\n";
    int algorithm = readNumber("Enter choice: ");
    if (algorithm < 1 || algorithm > 3) {
        cout << "Invalid algorithm choice.\n";
        return;
    }
    int comparisons = 0;
    int moves = 0;
    if (algorithm == 1) {
        bubbleSortDemo(items, keys, n, comparisons, moves);
    }
    else if (algorithm == 2) {
        insertionSortDemo(items, keys, n, comparisons, moves);
    }
    else {
        selectionSortDemo(items, keys, n, comparisons, moves);
    }
    cout << "\n========== SORTED BUS LIST ==========\n";
    cout << left << setw(10) << "Bus No";
    if (criteria == 1)
        cout << "Bus Number";
    else if (criteria == 2)
        cout << "ETA at Geu";
    else
        cout << "Free Seats";
    cout << endl;
    cout << "-------------------------------------\n";
    for (int i = 0; i < n; i++) {
        cout << left << setw(10)<< sys.buses[items[i]].busNo;
        if (criteria == 2)
            cout << formatTime(keys[i]);
        else
            cout << keys[i];
        cout << endl;
    }
    cout << "\n========== PERFORMANCE REPORT ==========\n";
    if (algorithm == 1)
        cout << "Algorithm: Bubble Sort\n";
    else if (algorithm == 2)
        cout << "Algorithm: Insertion Sort\n";
    else
        cout << "Algorithm: Selection Sort\n";
    cout << "Total Buses: " << n << endl;
    cout << "Comparisons: " << comparisons << endl;
    cout << "Swaps/Shifts: " << moves << endl;
    cout << "Total Steps: " << comparisons + moves << endl;
    cout << "========================================\n";
}

void printMenu() {
    cout << endl;
    cout << "=========== COLLEGE BUS TRACKER (Team ByteMe) ===========" << endl;
    cout << " Time: " << formatTime(sys.now) << "   Buses: " << sys.busCount
         << "   Stations: " << sys.stationCount << endl;
    cout << "---------------------------------------------------------" << endl;
    cout << "  1. Search station" << endl;
    cout << "  2. Search bus" << endl;
    cout << "  3. Search route" << endl;
    cout << "  4. Arrival board" << endl;
    cout << "  5. Recommend a bus" << endl;
    cout << "  6. Advance clock" << endl;
    cout << "  7. Sorting Demonstration" << endl;
    cout << "  0. Exit" << endl;
}

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
            case 6:  featureClock(); break;
            case 7:  featureSortingDemo(); break;
            case 0:  running = false; break;
            default: cout << "Invalid choice. Try again." << endl;
        }
    }
    cout << "Goodbye!" << endl;
    return 0;
}
