# Find paths to required dependencies in registry then writes in in deps.json
[string]$win32ntIncludeRegKeyPath = "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Components\000D7F8DE43B4822A7A914222CC3D61F"
[string]$win32ntLibRegKeyPath     = "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Components\0010F43C69F56E68655D0AD58CD3D890"


$win32ntIncludeEntryName = get-item -path $win32ntIncludeRegKeyPath | Select-Object -ExpandProperty Property
if ($win32ntIncludeEntryName.gettype().name -ne "String") {
    $win32ntIncludeEntryName = $win32ntIncludeEntryName[0] 
    # Picking a specific "best" present version isn't worth the hustle, any will do
}
[String]$win32ntIncludePath = Get-ItemPropertyValue -path $win32ntIncludeRegKeyPath -name $win32ntIncludeEntryName;
[void]($win32ntIncludePath += "\..\..\..\..")


$win32ntLibEntryName     = get-item -path $win32ntLibRegKeyPath | Select-Object -ExpandProperty Property
if ($win32ntLibEntryName.GetType().Name -ne "String") {
    $win32ntLibEntryName = $win32ntLibEntryName[0]
} 
[void](
[string]$win32ntLibPath     = Get-itempropertyvalue -path $win32ntLibRegKeyPath -name $win32ntLibEntryName)
[void]($win32ntLibPath+="\..\..\..")


[void](
[string]$vulkanIncludePath   = "$env:vk_sdk_path\Include")
[void](
[string]$vulkanLibPath       = "$env:vk_sdk_path\Lib")

Write-Output @"

Your Win32 API include path is:  $win32ntincludepath
Your Win32 API lib path is:      $win32ntlibpath
Your Vulkan API include path is: $vulkanincludepath
Yout Vulkan API lib path is:     $vulkanlibpath

They will now be written in deps.json to later be used by CMakeLists.txt.
You can access Win32 include path in source fild with WIN32INCLUDE macro,
and Vulkan include path with VULKANINCLUDE macro.

"@

@{
    win32nt = @{
        Include = "$win32ntincludepath"
        Lib     = "$win32ntLibPath"
    }
    vulkan = @{
        Include = $vulkanIncludePath
        Lib     = $vulkanLibPath
    }
} | ConvertTo-Json | out-file $PSScriptRoot/deps.json 