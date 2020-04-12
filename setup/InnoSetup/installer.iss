; Input defines:
; - Version (0.8)
; - BinDir (C:\tmp\build\meson-player\dist)
; - OutDir (C:\tmp\build\meson-player\installer)

#define Name "mesonplayer"
#define Title "Meson Player"
#define Arch "x86_64"
#define URL "https://mesonplayer.alkatrazstudio.net"
#define Publisher "Alkatraz Studio"
#define PublisherURL "https://alkatrazstudio.net/"
#define CopyrightYear "2012"


[Setup]
AppId={{40F11DC1-584D-4BB9-BF50-6D3F5700E61B}
AppName={#Title}
AppVersion={#Version}
VersionInfoVersion={#Version}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
VersionInfoCompany={#Publisher}
AppCopyright=(c) {#Publisher}, {#CopyrightYear}
AppSupportURL={#URL}
AppUpdatesURL={#URL}
DefaultDirName={autopf}\{#Title}
DefaultGroupName={#Title}
AllowNoIcons=yes
LicenseFile={#SourcePath}\LICENSE.SETUP.TXT
InfoAfterFile={#SourcePath}\README.SETUP.TXT
OutputDir={#OutDir}
OutputBaseFilename={#Name}-v{#Version}-win-{#Arch}-setup
SetupIconFile={#BinDir}\icons\app.ico
Compression=lzma2/ultra64
SolidCompression=yes
AppMutex={#Name}
UninstallDisplayIcon={app}\icons\icon.ico
UninstallDisplayName={#Title} v{#Version}
SourceDir={#BinDir}
ShowLanguageDialog=no
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
UsePreviousTasks=yes


[code]
procedure DeleteOldStartMenuEntries;
var
  entry: String;
begin
  entry := ExpandConstant('{group}');
  if DirExists(entry) then begin
    DelTree(entry, true, true, true);
  end
end;

// https://stackoverflow.com/a/2099805
/////////////////////////////////////////////////////////////////////
function GetUninstallString(): String;
var
  sUnInstPath: String;
  sUnInstallString: String;
begin
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#emit SetupSetting("AppId")}_is1');
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Result := sUnInstallString;
end;

/////////////////////////////////////////////////////////////////////
function IsUpgrade(): Boolean;
begin
  Result := (GetUninstallString() <> '');
end;

/////////////////////////////////////////////////////////////////////
function UnInstallOldVersion(): Integer;
var
  sUnInstallString: String;
  iResultCode: Integer;
begin
// Return Values:
// 1 - uninstall string is empty
// 2 - error executing the UnInstallString
// 3 - successfully executed the UnInstallString

  // default return value
  Result := 0;

  // get the uninstall string of the old app
  sUnInstallString := GetUninstallString();
  if sUnInstallString <> '' then begin
    sUnInstallString := RemoveQuotes(sUnInstallString);
    if Exec(sUnInstallString, '/VERYSILENT /NORESTART /SUPPRESSMSGBOXES', '', SW_HIDE, ewWaitUntilTerminated, iResultCode) then
      Result := 3
    else
      Result := 2;
  end else
    Result := 1;
end;

/////////////////////////////////////////////////////////////////////
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep=ssInstall) then
  begin
    if (IsUpgrade()) then
    begin
      UnInstallOldVersion();
    end;
    DeleteOldStartMenuEntries;
  end;
end;


[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl,{#SourcePath}\langs\en.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl,{#SourcePath}\langs\ru.isl"


[Files]
Source: "*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion isreadme


[Icons]
Name: "{group}\{#Title}"; Filename: "{app}\{#Name}.exe"; Flags: excludefromshowinnewinstall
Name: "{group}\{cm:ProgramOnTheWeb,{#Title}}"; Filename: "{#URL}"; Flags: excludefromshowinnewinstall
Name: "{group}\{cm:ProgramOnTheWeb,{#Publisher}}"; Filename: "{#PublisherURL}"; Flags: excludefromshowinnewinstall
Name: "{group}\{cm:UninstallProgram,{#Title}}"; Filename: "{uninstallexe}"; Flags: excludefromshowinnewinstall
Name: "{group}\tools\Enable Autorun"; Filename: "{app}\EnableAutorun.bat"; Flags: excludefromshowinnewinstall
Name: "{group}\tools\Disable Autorun"; Filename: "{app}\DisableAutorun.bat"; Flags: excludefromshowinnewinstall


[Registry]
; open folders in Meson Player
Root: HKA; Subkey: "Software\Classes\Directory\shell\{#StringChange(Title, ' ', '')}"; ValueType: string; ValueName: ""; ValueData: "Open in {#Title}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\Directory\Shell\{#StringChange(Title, ' ', '')}\Command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#Name}.exe"" ""%1"""

; open playlists in Meson Player
Root: HKA; Subkey:"Software\Classes\.asx\shell\{#StringChange(Title, ' ', '')}"; ValueType: string; ValueName: ""; ValueData: "Open in {#Title}"; Flags: uninsdeletekey
Root: HKA; Subkey:"Software\Classes\.asx\shell\{#StringChange(Title, ' ', '')}\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#Name}.exe"" ""%1"""
Root: HKA; Subkey:"Software\Classes\.m3u\shell\{#StringChange(Title, ' ', '')}"; ValueType: string; ValueName: ""; ValueData: "Open in {#Title}"; Flags: uninsdeletekey
Root: HKA; Subkey:"Software\Classes\.m3u\shell\{#StringChange(Title, ' ', '')}\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#Name}.exe"" ""%1"""
Root: HKA; Subkey:"Software\Classes\.m3u8\shell\{#StringChange(Title, ' ', '')}"; ValueType: string; ValueName: ""; ValueData: "Open in {#Title}"; Flags: uninsdeletekey
Root: HKA; Subkey:"Software\Classes\.m3u8\shell\{#StringChange(Title, ' ', '')}\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#Name}.exe"" ""%1"""
Root: HKA; Subkey:"Software\Classes\.xspf\shell\{#StringChange(Title, ' ', '')}"; ValueType: string; ValueName: ""; ValueData: "Open in {#Title}"; Flags: uninsdeletekey
Root: HKA; Subkey:"Software\Classes\.xspf\shell\{#StringChange(Title, ' ', '')}\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#Name}.exe"" ""%1"""
Root: HKA; Subkey:"Software\Classes\.pls\shell\{#StringChange(Title, ' ', '')}"; ValueType: string; ValueName: ""; ValueData: "Open in {#Title}"; Flags: uninsdeletekey
Root: HKA; Subkey:"Software\Classes\.pls\shell\{#StringChange(Title, ' ', '')}\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#Name}.exe"" ""%1"""
Root: HKA; Subkey:"Software\Classes\.wpl\shell\{#StringChange(Title, ' ', '')}"; ValueType: string; ValueName: ""; ValueData: "Open in {#Title}"; Flags: uninsdeletekey
Root: HKA; Subkey:"Software\Classes\.wpl\shell\{#StringChange(Title, ' ', '')}\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#Name}.exe"" ""%1"""
Root: HKA; Subkey:"Software\Classes\.cue\shell\{#StringChange(Title, ' ', '')}"; ValueType: string; ValueName: ""; ValueData: "Open in {#Title}"; Flags: uninsdeletekey
Root: HKA; Subkey:"Software\Classes\.cue\shell\{#StringChange(Title, ' ', '')}\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#Name}.exe"" ""%1"""


[Tasks]
Name: enableautorun; Description: "{cm:EnableAutorun}"


[Run]
Filename: "{app}\{#Name}.exe"; Description: "{cm:LaunchProgram,{#StringChange(Title, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\EnableAutorun.bat"; Description: "{cm:EnableAutorun}"; Tasks: enableautorun
Filename: "{app}\DisableAutorun.bat"; Tasks: not enableautorun
Filename: "{app}\manual\index.html"; Description: "{cm:OpenManual}"; Flags: nowait postinstall skipifsilent unchecked shellexec
