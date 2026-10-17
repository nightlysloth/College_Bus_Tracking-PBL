#ifndef CSVLOADER_H
#define CSVLOADER_H

#include <string>
#include "Models.h"
using namespace std;

string trimString(string text);
int splitString(string line, char separator, string parts[], int maxParts);

int loadStations(string path);
int loadBuses(string path);
int loadEdges(string path);
bool loadAllData(string folder);

#endif
