// algoSort.cpp
#include "algoSort.h"

// Global variables
std::vector<NameEntry> dataset;
std::string searchResult;

// Function to open a file dialog
std::string OpenFileDialog() {
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = { sizeof(ofn) };
    ofn.lpstrFilter = "Text Files\0*.txt\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Select a dataset";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    GetOpenFileNameA(&ofn); // Show the dialog
    return std::string(filename);
}

// Function to load data from a CSV file
std::vector<NameEntry> LoadDataset(const std::string& path) {
    std::vector<NameEntry> data;
    std::ifstream file(path);
    std::string line;
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string name, genderStr, freqStr;
        getline(ss, name, ',');
        getline(ss, genderStr, ',');
        getline(ss, freqStr, ',');
        if (!name.empty() && !genderStr.empty() && !freqStr.empty()) {
            data.push_back({ name, genderStr[0], std::stoi(freqStr) });
        }
    }
    return data;
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

    int winWidth = 500;
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
                        result += e.name + " (" + e.gender + ") - " + std::to_string(e.frequency) + "\n";
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    RegisterClassA(&wc);

    HWND hSearch = CreateWindowExA(0, "SearchDialogClass", "Search Name",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 250, 150,
        parent, NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(hSearch, SW_SHOW);
    UpdateWindow(hSearch);
}

// Window procedure for the main window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1: { // Load Dataset
                    std::string path = OpenFileDialog();
                    if (!path.empty()) {
                        dataset = LoadDataset(path);
                        DisplayMessage(hwnd, "Dataset loaded with " + std::to_string(dataset.size()) + " entries.");
                    }
                    break;
                }
                case 2: { // Display All
                    std::string result;
                    for (const auto& e : dataset) {
                        result += e.name + " (" + e.gender + "): " + std::to_string(e.frequency) + "\r\n";
                    }
                    if (!result.empty())
                        ShowScrollableTextWindow((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), hwnd, "All Names", result);
                    else
                        DisplayMessage(hwnd, "No data loaded.");
                    break;
                }

                case 3: { // Filter A*
                    char letter = 'A';
                    std::string result;
                    for (const auto& e : dataset) {
                        if (toupper(e.name[0]) == letter)
                            result += e.name + " (" + e.gender + "): " + std::to_string(e.frequency) + "\n";
                    }
                    DisplayMessage(hwnd, result.empty() ? "No matches found." : result);
                    break;
                }
                case 4: // Search
                    ShowSearchDialog(hwnd);
                    break;
                case 5: { // Sort A-Z
                    std::sort(dataset.begin(), dataset.end(), [](auto& a, auto& b) { return a.name < b.name; });
                    DisplayMessage(hwnd, "Sorted alphabetically.");
                    break;
                }
                case 6: { // Sort by Frequency
                    std::sort(dataset.begin(), dataset.end(), [](auto& a, auto& b) { return a.frequency > b.frequency; });
                    DisplayMessage(hwnd, "Sorted by frequency (descending).");
                    break;
                }
                case 7: // Exit
                    PostQuitMessage(0);
                    break;
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