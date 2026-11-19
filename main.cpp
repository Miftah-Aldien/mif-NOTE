// mif-NOTE C++ Edition - GPL v3
// Mesin editor: Scintilla (sama seperti Notepad++)
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <fstream>
#include <sstream>
#include "Scintilla.h"
#include "SciLexer.h"
HINSTANCE g_hInst;
HWND g_hEdit = NULL;
HWND g_hMain = NULL;
std::wstring g_currentFile = L"";
bool g_showLineNumbers = true;
bool g_wordWrap = true;
HMODULE g_hSci = NULL;
#define IDM_NEW 1001
#define IDM_OPEN 1002
#define IDM_SAVE 1003
#define IDM_SAVEAS 1004
#define IDM_EXIT 1005
#define IDM_VIEW_LINENUM 2001
#define IDM_VIEW_WRAP 2002
#define IDM_RTL 3001
#define IDM_LTR 3002
#define IDM_FONT 3004
LRESULT Sci(int msg, WPARAM wp = 0, LPARAM lp = 0) { return SendMessage(g_hEdit, msg, wp, lp); }
std::string ReadFileBytes(const std::wstring& path) { std::ifstream f(path, std::ios::binary); std::stringstream ss; ss << f.rdbuf(); return ss.str(); }
bool WriteFileBytes(const std::wstring& path, const std::string& data) { std::ofstream f(path, std::ios::binary); f.write(data.data(), data.size()); return (bool)f; }
std::string GetText() { int len = (int)Sci(SCI_GETLENGTH); std::string buf(len + 1, '\0'); Sci(SCI_GETTEXT, len + 1, (LPARAM)buf.data()); buf.resize(len); return buf; }
void SetText(const std::string& t) { Sci(SCI_CLEARALL); Sci(SCI_ADDTEXT, t.size(), (LPARAM)t.c_str()); Sci(SCI_SETSAVEPOINT); }
void UpdateTitle() { std::wstring t = L"mif-NOTE"; if (!g_currentFile.empty()) { size_t p = g_currentFile.find_last_of(L"\\/"); t += L" - " + (p == std::wstring::npos ? g_currentFile : g_currentFile.substr(p + 1)); } SetWindowText(g_hMain, t.c_str()); }
void ApplyLineNumbers() { if (g_showLineNumbers) { Sci(SCI_SETMARGINWIDTHN, 0, 40); Sci(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER); } else { Sci(SCI_SETMARGINWIDTHN, 0, 0); } }
void ApplyWrap() { Sci(SCI_SETWRAPMODE, g_wordWrap ? SC_WRAP_WORD : SC_WRAP_NONE); }
bool AskSaveIfDirty() { if (Sci(SCI_GETMODIFY) == 0) return true; int r = MessageBox(g_hMain, L"Simpan perubahan?", L"mif-NOTE", MB_YESNOCANCEL | MB_ICONQUESTION); if (r == IDCANCEL) return false; if (r == IDYES) { SendMessage(g_hMain, WM_COMMAND, IDM_SAVE, 0); return Sci(SCI_GETMODIFY) == 0; } return true; }
void DoOpen() { if (!AskSaveIfDirty()) return; wchar_t buf[MAX_PATH] = {0}; OPENFILENAMEW ofn = {0}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = g_hMain; ofn.lpstrFilter = L"RTF (*.rtf)\0*.rtf\0Text (*.txt)\0*.txt\0HTML (*.html)\0*.html\0Semua (*.*)\0*.*\0"; ofn.lpstrFile = buf; ofn.nMaxFile = MAX_PATH; ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST; if (!GetOpenFileNameW(&ofn)) return; g_currentFile = buf; SetText(ReadFileBytes(buf)); UpdateTitle(); }
void DoSaveAs() { wchar_t buf[MAX_PATH] = {0}; OPENFILENAMEW ofn = {0}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = g_hMain; ofn.lpstrFilter = L"RTF (*.rtf)\0*.rtf\0Text (*.txt)\0*.txt\0HTML (*.html)\0*.html\0"; ofn.lpstrFile = buf; ofn.nMaxFile = MAX_PATH; ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST; if (!GetSaveFileNameW(&ofn)) return; g_currentFile = buf; WriteFileBytes(buf, GetText()); Sci(SCI_SETSAVEPOINT); UpdateTitle(); MessageBox(g_hMain, (L"Tersimpan:\n" + g_currentFile).c_str(), L"mif-NOTE", MB_OK | MB_ICONINFORMATION); }
void DoSave() { if (g_currentFile.empty()) { DoSaveAs(); return; } WriteFileBytes(g_currentFile, GetText()); Sci(SCI_SETSAVEPOINT); }
void DoFont() { CHOOSEFONTW cf = {0}; LOGFONTW lf = {0}; char face[64] = {0}; Sci(SCI_STYLEGETFONT, 0, (LPARAM)face); int sz = (int)Sci(SCI_STYLEGETSIZE, 0); MultiByteToWideChar(CP_UTF8, 0, face, -1, lf.lfFaceName, 32); lf.lfHeight = -MulDiv(sz, GetDeviceCaps(GetDC(g_hMain), LOGPIXELSY), 72); cf.lStructSize = sizeof(cf); cf.hwndOwner = g_hMain; cf.lpLogFont = &lf; cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT; cf.iPointSize = sz * 10; if (!ChooseFontW(&cf)) return; char faceA[64] = {0}; WideCharToMultiByte(CP_UTF8, 0, lf.lfFaceName, -1, faceA, 64, NULL, NULL); int newSize = cf.iPointSize / 10; for (int st = 0; st < 256; st++) { Sci(SCI_STYLESETFONT, st, (LPARAM)faceA); Sci(SCI_STYLESETSIZE, st, newSize); } }
void SetRTL(bool rtl) { Sci(SCI_SETBIDIRECTIONAL, rtl ? SC_BIDIRECTIONAL_L2R : SC_BIDIRECTIONAL_DISABLED); MessageBox(g_hMain, rtl ? L"Mode RTL aktif. Pindah keyboard ke AR dari sistem (Alt+Shift / Win+Spasi) untuk ketik Arab." : L"Mode LTR aktif. Pindah keyboard ke EN dari sistem untuk ketik Latin.", L"mif-NOTE", MB_OK | MB_ICONINFORMATION); }
LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM wp, LPARAM lp) {
  switch (m) {
    case WM_CREATE: {
      g_hSci = LoadLibraryW(L"SciLexer.dll");
      if (!g_hSci) MessageBox(h, L"SciLexer.dll tidak ketemu. Copy SciLexer.dll di samping EXE.", L"mif-NOTE", MB_OK | MB_ICONERROR);
      g_hEdit = CreateWindowExW(0, L"Scintilla", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL, 0, 0, 800, 600, h, NULL, g_hInst, NULL);
      Sci(SCI_SETLEXER, SCLEX_NULL); Sci(SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas"); Sci(SCI_STYLESETSIZE, STYLE_DEFAULT, 12); Sci(SCI_STYLECLEARALL); Sci(SCI_SETCODEPAGE, SC_CP_UTF8); Sci(SCI_SETMARGINWIDTHN, 1, 0); ApplyLineNumbers(); ApplyWrap();
      HMENU menu = CreateMenu(); HMENU mFile = CreatePopupMenu();
      AppendMenuW(mFile, MF_STRING, IDM_NEW, L"Baru"); AppendMenuW(mFile, MF_STRING, IDM_OPEN, L"Buka RTF / TXT / HTML"); AppendMenuW(mFile, MF_STRING, IDM_SAVE, L"Simpan"); AppendMenuW(mFile, MF_STRING, IDM_SAVEAS, L"Simpan Sebagai"); AppendMenuW(mFile, MF_SEPARATOR, 0, NULL); AppendMenuW(mFile, MF_STRING, IDM_EXIT, L"Keluar");
      HMENU mView = CreatePopupMenu(); AppendMenuW(mView, MF_STRING | MF_CHECKED, IDM_VIEW_LINENUM, L"Tampilkan Nomor Baris"); AppendMenuW(mView, MF_STRING | MF_CHECKED, IDM_VIEW_WRAP, L"Word Wrap");
      HMENU mFmt = CreatePopupMenu(); AppendMenuW(mFmt, MF_STRING, IDM_RTL, L"RTL ->"); AppendMenuW(mFmt, MF_STRING, IDM_LTR, L"<- LTR"); AppendMenuW(mFmt, MF_SEPARATOR, 0, NULL); AppendMenuW(mFmt, MF_STRING, IDM_FONT, L"Font...");
      AppendMenuW(menu, MF_POPUP, (UINT_PTR)mFile, L"File"); AppendMenuW(menu, MF_POPUP, (UINT_PTR)mView, L"View"); AppendMenuW(menu, MF_POPUP, (UINT_PTR)mFmt, L"Format"); SetMenu(h, menu); UpdateTitle(); break;
    }
    case WM_SIZE: if (g_hEdit) MoveWindow(g_hEdit, 0, 0, LOWORD(lp), HIWORD(lp), TRUE); break;
    case WM_COMMAND: { int id = LOWORD(wp); switch (id) { case IDM_NEW: if (AskSaveIfDirty()) { SetText(""); g_currentFile = L""; UpdateTitle(); } break; case IDM_OPEN: DoOpen(); break; case IDM_SAVE: DoSave(); break; case IDM_SAVEAS: DoSaveAs(); break; case IDM_EXIT: SendMessage(h, WM_CLOSE, 0, 0); break; case IDM_VIEW_LINENUM: g_showLineNumbers = !g_showLineNumbers; ApplyLineNumbers(); break; case IDM_VIEW_WRAP: g_wordWrap = !g_wordWrap; ApplyWrap(); break; case IDM_RTL: SetRTL(true); break; case IDM_LTR: SetRTL(false); break; case IDM_FONT: DoFont(); break; } break; }
    case WM_CLOSE: if (!AskSaveIfDirty()) return 0; DestroyWindow(h); break;
    case WM_DESTROY: PostQuitMessage(0); break;
    default: return DefWindowProc(h, wp, lp);
  }
  return 0;
}
int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, PWSTR, int nCmd) {
  g_hInst = hi; WNDCLASSW wc = {0}; wc.lpfnWndProc = WndProc; wc.hInstance = hi; wc.lpszClassName = L"mif-NOTE"; wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hIcon = LoadIcon(hi, MAKEINTRESOURCE(101)); if (!wc.hIcon) wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); RegisterClassW(&wc);
  g_hMain = CreateWindowW(L"mif-NOTE", L"mif-NOTE", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 900, 600, NULL, NULL, hi, NULL);
  ShowWindow(g_hMain, nCmd); UpdateWindow(g_hMain); MSG msg; while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); } if (g_hSci) FreeLibrary(g_hSci); return 0;
}
