#include <windows.h>
#include "algoSort.h"

// Declare the prototypes of functions
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddButton(HWND hwnd, const char* text, int id, int y);

// Entry point for the Windows application
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "PopNameFinderWindowClass"; // Define the window class name

    WNDCLASSA wc = {}; // Initialize a window class structure
    wc.lpfnWndProc = WindowProc; // Set the window procedure function
    wc.hInstance = hInst; // Set the instance handle
    wc.lpszClassName = CLASS_NAME; // Set the class name
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Set the background color
    RegisterClassA(&wc); // Register the window class

    int winWidth = 400;
    int winHeight = 400;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int x = (screenWidth - winWidth) / 2; // Calculate horizontal center
    int y = (screenHeight - winHeight) / 2; // Calculate vertical center

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "Pop Name Finder",
        WS_OVERLAPPEDWINDOW,
        x, y,
        winWidth, winHeight,
        NULL,
        NULL,
        hInst,
        NULL
    );

    if (!hwnd) return 0;

    AddButton(hwnd, "Filter by State", 1, 20);
    AddButton(hwnd, "Filter by Year", 3, 60);
    AddButton(hwnd, "Search", 4, 100);
    AddButton(hwnd, "Sort A-Z", 5, 140);
    AddButton(hwnd, "Sort by Frequency", 6, 180);
    AddButton(hwnd, "Display All", 2, 220); 
    AddButton(hwnd, "Exit", 7, 260);

    // Load datasets from the "datasets" folder on startup
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    std::string datasetsPath = std::string(buffer) + "\\..\\datasets"; // Assuming "datasets" is one level up from "src"

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((datasetsPath + "\\*.txt").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string filePath = datasetsPath + "\\" + findFileData.cFileName;
            LoadDataset(filePath);
        } while (FindNextFileA(hFind, &findFileData) != 0);
        FindClose(hFind);
        DisplayMessage(hwnd, "Loaded datasets from the 'datasets' folder. Total entries: " + std::to_string(originalDataset.size()) + ".");
    } else {
        DisplayMessage(hwnd, "Could not find the 'datasets' folder or any .txt files within it.", "Warning");
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // Show result message from search
        if (!searchResult.empty()) {
            DisplayMessage(hwnd, searchResult);
            searchResult.clear();
        }
    }

    return 0;
}