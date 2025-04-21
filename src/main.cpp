#include "algoSort.h" 

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

    AddButton(hwnd, "1. Load Dataset", 1, 20);
    AddButton(hwnd, "2. Display All", 2, 60);
    AddButton(hwnd, "3. Filter A*", 3, 100);
    AddButton(hwnd, "4. Search", 4, 140);
    AddButton(hwnd, "5. Sort A-Z", 5, 180);
    AddButton(hwnd, "6. Sort by Frequency", 6, 220);
    AddButton(hwnd, "7. Exit", 7, 260);

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