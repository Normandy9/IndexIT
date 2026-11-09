#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <sstream>

namespace {
const WORD appIconResource = 101;
enum { PATH_ID = 1, BROWSE_ID, QUERY_ID, RUN_ID, SUGGESTIONS_ID, STATUS_ID, OUTPUT_ID };
enum Action { INDEX, SEARCH, FIND, SUGGEST, HISTORY, STATS, CLEAN, ACTION_COUNT };
enum { ACTION_ID_BASE = 100, SUGGEST_TIMER = 1, PROCESS_TIMER = 2 };
enum ProcessMode { NORMAL_COMMAND, LIVE_SUGGESTIONS };

const wchar_t* actionNames[ACTION_COUNT] = {
    L"Index", L"Search text", L"Find document", L"Suggestions", L"History", L"Statistics", L"Clean cache"
};
const wchar_t* actionDescriptions[ACTION_COUNT] = {
    L"Scan the selected folder and update IndexIt's searchable index.",
    L"Search document contents and show matching files.",
    L"Find indexed documents by file name, without searching their contents.",
    L"List indexed terms that begin with the entered prefix.",
    L"View recent searches stored by IndexIt.",
    L"Show document, term, and cache statistics for the selected folder.",
    L"Delete IndexIt cache files after you confirm the action."
};

HWND pathLabel, pathEdit, browseButton, actionHeading, descriptionLabel;
HWND actionButtons[ACTION_COUNT], queryLabel, queryEdit, suggestionsList, suggestionsHint;
HWND runButton, statusLabel, outputLabel, outputEdit, tooltipWindow;
HANDLE childProcess = nullptr;
HANDLE outputPipe = nullptr;
HBRUSH backgroundBrush, whiteBrush;
HFONT uiFont, headingFont;
HMODULE richEditModule;
Action selectedAction = SEARCH;
ProcessMode processMode = NORMAL_COMMAND;
std::wstring requestedPrefix;
std::string outputBytes;
const std::size_t maxOutputBytes = 4 * 1024 * 1024;
const COLORREF pageColor = RGB(244, 247, 251);
const COLORREF cardColor = RGB(255, 255, 255);
const COLORREF inkColor = RGB(35, 46, 64);
const COLORREF mutedColor = RGB(100, 116, 139);
const COLORREF accentColor = RGB(45, 95, 190);
const COLORREF borderColor = RGB(222, 229, 238);

std::wstring controlText(HWND control) {
    int length = GetWindowTextLengthW(control);
    std::vector<wchar_t> buffer(length + 1);
    GetWindowTextW(control, buffer.data(), length + 1);
    return std::wstring(buffer.data());
}

std::wstring quote(const std::wstring& value) {
    std::wstring result(1, L'"');
    std::size_t slashes = 0;
    for (wchar_t ch : value) {
        if (ch == L'\\') ++slashes;
        else {
            result.append(ch == L'"' ? slashes * 2 + 1 : slashes, L'\\');
            result += ch;
            slashes = 0;
        }
    }
    result.append(slashes * 2, L'\\');
    result += L'"';
    return result;
}

std::wstring cliPath() {
    std::vector<wchar_t> path(32768);
    DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (!length || length == path.size()) return {};
    std::wstring result(path.data(), length);
    std::size_t slash = result.find_last_of(L"\\/");
    result.resize(slash == std::wstring::npos ? 0 : slash + 1);
    return result + L"indexit.exe";
}

std::wstring outputText() {
    if (outputBytes.empty()) return L"(No output.)";
    UINT page = CP_UTF8;
    int length = MultiByteToWideChar(page, MB_ERR_INVALID_CHARS, outputBytes.data(),
                                     static_cast<int>(outputBytes.size()), nullptr, 0);
    if (!length) {
        page = CP_ACP;
        length = MultiByteToWideChar(page, 0, outputBytes.data(),
                                     static_cast<int>(outputBytes.size()), nullptr, 0);
    }
    if (!length) return L"(Could not decode command output.)";
    std::wstring result(length, L'\0');
    MultiByteToWideChar(page, 0, outputBytes.data(), static_cast<int>(outputBytes.size()),
                        &result[0], length);
    return result;
}

std::wstring currentPrefix() {
    std::wstring query = controlText(queryEdit);
    std::size_t split = query.find_last_of(L" \t\r\n");
    return query.substr(split == std::wstring::npos ? 0 : split + 1);
}

void setStatus(const std::wstring& text) {
    SetWindowTextW(statusLabel, text.c_str());
}

void updateAction() {
    for (int i = 0; i < ACTION_COUNT; ++i) {
        InvalidateRect(actionButtons[i], nullptr, TRUE);
    }
    SetWindowTextW(descriptionLabel, actionDescriptions[selectedAction]);
    SetWindowTextW(statusLabel, L"Ready");

    bool needsQuery = selectedAction == SEARCH || selectedAction == FIND || selectedAction == SUGGEST;
    EnableWindow(queryEdit, needsQuery);
    EnableWindow(queryLabel, needsQuery);
    SetWindowTextW(queryLabel, selectedAction == FIND ? L"File name" : L"Query");
    if (selectedAction != SEARCH) {
        ShowWindow(suggestionsList, SW_HIDE);
        ShowWindow(suggestionsHint, SW_HIDE);
    } else if (!controlText(queryEdit).empty()) {
        SetTimer(GetParent(runButton), SUGGEST_TIMER, 300, nullptr);
    }
}

void drawRounded(HDC dc, const RECT& rect, COLORREF fill, COLORREF outline, int radius) {
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, outline);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void paintWindow(HWND window) {
    PAINTSTRUCT paint;
    HDC dc = BeginPaint(window, &paint);
    RECT client;
    GetClientRect(window, &client);
    FillRect(dc, &client, backgroundBrush);

    RECT header{0, 0, client.right, 86};
    HBRUSH navy = CreateSolidBrush(RGB(25, 38, 59));
    FillRect(dc, &header, navy);
    DeleteObject(navy);

    HFONT oldFont = static_cast<HFONT>(SelectObject(dc, headingFont));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));
    RECT title{28, 15, client.right - 24, 48};
    DrawTextW(dc, L"IndexIt", -1, &title, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, uiFont);
    SetTextColor(dc, RGB(203, 215, 232));
    RECT subtitle{30, 49, client.right - 24, 74};
    DrawTextW(dc, L"Search and manage your document index", -1, &subtitle,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, oldFont);

    RECT folderCard{16, 104, client.right - 16, 174};
    RECT actionCard{16, 186, client.right - 16, 316};
    bool suggestionsVisible = IsWindowVisible(suggestionsList) != FALSE;
    RECT queryCard{16, 328, client.right - 16, suggestionsVisible ? 514 : 396};
    int outputTop = suggestionsVisible ? 526 : 420;
    RECT outputCard{16, outputTop - 10, client.right - 16, client.bottom - 16};
    drawRounded(dc, folderCard, cardColor, borderColor, 12);
    drawRounded(dc, actionCard, cardColor, borderColor, 12);
    drawRounded(dc, queryCard, cardColor, borderColor, 12);
    drawRounded(dc, outputCard, cardColor, borderColor, 12);
    EndPaint(window, &paint);
}

void drawButton(const DRAWITEMSTRUCT* item) {
    bool primary = item->CtlID == RUN_ID;
    bool selected = item->CtlID >= ACTION_ID_BASE &&
                    item->CtlID == ACTION_ID_BASE + selectedAction;
    bool pressed = (item->itemState & ODS_SELECTED) != 0;
    COLORREF fill = primary || selected ? (pressed ? RGB(35, 76, 153) : accentColor) : cardColor;
    COLORREF outline = primary || selected ? fill : borderColor;
    COLORREF text = primary || selected ? RGB(255, 255, 255) : inkColor;
    drawRounded(item->hDC, item->rcItem, fill, outline, 8);
    wchar_t label[128] = {};
    GetWindowTextW(item->hwndItem, label, 127);
    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, text);
    HFONT oldFont = static_cast<HFONT>(SelectObject(item->hDC, uiFont));
    RECT textRect = item->rcItem;
    DrawTextW(item->hDC, label, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    SelectObject(item->hDC, oldFont);
}

void layout(HWND window) {
    RECT client;
    GetClientRect(window, &client);
    int width = client.right;
    int height = client.bottom;
    MoveWindow(pathLabel, 32, 128, 60, 24, TRUE);
    MoveWindow(pathEdit, 104, 122, width - 226, 30, TRUE);
    MoveWindow(browseButton, width - 106, 122, 74, 30, TRUE);

    MoveWindow(actionHeading, 32, 198, width - 64, 20, TRUE);
    const int gap = 8;
    int buttonWidth = (width - 64 - gap * (ACTION_COUNT - 1)) / ACTION_COUNT;
    for (int i = 0; i < ACTION_COUNT; ++i)
        MoveWindow(actionButtons[i], 32 + i * (buttonWidth + gap), 224, buttonWidth, 34, TRUE);
    MoveWindow(descriptionLabel, 32, 270, width - 64, 22, TRUE);

    MoveWindow(queryLabel, 32, 357, 64, 24, TRUE);
    MoveWindow(queryEdit, 104, 351, width - 226, 30, TRUE);
    MoveWindow(runButton, width - 106, 350, 74, 32, TRUE);
    MoveWindow(suggestionsHint, 104, 386, width - 136, 18, TRUE);
    MoveWindow(suggestionsList, 104, 406, width - 226, 100, TRUE);

    bool suggestionsVisible = IsWindowVisible(suggestionsList) != FALSE;
    int outputTop = suggestionsVisible ? 526 : 420;
    MoveWindow(statusLabel, 32, outputTop - 28, width - 64, 20, TRUE);
    MoveWindow(outputLabel, 32, outputTop, width - 64, 20, TRUE);
    MoveWindow(outputEdit, 32, outputTop + 26, width - 64, height - outputTop - 42, TRUE);
}

void browse(HWND window) {
    BROWSEINFOW info{};
    info.hwndOwner = window;
    info.lpszTitle = L"Choose the folder to index or search";
    info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST selected = SHBrowseForFolderW(&info);
    if (!selected) return;
    wchar_t path[MAX_PATH];
    if (SHGetPathFromIDListW(selected, path)) SetWindowTextW(pathEdit, path);
    CoTaskMemFree(selected);
}

bool startProcess(HWND window, const std::wstring& arguments, ProcessMode mode) {
    if (childProcess) return false;
    std::wstring executable = cliPath();
    if (executable.empty() || GetFileAttributesW(executable.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(window, L"indexit.exe must be beside this application.", L"IndexIt", MB_ICONERROR);
        return false;
    }

    SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
    HANDLE readPipe = nullptr, writePipe = nullptr;
    HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                               &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (input == INVALID_HANDLE_VALUE || !CreatePipe(&readPipe, &writePipe, &security, 0)) {
        if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
        MessageBoxW(window, L"Could not create the background command pipe.", L"IndexIt", MB_ICONERROR);
        return false;
    }
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    std::wstring commandLine = quote(executable) + L" " + arguments;
    std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back(L'\0');
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input;
    startup.hStdOutput = writePipe;
    startup.hStdError = writePipe;
    PROCESS_INFORMATION process{};
    BOOL created = CreateProcessW(executable.c_str(), mutableCommand.data(), nullptr, nullptr, TRUE,
                                  CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
    CloseHandle(input);
    CloseHandle(writePipe);
    if (!created) {
        CloseHandle(readPipe);
        MessageBoxW(window, L"Could not start indexit.exe.", L"IndexIt", MB_ICONERROR);
        return false;
    }

    CloseHandle(process.hThread);
    childProcess = process.hProcess;
    outputPipe = readPipe;
    processMode = mode;
    outputBytes.clear();
    EnableWindow(runButton, FALSE);
    SetTimer(window, PROCESS_TIMER, 100, nullptr);
    return true;
}

void runCommand(HWND window) {
    if (childProcess) return;
    std::wstring folder = controlText(pathEdit);
    bool needsFolder = selectedAction == INDEX || selectedAction == SEARCH ||
                       selectedAction == FIND || selectedAction == SUGGEST || selectedAction == STATS;
    if (needsFolder) {
        DWORD attributes = GetFileAttributesW(folder.c_str());
        if (folder.empty() || attributes == INVALID_FILE_ATTRIBUTES ||
            !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            MessageBoxW(window, L"Choose an existing folder first.", L"IndexIt", MB_ICONWARNING);
            return;
        }
    }

    std::wstring arguments;
    if (selectedAction == INDEX) arguments = L"index " + quote(folder);
    else if (selectedAction == SEARCH || selectedAction == FIND || selectedAction == SUGGEST) {
        std::wstring query = selectedAction == SUGGEST ? currentPrefix() : controlText(queryEdit);
        if (query.empty()) {
            MessageBoxW(window, selectedAction == FIND ? L"Enter part of a file name." :
                        L"Enter a search query or suggestion prefix.", L"IndexIt", MB_ICONWARNING);
            return;
        }
        const wchar_t* command = selectedAction == SEARCH ? L"search " :
                                 selectedAction == FIND ? L"find " : L"suggest ";
        arguments = std::wstring(command) + quote(folder) + L" " + quote(query);
    } else if (selectedAction == HISTORY) arguments = L"history";
    else if (selectedAction == STATS) arguments = L"stats " + quote(folder);
    else if (selectedAction == CLEAN) {
        if (MessageBoxW(window, L"Delete IndexIt's cached index and database files?",
                        L"Confirm cache cleanup", MB_YESNO | MB_ICONWARNING) != IDYES) return;
        arguments = L"clean --yes";
    }

    SetWindowTextW(outputEdit, L"");
    ShowWindow(suggestionsList, SW_HIDE);
    ShowWindow(suggestionsHint, SW_HIDE);
    layout(window);
    setStatus(L"Running IndexIt in the background...");
    startProcess(window, arguments, NORMAL_COMMAND);
}

void updateSuggestions(HWND window) {
    if (selectedAction != SEARCH || childProcess) return;
    requestedPrefix = currentPrefix();
    if (requestedPrefix.empty()) {
        ShowWindow(suggestionsList, SW_HIDE);
        ShowWindow(suggestionsHint, SW_HIDE);
        layout(window);
        return;
    }

    std::wstring folder = controlText(pathEdit);
    DWORD attributes = GetFileAttributesW(folder.c_str());
    if (folder.empty() || attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        ShowWindow(suggestionsList, SW_HIDE);
        ShowWindow(suggestionsHint, SW_HIDE);
        layout(window);
        return;
    }

    std::wstring arguments = L"suggest " + quote(folder) + L" " + quote(requestedPrefix);
    if (startProcess(window, arguments, LIVE_SUGGESTIONS)) {
        setStatus(L"Finding matching terms...");
    }
}

void showSuggestions(HWND window, const std::wstring& output) {
    SendMessageW(suggestionsList, LB_RESETCONTENT, 0, 0);
    std::wistringstream lines(output);
    std::wstring line;
    std::getline(lines, line); // CLI heading
    const std::wstring suffix = L" (doc frequency:";
    while (std::getline(lines, line)) {
        std::size_t begin = line.find_first_not_of(L" \t\r");
        std::size_t end = line.find(suffix);
        if (begin == std::wstring::npos || end == std::wstring::npos || end <= begin) continue;
        SendMessageW(suggestionsList, LB_ADDSTRING, 0,
                     reinterpret_cast<LPARAM>(line.substr(begin, end - begin).c_str()));
    }

    bool hasSuggestions = SendMessageW(suggestionsList, LB_GETCOUNT, 0, 0) > 0;
    ShowWindow(suggestionsList, hasSuggestions ? SW_SHOW : SW_HIDE);
    ShowWindow(suggestionsHint, hasSuggestions ? SW_SHOW : SW_HIDE);
    setStatus(hasSuggestions ? L"Suggestions update as you type; double-click one to complete the word."
                             : L"No matching suggestions.");
    layout(window);

    if (requestedPrefix != currentPrefix()) SetTimer(window, SUGGEST_TIMER, 300, nullptr);
}

void useSelectedSuggestion() {
    int selected = static_cast<int>(SendMessageW(suggestionsList, LB_GETCURSEL, 0, 0));
    if (selected == LB_ERR) return;
    int length = static_cast<int>(SendMessageW(suggestionsList, LB_GETTEXTLEN, selected, 0));
    std::vector<wchar_t> suggestion(length + 1);
    SendMessageW(suggestionsList, LB_GETTEXT, selected, reinterpret_cast<LPARAM>(suggestion.data()));

    std::wstring query = controlText(queryEdit);
    std::size_t split = query.find_last_of(L" \t\r\n");
    query.resize(split == std::wstring::npos ? 0 : split + 1);
    query += suggestion.data();
    SetWindowTextW(queryEdit, query.c_str());
    SendMessageW(queryEdit, EM_SETSEL, query.size(), query.size());
    SetFocus(queryEdit);
}

void pollCommand(HWND window) {
    char buffer[4096];
    DWORD available = 0;
    while (PeekNamedPipe(outputPipe, nullptr, 0, nullptr, &available, nullptr) && available) {
        DWORD bytesRead = 0;
        DWORD wanted = available < sizeof(buffer) ? available : sizeof(buffer);
        if (!ReadFile(outputPipe, buffer, wanted, &bytesRead, nullptr) || !bytesRead) break;
        if (outputBytes.size() < maxOutputBytes) {
            std::size_t remaining = maxOutputBytes - outputBytes.size();
            outputBytes.append(buffer, bytesRead < remaining ? bytesRead : remaining);
        }
    }

    if (WaitForSingleObject(childProcess, 0) != WAIT_OBJECT_0) return;
    if (PeekNamedPipe(outputPipe, nullptr, 0, nullptr, &available, nullptr) && available) return;

    DWORD exitCode = 1;
    GetExitCodeProcess(childProcess, &exitCode);
    CloseHandle(childProcess);
    CloseHandle(outputPipe);
    childProcess = nullptr;
    outputPipe = nullptr;
    KillTimer(window, PROCESS_TIMER);
    EnableWindow(runButton, TRUE);

    if (processMode == LIVE_SUGGESTIONS) {
        showSuggestions(window, outputText());
    } else {
        SetWindowTextW(outputEdit, outputText().c_str());
        setStatus(exitCode == 0 ? L"Finished successfully." : L"Finished with an error.");
    }
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        richEditModule = LoadLibraryW(L"Msftedit.dll");
        if (!richEditModule) return -1;
        auto control = [&](DWORD exStyle, const wchar_t* type, const wchar_t* text,
                           DWORD style, int id) {
            HWND hwnd = CreateWindowExW(exStyle, type, text, style | WS_CHILD | WS_VISIBLE,
                                        0, 0, 0, 0, window,
                                        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                                        GetModuleHandleW(nullptr), nullptr);
            SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(uiFont), TRUE);
            return hwnd;
        };

        pathLabel = control(0, L"STATIC", L"Folder", 0, 0);
        pathEdit = control(WS_EX_CLIENTEDGE, L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, PATH_ID);
        browseButton = control(0, L"BUTTON", L"Browse...", BS_PUSHBUTTON | WS_TABSTOP, BROWSE_ID);
        actionHeading = control(0, L"STATIC", L"CHOOSE AN ACTION", 0, 0);
        for (int i = 0; i < ACTION_COUNT; ++i) {
            actionButtons[i] = control(0, L"BUTTON", actionNames[i],
                                       BS_OWNERDRAW | WS_TABSTOP,
                                       ACTION_ID_BASE + i);
        }
        descriptionLabel = control(0, L"STATIC", L"", 0, 0);
        queryLabel = control(0, L"STATIC", L"Query", 0, 0);
        queryEdit = control(WS_EX_CLIENTEDGE, L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, QUERY_ID);
        runButton = control(0, L"BUTTON", L"Run", BS_OWNERDRAW | WS_TABSTOP, RUN_ID);
        suggestionsHint = control(0, L"STATIC", L"Double-click a suggestion to complete the current word.", 0, 0);
        suggestionsList = control(0, L"LISTBOX", L"", LBS_NOTIFY | LBS_NOINTEGRALHEIGHT |
                                  WS_BORDER | WS_VSCROLL | WS_TABSTOP, SUGGESTIONS_ID);
        statusLabel = control(0, L"STATIC", L"", 0, STATUS_ID);
        outputLabel = control(0, L"STATIC", L"Output", 0, 0);
        outputEdit = control(WS_EX_CLIENTEDGE, L"RICHEDIT50W", L"",
                             ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
                             OUTPUT_ID);
        ShowWindow(suggestionsList, SW_HIDE);
        ShowWindow(suggestionsHint, SW_HIDE);

        tooltipWindow = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr,
                                        WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
                                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                        window, nullptr, GetModuleHandleW(nullptr), nullptr);
        for (int i = 0; i < ACTION_COUNT; ++i) {
            TOOLINFOW tool{};
            tool.cbSize = sizeof(tool);
            tool.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            tool.hwnd = window;
            tool.uId = reinterpret_cast<UINT_PTR>(actionButtons[i]);
            tool.lpszText = const_cast<wchar_t*>(actionDescriptions[i]);
            SendMessageW(tooltipWindow, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&tool));
        }

        layout(window);
        updateAction();
        return 0;
    }
    case WM_SIZE:
        layout(window);
        InvalidateRect(window, nullptr, TRUE);
        return 0;
    case WM_GETMINMAXINFO: {
        MINMAXINFO* limits = reinterpret_cast<MINMAXINFO*>(lParam);
        limits->ptMinTrackSize.x = 760;
        limits->ptMinTrackSize.y = 620;
        return 0;
    }
    case WM_PAINT:
        paintWindow(window);
        return 0;
    case WM_ERASEBKGND: {
        RECT client;
        GetClientRect(window, &client);
        HDC dc = reinterpret_cast<HDC>(wParam);
        RECT invalid;
        if (GetClipBox(dc, &invalid) != ERROR) FillRect(dc, &invalid, backgroundBrush);
        return 1;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, mutedColor);
        return reinterpret_cast<LRESULT>(backgroundBrush);
    }
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetBkColor(dc, cardColor);
        SetTextColor(dc, inkColor);
        return reinterpret_cast<LRESULT>(whiteBrush);
    }
    case WM_DRAWITEM:
        drawButton(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) >= ACTION_ID_BASE && LOWORD(wParam) < ACTION_ID_BASE + ACTION_COUNT) {
            selectedAction = static_cast<Action>(LOWORD(wParam) - ACTION_ID_BASE);
            updateAction();
            layout(window);
            return 0;
        }
        switch (LOWORD(wParam)) {
        case BROWSE_ID: browse(window); return 0;
        case RUN_ID: runCommand(window); return 0;
        case QUERY_ID:
            if (HIWORD(wParam) == EN_CHANGE && selectedAction == SEARCH && !childProcess) {
                KillTimer(window, SUGGEST_TIMER);
                if (!currentPrefix().empty()) SetTimer(window, SUGGEST_TIMER, 300, nullptr);
                else {
                    ShowWindow(suggestionsList, SW_HIDE);
                    ShowWindow(suggestionsHint, SW_HIDE);
                    layout(window);
                }
            }
            return 0;
        case PATH_ID:
            if (HIWORD(wParam) == EN_CHANGE && selectedAction == SEARCH && !childProcess) {
                KillTimer(window, SUGGEST_TIMER);
                if (!currentPrefix().empty()) SetTimer(window, SUGGEST_TIMER, 300, nullptr);
            }
            return 0;
        case SUGGESTIONS_ID:
            if (HIWORD(wParam) == LBN_DBLCLK) useSelectedSuggestion();
            return 0;
        }
        break;
    case WM_TIMER:
        if (wParam == SUGGEST_TIMER) {
            KillTimer(window, SUGGEST_TIMER);
            updateSuggestions(window);
        } else if (wParam == PROCESS_TIMER) pollCommand(window);
        return 0;
    case WM_CLOSE:
        KillTimer(window, SUGGEST_TIMER);
        if (childProcess && MessageBoxW(window,
            L"An operation is still running. Stop it and close IndexIt?",
            L"IndexIt", MB_YESNO | MB_ICONWARNING) != IDYES) return 0;
        if (childProcess) {
            TerminateProcess(childProcess, 1);
            WaitForSingleObject(childProcess, 2000);
            CloseHandle(childProcess);
            CloseHandle(outputPipe);
            childProcess = nullptr;
            outputPipe = nullptr;
        }
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        if (tooltipWindow) DestroyWindow(tooltipWindow);
        DeleteObject(backgroundBrush);
        DeleteObject(whiteBrush);
        DeleteObject(uiFont);
        DeleteObject(headingFont);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand) {
    InitCommonControls();
    backgroundBrush = CreateSolidBrush(pageColor);
    whiteBrush = CreateSolidBrush(cardColor);
    uiFont = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                         OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                         DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    headingFont = CreateFontW(-27, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    const wchar_t className[] = L"IndexItMainWindow";
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    windowClass.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(appIconResource));
    windowClass.hbrBackground = backgroundBrush;
    windowClass.lpszClassName = className;
    if (!RegisterClassW(&windowClass)) return 1;

    HWND window = CreateWindowExW(0, className, L"IndexIt",
                                  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 900, 700,
                                  nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}
