param([string]$OutputPath = (Join-Path $PSScriptRoot 'session-identity.json'))
$ErrorActionPreference = 'Stop'
$instances = @(Get-Process -Name 'HD-Player' -ErrorAction SilentlyContinue | ForEach-Object {
    [ordered]@{
        process_id = $_.Id
        process_created_filetime = $_.StartTime.ToUniversalTime().ToFileTimeUtc().ToString()
        window_title = $_.MainWindowTitle
        executable = $_.Path
    }
})
if ($instances.Count -eq 0) { throw 'No HD-Player process is running.' }
ConvertTo-Json -InputObject $instances -Depth 4 | Set-Content -LiteralPath $OutputPath -Encoding UTF8
Write-Output "Saved emulator identities to $OutputPath"
Write-Output 'These are host identities only. Guest mappings and game fields still require capture.'
