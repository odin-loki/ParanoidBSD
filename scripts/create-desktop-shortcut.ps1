# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
$WshShell = New-Object -ComObject WScript.Shell
$Desktop = [Environment]::GetFolderPath('Desktop')
$Shortcut = $WshShell.CreateShortcut("$Desktop\PBSD Migration.lnk")
$Shortcut.TargetPath = "C:\Users\odinl\OneDrive\Desktop\Operating System\Watch-PBSD.bat"
$Shortcut.WorkingDirectory = "C:\Users\odinl\OneDrive\Desktop\Operating System"
$Shortcut.WindowStyle = 1
$Shortcut.Description = "PBSD migration progress monitor"
$Shortcut.Save()
Write-Output "Shortcut: $Desktop\PBSD Migration.lnk"
