#include "algoSort.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <windows.h>

// Global variables
std::vector<NameEntry> originalDataset; // Store the full dataset in load order
std::vector<NameEntry> dataset;         // The currently displayed/filtered dataset
std::string searchResult;
std::set<int> availableYears;
std::set<std::string> availableStates;
int currentFilterYear = -1; // Initialize with no year filter
std::string currentFilterState = ""; // Initialize with no state filter

// Function to load data from a CSV file
std::vector<NameEntry> LoadDataset(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    int lineNumber = 0;

    if (!file.is_open()) {
        MessageBoxA(NULL, ("Failed to open file: " + path).c_str(), "Error", MB_OK | MB_ICONERROR);
        return dataset;
    }

    while (getline(file, line)) {
        lineNumber++;
        std::stringstream ss(line);
        std::string state, genderStr, yearStr, name, freqStr;
        getline(ss, state, ',');
        getline(ss, genderStr, ',');
        getline(ss, yearStr, ',');
        getline(ss, name, ',');
        getline(ss, freqStr, ',');

        if (!state.empty() && !genderStr.empty() && !yearStr.empty() && !name.empty() && !freqStr.empty()) {
            try {
                int year = std::stoi(yearStr);
                NameEntry entry = { state, genderStr[0], year, name, std::stoi(freqStr) };
                originalDataset.push_back(entry); // Add to originalDataset in file order
                dataset.push_back(entry);         // Also add to the working dataset
                availableYears.insert(year);
                availableStates.insert(state);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error converting to integer on line " << lineNumber << ": " << e.what() << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "Out of range error on line " << lineNumber << ": " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Skipping incomplete line " << lineNumber << std::endl;
        }
    }
    file.close();
    return dataset;
}

// Function to display a message box
void DisplayMessage(HWND hwnd, const std::string& text, const std::string& title /*= "Info"*/) {
    MessageBoxA(hwnd, text.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
}

// Function to show a window with scrollable text
void ShowScrollableTextWindow(HINSTANCE hInstance, HWND parent, const std::string& title, const std::string& content) {
    const char* className = "ScrollableTextWindow";
    static bool registered = false;

    if (!registered) {
        WNDCLASSA wc = {};
        wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
            static HWND hEdit;
            switch (msg) {
                case WM_CREATE: {
                    CREATESTRUCTA* pcs = (CREATESTRUCTA*)lParam;
                    LPCSTR text = (LPCSTR)pcs->lpCreateParams;
                    hEdit = CreateWindowA("EDIT", "",
                        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                        0, 0, 0, 0, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
                    SetWindowTextA(hEdit, text);
                    break;
                }
                case WM_SIZE:
                    MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
                    break;
                case WM_CLOSE:
                    DestroyWindow(hwnd);
                    break;
                }
            return DefWindowProcA(hwnd, msg, wParam, lParam);
        };
        wc.hInstance = hInstance;
        wc.lpszClassName = className;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClassA(&wc);
        registered = true;
    }

    int winWidth = 600;
    int winHeight = 400;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - winWidth) / 2;
    int y = (screenHeight - winHeight) / 2;

    HWND hwnd = CreateWindowExA(0, className, title.c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, winWidth, winHeight,
        parent, NULL, hInstance, (LPVOID)content.c_str());

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

// Window procedure for the search dialog
LRESULT CALLBACK SearchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;

    switch (msg) {
        case WM_CREATE:
            CreateWindowA("STATIC", "Enter name:", WS_VISIBLE | WS_CHILD,
                10, 10, 100, 20, hwnd, NULL, NULL, NULL);
            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER,
                10, 35, 200, 20, hwnd, (HMENU)ID_INPUT, NULL, NULL);
            CreateWindowA("BUTTON", "OK", WS_VISIBLE | WS_CHILD,
                30, 70, 60, 25, hwnd, (HMENU)ID_OK, NULL, NULL);
            CreateWindowA("BUTTON", "Cancel", WS_VISIBLE | WS_CHILD,
                120, 70, 60, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_OK) {
                char buffer[100];
                GetWindowTextA(hEdit, buffer, 100);
                std::string target(buffer);
                std::string result;
                for (const auto& e : dataset) {
                    if (_stricmp(e.name.c_str(), target.c_str()) == 0) {
                        result += e.state + " - " + std::to_string(e.year) + " - " + e.name + " [" + e.gender + "] - " + std::to_string(e.frequency) + "\n";
                    }
                }
                searchResult = result.empty() ? "Name not found." : result;
                DestroyWindow(hwnd);
            } else if (LOWORD(wParam) == ID_CANCEL) {
                searchResult = "";
                DestroyWindow(hwnd);
            }
            break;

        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// Function to show the search dialog
void ShowSearchDialog(HWND parent) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = SearchWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "SearchDialogClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    int winWidth = 250;
    int winHeight = 150;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - winWidth) / 2;
    int y = (screenHeight - winHeight) / 2;

    HWND hSearch = CreateWindowExA(0, "SearchDialogClass", "Search Name",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y, winWidth, winHeight,
        parent, NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(hSearch, SW_SHOW);
    UpdateWindow(hSearch);
}

// Window procedure for the Filter by Year dialog (using text entry)
LRESULT CALLBACK FilterByYearWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;

    switch (msg) {
        case WM_CREATE: {
            CreateWindowA("STATIC", "Enter Year:", WS_VISIBLE | WS_CHILD,
                10, 10, 80, 20, hwnd, NULL, NULL, NULL);
            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, // ES_NUMBER allows only digits
                10, 35, 150, 20, hwnd, (HMENU)ID_YEAR_COMBO, GetModuleHandle(NULL), NULL);

            CreateWindowA("BUTTON", "Filter", WS_VISIBLE | WS_CHILD,
                20, 70, 80, 25, hwnd, (HMENU)ID_FILTER_BUTTON, NULL, NULL);
            CreateWindowA("BUTTON", "Cancel", WS_VISIBLE | WS_CHILD,
                110, 70, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);
            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_FILTER_BUTTON) {
                char buffer[10];
                GetWindowTextA(hEdit, buffer, 10);
                if (strlen(buffer) > 0) {
                    try {
                        int year = std::stoi(buffer);
                        currentFilterYear = year;
                        // Apply the year filter immediately
                        std::vector<NameEntry> filteredDataset;
                        for (const auto& e : dataset) {
                            if (e.year == currentFilterYear) {
                                filteredDataset.push_back(e);
                            }
                        }
                        dataset = filteredDataset;
                        DisplayMessage(hwnd, "Year filter applied. Click 'Display All' to view.");
                        DestroyWindow(hwnd);
                    } catch (const std::invalid_argument& e) {
                        DisplayMessage(hwnd, "Invalid year format. Please enter a number.");
                    } catch (const std::out_of_range& e) {
                        DisplayMessage(hwnd, "Year entered is out of range.");
                    }
                } else {
                    DisplayMessage(hwnd, "Please enter a year to filter by.");
                }
            } else if (LOWORD(wParam) == ID_CANCEL) {
                DestroyWindow(hwnd);
            }
            break;

        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// Function to show the Filter by Year dialog
void ShowFilterByYearDialog(HWND parent) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = FilterByYearWndProc; // Use the modified window procedure
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "FilterByYearDialogClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    int winWidth = 220;
    int winHeight = 120;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - winWidth) / 2;
    int y = (screenHeight - winHeight) / 2;

    HWND hFilterDlg = CreateWindowExA(0, "FilterByYearDialogClass", "Filter by Year",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y, winWidth, winHeight, // Adjust height of the dialog
        parent, NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(hFilterDlg, SW_SHOW);
    UpdateWindow(hFilterDlg);
}

// Window procedure for the Filter by State dialog (using text entry)
LRESULT CALLBACK FilterByStateWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;

    switch (msg) {
        case WM_CREATE: {
            CreateWindowA("STATIC", "Enter State:", WS_VISIBLE | WS_CHILD,
                10, 10, 80, 20, hwnd, NULL, NULL, NULL);
            hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_UPPERCASE, // ES_UPPERCASE for consistent state codes
                10, 35, 150, 20, hwnd, (HMENU)ID_STATE_INPUT, GetModuleHandle(NULL), NULL);

            CreateWindowA("BUTTON", "Filter", WS_VISIBLE | WS_CHILD,
                20, 70, 80, 25, hwnd, (HMENU)ID_FILTER_STATE_BUTTON, NULL, NULL);
            CreateWindowA("BUTTON", "Cancel", WS_VISIBLE | WS_CHILD,
                110, 70, 80, 25, hwnd, (HMENU)ID_CANCEL, NULL, NULL);
            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_FILTER_STATE_BUTTON) {
                char buffer[3]; // Assuming state codes are 2 letters
                GetWindowTextA(hEdit, buffer, 3);
                std::string enteredState = buffer;
                if (!enteredState.empty()) {
                    currentFilterState = enteredState;
                    // Apply the state filter immediately
                    std::vector<NameEntry> filteredDataset;
                    for (const auto& e : dataset) {
                        if (_stricmp(e.state.c_str(), currentFilterState.c_str()) == 0) {
                            filteredDataset.push_back(e);
                        }
                    }
                    dataset = filteredDataset;
                    DisplayMessage(hwnd, "State filter applied. Click 'Display All' to view.");
                    DestroyWindow(hwnd);
                } else {
                    DisplayMessage(hwnd, "Please enter a state to filter by.");
                }
            } else if (LOWORD(wParam) == ID_CANCEL) {
                DestroyWindow(hwnd);
            }
            break;

        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// Function to show the Filter by State dialog
void ShowFilterByStateDialog(HWND parent) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = FilterByStateWndProc; // Use the state filter window procedure
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "FilterByStateDialogClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassA(&wc);

    int winWidth = 220;
    int winHeight = 120;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - winWidth) / 2;
    int y = (screenHeight - winHeight) / 2;

    HWND hFilterDlg = CreateWindowExA(0, "FilterByStateDialogClass", "Filter by State",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y, winWidth, winHeight, // Adjust height of the dialog
        parent, NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(hFilterDlg, SW_SHOW);
    UpdateWindow(hFilterDlg);
}

// Merges two sorted subarrays for frequency (sorting by frequency)
void mergeFreq(std::vector<NameEntry>& arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    std::vector<NameEntry> L(n1);
    std::vector<NameEntry> R(n2);

    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    int i = 0;
    int j = 0;
    int k = l;
    while (i < n1 && j < n2) {
        // Sort in descending order of frequency
        if (L[i].frequency >= R[j].frequency) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSortFreqRecursive(std::vector<NameEntry>& arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSortFreqRecursive(arr, l, m);
        mergeSortFreqRecursive(arr, m + 1, r);
        mergeFreq(arr, l, m, r);
    }
}

// Function for merge sort (by frequency - descending)
void MergeSortByFrequency() {
    mergeSortFreqRecursive(dataset, 0, dataset.size() - 1);
}

// Merges two sorted subarrays for alphabetical order (sorting by name)
void mergeAlpha(std::vector<NameEntry>& arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    std::vector<NameEntry> L(n1);
    std::vector<NameEntry> R(n2);

    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    int i = 0;
    int j = 0;
    int k = l;
    while (i < n1 && j < n2) {
        // Sort in alphabetical order by name
        if (L[i].name <= R[j].name) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSortAlphaRecursive(std::vector<NameEntry>& arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSortAlphaRecursive(arr, l, m);
        mergeSortAlphaRecursive(arr, m + 1, r);
        mergeAlpha(arr, l, m, r);
    }
}

// Function for merge sort (Alphabetical order)
void MergeSortAlpha() {
    mergeSortAlphaRecursive(dataset, 0, dataset.size() - 1);
}

// Window procedure for the main window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1: { // Filter by State
                    ShowFilterByStateDialog(hwnd);
                    break;
                }
                case 2: { // Display All
                    std::string result;
                    for (const auto& e : dataset) { // Use the current state of 'dataset'
                        result += e.state + " | " +
                                  std::to_string(e.year) + " | " +
                                  e.name + " [" + e.gender + "] - " +
                                  std::to_string(e.frequency) + "\r\n";
                    }
                    if (!result.empty())
                        ShowScrollableTextWindow((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), hwnd, "All Names", result);
                    else
                        DisplayMessage(hwnd, "No data to display.");
                    break;
                }
                case 3: { // Filter by Year
                    ShowFilterByYearDialog(hwnd);
                    break;
                }
                case 4: { // Search
                    ShowSearchDialog(hwnd);
                    break;
                }
                case 5: { // Sort A-Z
                    std::vector<NameEntry> filteredData;
                    for (const auto& e : originalDataset) {
                        bool yearMatch = (currentFilterYear == -1 || e.year == currentFilterYear);
                        bool stateMatch = (currentFilterState.empty() || _stricmp(e.state.c_str(), currentFilterState.c_str()) == 0);
                        if (yearMatch && stateMatch) {
                            filteredData.push_back(e);
                        }
                    }
                    dataset = filteredData;
                    MergeSortAlpha(); // Sort the filtered dataset in place
                    DisplayMessage(hwnd, "Data sorted alphabetically (A-Z). Click 'Display All' to view.");
                    break;
                }
                case 6: { // Sort by Frequency
                    std::vector<NameEntry> filteredData;
                    for (const auto& e : originalDataset) {
                        bool yearMatch = (currentFilterYear == -1 || e.year == currentFilterYear);
                        bool stateMatch = (currentFilterState.empty() || _stricmp(e.state.c_str(), currentFilterState.c_str()) == 0);
                        if (yearMatch && stateMatch) {
                            filteredData.push_back(e);
                        }
                    }
                    dataset = filteredData;
                    MergeSortByFrequency(); // Sort the filtered dataset in place
                    DisplayMessage(hwnd, "Data sorted by frequency (descending). Click 'Display All' to view.");
                    break;
                }
                case 7: { // Exit
                    PostQuitMessage(0);
                    break;
                }
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

// Function to add a button to a window
void AddButton(HWND hwnd, const char* text, int id, int y) {
    CreateWindowA("BUTTON", text, WS_VISIBLE | WS_CHILD, 20, y, 200, 30, hwnd, (HMENU)id, NULL, NULL);
}