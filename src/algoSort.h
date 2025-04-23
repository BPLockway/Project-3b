#ifndef ALGORITHMSORT_H
#define ALGORITHMSORT_H

#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <set>
#include <windows.h> // For FindFirstFile/FindNextFile

#define ID_INPUT 1
#define ID_OK 2
#define ID_CANCEL 3
#define ID_YEAR_COMBO 4
#define ID_FILTER_BUTTON 5
#define ID_STATE_INPUT 6
#define ID_FILTER_STATE_BUTTON 7

// Define the NameEntry structure with the new dataset format
struct NameEntry {
    std::string state;
    char gender;
    int year;
    std::string name;
    int frequency;
};

// Declare global datasets and search result
extern std::vector<NameEntry> originalDataset; // Store the full dataset
extern std::vector<NameEntry> dataset;        // The currently displayed/filtered dataset
extern std::string searchResult;
extern std::set<int> availableYears;
extern std::set<std::string> availableStates;
extern int currentFilterYear; // To store the year being filtered
extern std::string currentFilterState; // To store the state being filtered

// Function declarations
std::vector<NameEntry> LoadDataset(const std::string& path);
void DisplayMessage(HWND hwnd, const std::string& text, const std::string& title = "Info");
void ShowScrollableTextWindow(HINSTANCE hInstance, HWND parent, const std::string& title, const std::string& content);
void ShowSearchDialog(HWND parent);
void ShowFilterByYearDialog(HWND parent);
void ShowFilterByStateDialog(HWND parent);
void MergeSortAlpha();
void MergeSortByFrequency();
std::string OpenFileDialog();

#endif // ALGORITHMSORT_H