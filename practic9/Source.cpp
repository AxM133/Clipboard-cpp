#include <windows.h>
#include <string>
#include <vector>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddClipboardItem(HWND hList, const std::wstring& text);
std::wstring GetClipboardText(HWND hwnd);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Clipboard Manager";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Clipboard Manager",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, // Запрещаем изменение размера
        CW_USEDEFAULT, CW_USEDEFAULT, 385, 400, // Устанавливаем конкретный размер окна
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hwnd == nullptr) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Регистрация для получения уведомлений об изменениях в буфере обмена
    AddClipboardFormatListener(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void AddClipboardItem(HWND hList, const std::wstring& text) {
    SendMessageW(hList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
}

std::wstring GetClipboardText(HWND hwnd) {
    std::wstring text;
    if (OpenClipboard(hwnd)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pszText) {
                text = pszText;
                GlobalUnlock(hData);
            }
        }
        CloseClipboard();
    }
    return text;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hList, hButtonClear, hButtonEdit, hButtonSettings, hButtonPlugin, hButtonAbout, hButtonExit;
    switch (uMsg) {
    case WM_CREATE:
        hList = CreateWindowEx(0, L"LISTBOX", nullptr,
            WS_CHILD | WS_VISIBLE | LBS_STANDARD,
            10, 10, 350, 300,
            hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);

        hButtonClear = CreateWindow(L"BUTTON", L"Clear history",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 320, 100, 30,
            hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);

        hButtonAbout = CreateWindow(L"BUTTON", L"About",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            130, 320, 100, 30,
            hwnd, (HMENU)6, GetModuleHandle(NULL), NULL);

        hButtonExit = CreateWindow(L"BUTTON", L"Exit",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            250, 320, 100, 30,
            hwnd, (HMENU)7, GetModuleHandle(NULL), NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: // List box item selected
            if (HIWORD(wParam) == LBN_DBLCLK) {
                int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (index != LB_ERR) {
                    wchar_t text[256];
                    SendMessageW(hList, LB_GETTEXT, index, reinterpret_cast<LPARAM>(text));
                    MessageBoxW(hwnd, text, L"Элемент буфера обмена", MB_OK);
                }
            }
            break;

        case 2: // Clear history button
            SendMessage(hList, LB_RESETCONTENT, 0, 0);
            break;

        case 6: // About button
            MessageBox(hwnd, L"Это приложение: Буфер обмена", L"About", MB_OK);
            break;

        case 7: // Exit button
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_CLIPBOARDUPDATE:
    {
        std::wstring clipboardText = GetClipboardText(hwnd);
        if (!clipboardText.empty()) {
            AddClipboardItem(hList, clipboardText);
        }
    }
    break;

    case WM_DESTROY:
        RemoveClipboardFormatListener(hwnd);
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
