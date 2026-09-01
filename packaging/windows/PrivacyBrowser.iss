; Inno Setup script for the Windows installer (PLAN.md 47).
;
; Deliberately boring: it copies files, makes a Start-menu entry, and that is
; all. No service, no scheduled task, no updater, no registry beyond what an
; uninstaller needs, and no "send usage data" checkbox - there is nothing to
; send it to (PLAN.md 47, 48).
;
; Build after scripts\package-windows.ps1 has produced dist\...-win64\:
;   iscc /DSourceDir=..\..\dist\PrivacyBrowser-0.1.0-win64 /DAppVersion=0.1.0 PrivacyBrowser.iss

#ifndef SourceDir
  #error Define SourceDir: the staged folder produced by package-windows.ps1
#endif
#ifndef AppVersion
  #define AppVersion "0.1.0"
#endif

[Setup]
AppName=Privacy Browser
AppVersion={#AppVersion}
DefaultDirName={autopf}\Privacy Browser
DefaultGroupName=Privacy Browser
UninstallDisplayIcon={app}\PrivacyBrowser.exe
OutputBaseFilename=PrivacyBrowser-{#AppVersion}-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
; A per-user install by default, so administrator rights are not required.
PrivilegesRequiredOverridesAllowed=dialog

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\Privacy Browser"; Filename: "{app}\PrivacyBrowser.exe"
Name: "{group}\Privacy policy"; Filename: "{app}\privacy-policy.md"
Name: "{autodesktop}\Privacy Browser"; Filename: "{app}\PrivacyBrowser.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Shortcuts:"

[Run]
Filename: "{app}\PrivacyBrowser.exe"; Description: "Start Privacy Browser"; \
    Flags: nowait postinstall skipifsilent

; Uninstalling removes what was installed. The browser writes nothing else
; unless the user turned on "Remember settings"; that file is left alone
; because it is theirs, and the message below says where it is.
[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Messages]
FinishedLabel=Privacy Browser is installed.%n%nIt saves no history, keeps no cookies between sessions, and sends nothing to its developers.
