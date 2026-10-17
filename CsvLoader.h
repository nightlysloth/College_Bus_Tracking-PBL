// =====================================================================
// File: CsvLoader.h
// Purpose: Simple CSV file reader for the College Bus Tracking System.
//
// All functions use call-by-value and standard file streams (<fstream>).
// No static functions or variables.
// =====================================================================
#ifndef CSVLOADER_H
#define CSVLOADER_H

#include <string>
#include "Models.h"
using namespace std;

// String helpers (call-by-value)
string trimString(string text);
int splitString(string line, char separator, string parts[], int maxParts);

// Data loading functions (load into the global 'sys' object)
int loadStations(string path);
int loadBuses(string path);
int loadEdges(string path);
bool loadAllData(string folder);
int verifyRouteStops(string path);

#endif
