mif-NOTE C++ Edition - GPL v3
Mesin editor: Scintilla (sama seperti Notepad++)
Bahasa: C++ + Win32 API
Lisensi: GPL v3 (ikut Notepad++)

Fitur mif-NOTE:
- Buka / Simpan RTF, TXT, HTML, CFG, code (via teks UTF-8)
- Multi-font via Scintilla styles
- RTL / LTR (Bidi, keyboard TIDAK diubah, user atur dari sistem Alt+Shift / Win+Spasi)
- View > Tampilkan Nomor Baris (margin Scintilla, TIDAK melenceng, TIDAK nambah waktu Word Wrap)
- View > Word Wrap
- Format > Font... (dialog Windows, keyboard selalu EN)
- Klik kanan Cut/Copy/Paste via Scintilla
- Pencet X tanya Save Yes/No/Cancel
- Icon Notepad tertanam di EXE + jendela (via .rc)
- Judul pendek: mif-NOTE

Cara build (GitHub, seperti APK):
1. Upload folder ini ke repo GitHub baru, misal mif-note-cpp
2. Buka Actions > Build mif-NOTE C++ > Run workflow
3. Download artifact mif-note-cpp -> ada Mif-NOTE.exe + SciLexer.dll
4. Copy 2 file itu ke 1 folder di Pavilion / EeePC, double-klik Mif-NOTE.exe
   (SciLexer.dll harus di samping EXE, itu mesin Scintilla-nya)

Catatan GPL:
- Project ini GPL v3, ikut Notepad++ / Scintilla.
- Kalau dibagikan lagi, tetap harus GPL.

Perbedaan vs mif-NOTE C# lama:
- C#: RichTextBox, nomor baris digambar manual pakai Panel -> gampang melenceng.
- C++ ini: Scintilla, nomor baris bawaan margin -> tidak melenceng, Word Wrap tidak nambah nomor.
- C#: build di Pavilion pakai dotnet.
- C++ ini: build di GitHub pakai windows-latest + CMake.
