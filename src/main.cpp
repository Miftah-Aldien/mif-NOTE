// mif-NOTE C++ - GPL v3 - TOOLBAR FONT (versi aman, pasti compile)
// Mesin: RichEdit bawaan Windows (Msftedit.dll)
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <richedit.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
HINSTANCE g_hInst;
HWND g_hMain = NULL, g_hEdit = NULL, g_hLine = NULL;
HWND g_hToolbar = NULL, g_hFontName = NULL, g_hFontSize = NULL;
std::wstring g_currentFile = L"";
bool g_showLine = true, g_wordWrap = true;
int g_lastLineCount = -1;
HKL g_hklSavedFont = NULL;
#define ID_EDIT 500
#define ID_FONTNAME 501
#define ID_FONTSIZE 502
#define IDM_NEW 1001
#define IDM_OPEN 1002
#define IDM_SAVE 1003
#define IDM_SAVEAS 1004
#define IDM_EXIT 1005
#define IDM_VIEW_LINE 2001
#define IDM_VIEW_WRAP 2002
#define IDM_RTL 3001
#define IDM_LTR 3002
#define IDM_SELECTALL 3005
#define IDM_CUT 3006
#define IDM_COPY 3007
#define IDM_PASTE 3008

std::wstring GetEditText() {
  int len = GetWindowTextLengthW(g_hEdit);
  std::wstring s(len + 1, 0);
  GetWindowTextW(g_hEdit, &s[0], len + 1);
  s.resize(len);
  return s;
}
void UpdateTitle() { SetWindowTextW(g_hMain, L"mif-NOTE"); }
bool IsDirty() { return (bool)SendMessageW(g_hEdit, EM_GETMODIFY, 0, 0); }
void UpdateLineNumbers() {
  if (!g_showLine) return;
  int lines = (int)SendMessageW(g_hEdit, EM_GETLINECOUNT, 0, 0);
  if (lines == g_lastLineCount) return;
  g_lastLineCount = lines;
  std::wstring s;
  for (int i = 1; i <= lines; i++) { s += std::to_wstring(i); s += L"\r\n"; }
  SendMessageW(g_hLine, WM_SETREDRAW, FALSE, 0);
  SetWindowTextW(g_hLine, s.c_str());
  SendMessageW(g_hLine, WM_SETREDRAW, TRUE, 0);
  InvalidateRect(g_hLine, NULL, TRUE);
}
struct StreamCtx { std::string data; size_t pos; };
static DWORD CALLBACK StreamIn(DWORD_PTR dw, LPBYTE b, LONG n, LONG* r) {
  StreamCtx* c = (StreamCtx*)dw; size_t rem = c->data.size() - c->pos;
  size_t take = rem < (size_t)n ? rem : (size_t)n;
  memcpy(b, c->data.data() + c->pos, take); c->pos += take; *r = (LONG)take; return 0;
}
static DWORD CALLBACK StreamOut(DWORD_PTR dw, LPBYTE b, LONG n, LONG* r) {
  StreamCtx* c = (StreamCtx*)dw; c->data.append((char*)b, n); *r = n; return 0;
}
bool EndsWith(const std::wstring& s, const wchar_t* ext) {
  if (s.size() < wcslen(ext)) return false;
  std::wstring e = s.substr(s.size() - wcslen(ext));
  for (auto& ch : e) ch = towlower(ch);
  std::wstring x = ext; for (auto& ch : x) ch = towlower(ch);
  return e == x;
}
void SetTextFromFile(const std::wstring& path, const std::string& bytes) {
  if (EndsWith(path, L".rtf")) {
    EDITSTREAM es = {0}; StreamCtx ctx; ctx.data = bytes;
    es.dwCookie = (DWORD_PTR)&ctx; es.pfnCallback = StreamIn;
    SendMessageW(g_hEdit, EM_STREAMIN, SF_RTF, (LPARAM)&es);
  } else {
    int n = MultiByteToWideChar(CP_UTF8, 0, bytes.c_str(), (int)bytes.size(), NULL, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, bytes.c_str(), (int)bytes.size(), &w[0], n);
    SetWindowTextW(g_hEdit, w.c_str());
  }
  SendMessageW(g_hEdit, EM_SETMODIFY, FALSE, 0);
  g_lastLineCount = -1; UpdateLineNumbers();
}
std::string GetBytesForSave() {
  if (EndsWith(g_currentFile, L".rtf")) {
    EDITSTREAM es = {0}; StreamCtx ctx;
    es.dwCookie = (DWORD_PTR)&ctx; es.pfnCallback = StreamOut;
    SendMessageW(g_hEdit, EM_STREAMOUT, SF_RTF, (LPARAM)&es);
    return ctx.data;
  }
  std::wstring w = GetEditText();
  int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
  std::string s(n, 0);
  WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, NULL, NULL);
  return s;
}
bool AskSaveIfDirty() {
  if (!IsDirty()) return true;
  int r = MessageBoxW(g_hMain, L"Simpan perubahan?", L"mif-NOTE", MB_YESNOCANCEL | MB_ICONQUESTION);
  if (r == IDCANCEL) return false;
  if (r == IDYES) { SendMessageW(g_hMain, WM_COMMAND, IDM_SAVE, 0); return !IsDirty(); }
  return true;
}
void DoSaveAs() {
  wchar_t buf[MAX_PATH] = {0};
  if (!g_currentFile.empty()) wcscpy_s(buf, g_currentFile.c_str());
  OPENFILENAMEW ofn = {0}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = g_hMain;
  ofn.lpstrFilter = L"RTF (*.rtf)\0*.rtf\0Text (*.txt)\0*.txt\0HTML (*.html)\0*.html\0Semua (*.*)\0*.*\0";
  ofn.lpstrFile = buf; ofn.nMaxFile = MAX_PATH; ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
  if (!GetOpenFileNameW(&ofn)) return;
  g_currentFile = buf;
  std::string data = GetBytesForSave();
  std::ofstream f(buf, std::ios::binary); f.write(data.data(), data.size());
  SendMessageW(g_hEdit, EM_SETMODIFY, FALSE, 0); UpdateTitle();
}
void DoSave() { if (g_currentFile.empty()) { DoSaveAs(); return; } std::string d = GetBytesForSave(); std::ofstream f(g_currentFile, std::ios::binary); f.write(d.data(), d.size()); SendMessageW(g_hEdit, EM_SETMODIFY, FALSE, 0); }
void DoOpen() {
  if (!AskSaveIfDirty()) return;
  wchar_t buf[MAX_PATH] = {0}; OPENFILENAMEW ofn = {0}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = g_hMain;
  ofn.lpstrFilter = L"RTF (*.rtf)\0*.rtf\0Text (*.txt)\0*.txt\0HTML (*.html)\0*.html\0Semua (*.*)\0*.*\0";
  ofn.lpstrFile = buf; ofn.nMaxFile = MAX_PATH; ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  if (!GetOpenFileNameW(&ofn)) return;
  std::ifstream in(buf, std::ios::binary); std::stringstream ss; ss << in.rdbuf();
  g_currentFile = buf; SetTextFromFile(buf, ss.str()); UpdateTitle();
}
void DoNew() { if (!AskSaveIfDirty()) return; g_currentFile.clear(); SetWindowTextW(g_hEdit, L""); SendMessageW(g_hEdit, EM_SETMODIFY, FALSE, 0); UpdateTitle(); g_lastLineCount = -1; UpdateLineNumbers(); }
void ApplyWrap() {
  if (g_wordWrap) SendMessageW(g_hEdit, EM_SETTARGETDEVICE, 0, 0);
  else { RECT rc; GetClientRect(g_hEdit, &rc); SendMessageW(g_hEdit, EM_SETTARGETDEVICE, 0, (LPARAM)(rc.right * 10)); }
}
void ApplyRTL(bool rtl) {
  // Versi aman yang dulu berhasil compile: WS_EX_RTLREADING (seluruh kontrol)
  // PARAFORMAT2 dihapus karena bikin gagal compile di GitHub
  LONG ex = GetWindowLongW(g_hEdit, GWL_EXSTYLE);
  if (rtl) SetWindowLongW(g_hEdit, GWL_EXSTYLE, ex | WS_EX_RTLREADING);
  else SetWindowLongW(g_hEdit, GWL_EXSTYLE, ex & ~WS_EX_RTLREADING);
  SetWindowPos(g_hEdit, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}
void ApplyFontFromToolbar() {
  if (!g_hFontName || !g_hFontSize) return;
  wchar_t face[64] = {0};
  GetWindowTextW(g_hFontName, face, 63);
  wchar_t szTxt[16] = {0};
  GetWindowTextW(g_hFontSize, szTxt, 15);
  int pt = (int)wcstol(szTxt, NULL, 10);
  if (face[0] == 0) return;
  if (pt <= 0) pt = 11;
  if (pt > 200) pt = 200;
  CHARFORMAT2W ncf = {0}; ncf.cbSize = sizeof(ncf);
  ncf.dwMask = CFM_FACE | CFM_SIZE;
  wcscpy_s(ncf.szFaceName, face);
  ncf.yHeight = pt * 20;
  CHARRANGE cr = {0};
  SendMessageW(g_hEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
  bool adaBlok = (cr.cpMax != cr.cpMin);
  if (adaBlok) {
    SendMessageW(g_hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&ncf);
  } else {
    SendMessageW(g_hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&ncf);
    SendMessageW(g_hEdit, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&ncf);
  }
  SetFocus(g_hEdit);
}
int CALLBACK EnumFontProc(const LOGFONTW* lf, const TEXTMETRICW* tm, DWORD type, LPARAM lp) {
  (void)tm; (void)type; (void)lp;
  if (lf->lfFaceName[0] == L'@') return 1;
  if (SendMessageW(g_hFontName, CB_FINDSTRINGEXACT, 0, (LPARAM)lf->lfFaceName) == CB_ERR) {
    SendMessageW(g_hFontName, CB_ADDSTRING, 0, (LPARAM)lf->lfFaceName);
  }
  return 1;
}
void FillFontToolbar() {
  HDC hdc = GetDC(g_hMain);
  LOGFONTW lf = {0}; lf.lfCharSet = DEFAULT_CHARSET;
  EnumFontFamiliesExW(hdc, &lf, EnumFontProc, 0, 0);
  ReleaseDC(g_hMain, hdc);
  const wchar_t* sizes[] = { L"8", L"9", L"10", L"11", L"12", L"14", L"16", L"18", L"20", L"24", L"28", L"32", L"36", L"48", L"72" };
  for (int i = 0; i < 16; i++) SendMessageW(g_hFontSize, CB_ADDSTRING, 0, (LPARAM)sizes[i]);
  SendMessageW(g_hFontName, CB_SELECTSTRING, 0, (LPARAM)L"Calibri");
  if (SendMessageW(g_hFontName, CB_GETCURSEL, 0, 0) == CB_ERR)
    SendMessageW(g_hFontName, CB_SETCURSEL, 0, 0);
  SendMessageW(g_hFontSize, CB_SELECTSTRING, 0, (LPARAM)L"11");
}
void Resize() {
  RECT rc; GetClientRect(g_hMain, &rc);
  int th = 30;
  int lw = g_showLine ? 42 : 0;
  if (g_hToolbar) MoveWindow(g_hToolbar, 0, 0, rc.right, th, TRUE);
  if (g_hLine) MoveWindow(g_hLine, 0, th, lw, rc.bottom - th, TRUE);
  if (g_hEdit) MoveWindow(g_hEdit, lw, th, rc.right - lw, rc.bottom - th, TRUE);
  ShowWindow(g_hLine, g_showLine ? SW_SHOW : SW_HIDE);
}
LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM wp, LPARAM lp) {
  if (m == WM_SIZE) { Resize(); return 0; }
  if (m == WM_CLOSE) { if (!AskSaveIfDirty()) return 0; DestroyWindow(h); return 0; }
  if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
  if (m == WM_COMMAND) {
    if (LOWORD(wp) == ID_FONTNAME || LOWORD(wp) == ID_FONTSIZE) {
      if (HIWORD(wp) == CBN_SETFOCUS) {
        g_hklSavedFont = GetKeyboardLayout(0);
        HKL hklEN = LoadKeyboardLayoutW(L"00000409", KLF_ACTIVATE);
        ActivateKeyboardLayout(hklEN, 0);
        return 0;
      }
      if (HIWORD(wp) == CBN_KILLFOCUS) {
        if (g_hklSavedFont) ActivateKeyboardLayout(g_hklSavedFont, 0);
        return 0;
      }
      if (HIWORD(wp) == CBN_SELCHANGE) { ApplyFontFromToolbar(); return 0; }
    }
    if (LOWORD(wp) == ID_EDIT && HIWORD(wp) == EN_CHANGE) { UpdateLineNumbers(); return 0; }
    switch (LOWORD(wp)) {
      case IDM_NEW: DoNew(); return 0;
      case IDM_OPEN: DoOpen(); return 0;
      case IDM_SAVE: DoSave(); return 0;
      case IDM_SAVEAS: DoSaveAs(); return 0;
      case IDM_EXIT: SendMessageW(h, WM_CLOSE, 0, 0); return 0;
      case IDM_VIEW_LINE: g_showLine = !g_showLine; g_lastLineCount = -1; Resize(); UpdateLineNumbers(); return 0;
      case IDM_VIEW_WRAP: g_wordWrap = !g_wordWrap; ApplyWrap(); return 0;
      case IDM_RTL: ApplyRTL(true); return 0;
      case IDM_LTR: ApplyRTL(false); return 0;
      case IDM_SELECTALL: SendMessageW(g_hEdit, EM_SETSEL, 0, -1); return 0;
      case IDM_CUT: SendMessageW(g_hEdit, WM_CUT, 0, 0); return 0;
      case IDM_COPY: SendMessageW(g_hEdit, WM_COPY, 0, 0); return 0;
      case IDM_PASTE: SendMessageW(g_hEdit, WM_PASTE, 0, 0); return 0;
    }
  }
  return DefWindowProcW(h, m, wp, lp);
}
int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, PWSTR, int nCmd) {
  g_hInst = hi;
  LoadLibraryW(L"Msftedit.dll");
  WNDCLASSW wc = {0}; wc.lpfnWndProc = WndProc; wc.hInstance = hi; wc.lpszClassName = L"MifNote";
  wc.hCursor = LoadCursorW(NULL, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.hIcon = LoadIconW(hi, MAKEINTRESOURCEW(101));
  RegisterClassW(&wc);
  g_hMain = CreateWindowW(L"MifNote", L"mif-NOTE", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hi, NULL);
  HMENU bar = CreateMenu(), file = CreateMenu(), view = CreateMenu(), fmt = CreateMenu();
  AppendMenuW(file, MF_STRING, IDM_NEW, L"&Baru\tCtrl+N");
  AppendMenuW(file, MF_STRING, IDM_OPEN, L"&Buka...\tCtrl+O");
  AppendMenuW(file, MF_STRING, IDM_SAVE, L"&Simpan\tCtrl+S");
  AppendMenuW(file, MF_STRING, IDM_SAVEAS, L"Simpan &Sebagai...");
  AppendMenuW(file, MF_SEPARATOR, 0, NULL);
  AppendMenuW(file, MF_STRING, IDM_EXIT, L"&Keluar");
  AppendMenuW(view, MF_STRING, IDM_VIEW_LINE, L"Nomor &Baris");
  AppendMenuW(view, MF_STRING, IDM_VIEW_WRAP, L"&Word Wrap");
  AppendMenuW(fmt, MF_STRING, IDM_RTL, L"Arab / &RTL");
  AppendMenuW(fmt, MF_STRING, IDM_LTR, L"Latin / &LTR");
  AppendMenuW(bar, MF_POPUP, (UINT_PTR)file, L"&File");
  AppendMenuW(bar, MF_POPUP, (UINT_PTR)view, L"&Tampilan");
  AppendMenuW(bar, MF_POPUP, (UINT_PTR)fmt, L"F&ormat");
  SetMenu(g_hMain, bar);
  g_hToolbar = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 0, 0, 800, 30, g_hMain, NULL, hi, NULL);
  CreateWindowExW(0, L"STATIC", L"Font:", WS_CHILD | WS_VISIBLE, 5, 7, 34, 16, g_hToolbar, NULL, hi, NULL);
  g_hFontName = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_SORT | WS_VSCROLL, 42, 4, 200, 200, g_hToolbar, (HMENU)(UINT_PTR)ID_FONTNAME, hi, NULL);
  CreateWindowExW(0, L"STATIC", L"Size:", WS_CHILD | WS_VISIBLE, 250, 7, 30, 16, g_hToolbar, NULL, hi, NULL);
  g_hFontSize = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL, 285, 4, 60, 200, g_hToolbar, (HMENU)(UINT_PTR)ID_FONTSIZE, hi, NULL);
  FillFontToolbar();
  g_hLine = CreateWindowExW(0, L"EDIT", L"1\r\n", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_RIGHT | ES_AUTOVSCROLL, 0, 30, 42, 570, g_hMain, NULL, hi, NULL);
  g_hEdit = CreateWindowExW(0, MSFTEDIT_CLASS, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | ES_SAVESEL, 42, 30, 758, 570, g_hMain, (HMENU)(UINT_PTR)ID_EDIT, hi, NULL);
  SendMessageW(g_hEdit, EM_SETEVENTMASK, 0, ENM_CHANGE);
  SendMessageW(g_hLine, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
  ShowWindow(g_hMain, nCmd); UpdateWindow(g_hMain);
  SetFocus(g_hEdit);
  MSG msg; while (GetMessageW(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  return 0;
}
