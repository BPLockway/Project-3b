#ifndef ALGOSORT_H
#define ALGOSORT_H

#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype> // For toupper

#define ID_INPUT 101
#define ID_OK 102
#define ID_CANCEL 103

struct NameEntry {
    std::string name;
    char gender;
    int frequency;
};

extern std::vector<NameEntry> dataset;
extern std::string searchResult;

std::string OpenFileDialog();
std::vector<NameEntry> LoadDataset(const std::string& path);
void DisplayMessage(HWND hwnd, const std::string& text, const std::string& title = "Info");
void ShowScrollableTextWindow(HINSTANCE hInstance, HWND parent, const std::string& title, const std::string& content);
LRESULT CALLBACK SearchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ShowSearchDialog(HWND parent);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddButton(HWND hwnd, const char* text, int id, int y);

#endif