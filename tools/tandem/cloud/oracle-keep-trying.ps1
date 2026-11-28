# Project Tandem: keeps asking Oracle for the free server computer until it gets one
# (docs/TANDEM_ORACLE_SETUP.md, step 4 - when every availability domain says "Out of capacity").
#
# Brennan 2026-11-27 20:18: "is there a possible way for you to setup a script so that you can continue trying to
# create the instance for oracle, so that i dont have too?"
#
# It runs on your own PC: Oracle's tool ("oci") has to be installed first; the first time, the script asks for an
# Oracle API key (it says how). Steps: docs/TANDEM_ORACLE_SETUP.md, "Let a script keep trying". Start it with
# oracle-keep-trying.bat (double-click).
# It only ever asks for the Always Free shape (VM.Standard.A1.Flex, 1 OCPU, 6 GB) and stops as soon as one exists,
# so it can't make a second computer or one that costs money.
#
# Every try goes in oracle-keep-trying.log next to this file. When it gets the computer it beeps, shows a box, and
# writes the address into oracle-computer.txt. Close the window to stop it.

param([int]$WaitSeconds = 60)

$ErrorActionPreference = 'Continue'
$env:OCI_CLI_SUPPRESS_FILE_PERMISSIONS_WARNING = 'True'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$name = 'Wallys Journey'
$shape = 'VM.Standard.A1.Flex'
$log = Join-Path $here 'oracle-keep-trying.log'
$keyFile = Join-Path $here 'wallys-key'

function Say([string]$text)
{
    $line = (Get-Date -Format 'yyyy-MM-dd HH:mm:ss') + '  ' + $text
    Write-Host $line
    Add-Content -Path $log -Value $line
}

# (keys pressed or pasted earlier - a pasted text's last Enter - mustn't answer "Press Enter to close")
function Wait-Enter([string]$prompt)
{
    try { $Host.UI.RawUI.FlushInputBuffer() } catch { }
    Read-Host $prompt | Out-Null
}

function Stop-Here([string]$text)
{
    Say $text
    Wait-Enter 'Press Enter to close'
    exit 1
}

# Oracle's program itself (not a PowerShell function of the same name - 2026-11-27: one called itself)
$ociExe = Get-Command oci -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1

# runs Oracle's oci; gives back what it printed as one piece of text, and whether it worked
function Invoke-Oci([string[]]$OciArgs)
{
    $out = & $ociExe.Source @OciArgs 2>&1 | ForEach-Object { "$_" }
    return @{ Ok = ($LASTEXITCODE -eq 0); Text = ($out -join "`n") }
}

# runs oci and reads its answer (JSON); $null if it didn't work
function Get-OciData([string[]]$OciArgs)
{
    $r = Invoke-Oci $OciArgs
    if (-not $r.Ok) { return $null }
    $start = $r.Text.IndexOf('{')
    if ($start -lt 0) { return @() }
    return ($r.Text.Substring($start) | ConvertFrom-Json).data
}

Say '---- Wally''s Journey: asking Oracle for the free server computer ----'

# 1. Oracle's tool installed and signed in
if ($ociExe -eq $null)
{
    Stop-Here 'Oracle''s tool "oci" isn''t installed yet (or this window was open before it was): see step A of "Let a script keep trying".'
}
$config = Join-Path $env:USERPROFILE '.oci\config'
if (-not (Test-Path $config))
{
    # (first time: this PC gets a key to your Oracle account - Oracle's own "Configuration file preview", with the
    # downloaded key put next to it)
    Say 'First time on this PC: connecting it to your Oracle account.'
    Write-Host ''
    Write-Host '  1. On the Oracle website: your profile picture (top right) > My profile > Tokens and keys'
    Write-Host '     (or "API keys") > Add API key.'
    Write-Host '  2. Pick "Generate API key pair", press "Download private key", then press "Add".'
    Write-Host '  3. Copy all the text in the "Configuration file preview" box, paste it here (right-click),'
    Write-Host '     then press Enter once more on an empty line.'
    Write-Host ''
    $lines = @()
    while ($true)
    {
        $l = Read-Host
        if ($l -eq $null) { break }
        $l = $l.Trim()
        if ($l -eq '') { if ($lines.Count -gt 0) { break } else { continue } }
        $lines += $l
    }
    if (-not ($lines -match '^tenancy\s*=') -or -not ($lines -match '^user\s*='))
    {
        Stop-Here 'That wasn''t the "Configuration file preview" text (it starts with [DEFAULT]). Start this again.'
    }
    $downloads = Join-Path $env:USERPROFILE 'Downloads'
    $pem = Get-ChildItem -Path $downloads -Filter '*.pem' -ErrorAction SilentlyContinue |
           Sort-Object -Property LastWriteTime -Descending | Select-Object -First 1
    if ($pem -eq $null)
    {
        Stop-Here "Couldn't find the downloaded key (a .pem file) in $downloads. Download it again (step 2) and start this again."
    }
    $ociDir = Join-Path $env:USERPROFILE '.oci'
    New-Item -ItemType Directory -Force -Path $ociDir | Out-Null
    $keyCopy = Join-Path $ociDir 'oci_api_key.pem'
    Copy-Item -Path $pem.FullName -Destination $keyCopy -Force
    $lines = @($lines | Where-Object { $_ -notmatch '^key_file\s*=' }) + "key_file=$keyCopy"
    Set-Content -Path $config -Value $lines -Encoding Ascii
    Say "Saved $config (key: $keyCopy)."
}
$m = Select-String -Path $config -Pattern '^\s*tenancy\s*=\s*(\S+)' | Select-Object -First 1
if ($m -eq $null)
{
    Stop-Here "$config has no tenancy= line: delete that file and start this again (it asks for Oracle's key text)."
}
$tenancy = $m.Matches[0].Groups[1].Value

$r = Invoke-Oci @('iam', 'availability-domain', 'list', '--compartment-id', $tenancy)
if (-not $r.Ok)
{
    Say 'Oracle didn''t accept the sign-in in the config file. What it said:'
    Stop-Here $r.Text
}
$ads = @((Get-OciData @('iam', 'availability-domain', 'list', '--compartment-id', $tenancy)) | ForEach-Object { $_.name })
Say ('Signed in. Availability domains: ' + ($ads -join ', '))

# 2. Not one already (this never makes a second)
function Find-Computer
{
    $list = Get-OciData @('compute', 'instance', 'list', '--compartment-id', $tenancy, '--all')
    return @($list | Where-Object { $_.'display-name' -eq $name -and $_.'lifecycle-state' -notin @('TERMINATED', 'TERMINATING') }) | Select-Object -First 1
}

function Report-Computer($inst)
{
    Say ('Got it! "' + $name + '" is ' + $inst.'lifecycle-state' + ' in ' + $inst.'availability-domain' + '.')
    $ip = ''
    for ($i = 0; $i -lt 30 -and $ip -eq ''; $i++)
    {
        $vnics = Get-OciData @('compute', 'instance', 'list-vnics', '--instance-id', $inst.id)
        if ($vnics -ne $null -and @($vnics).Count -gt 0 -and $vnics[0].'public-ip') { $ip = $vnics[0].'public-ip' }
        else { Start-Sleep -Seconds 20 }
    }
    if ($ip -eq '') { $ip = '(not given yet - look on the instance''s page in Oracle)' }
    Say "Public IP address: $ip"
    Say "SSH key for step 5: $keyFile"
    $note = Join-Path $here 'oracle-computer.txt'
    Set-Content -Path $note -Value @(
        "Wally's Journey server computer (Oracle)",
        "Public IP address: $ip",
        "Key file (KEYFILE in step 5): $keyFile",
        "Next: step 5 of docs/TANDEM_ORACLE_SETUP.md - put the server on it.")
    [console]::beep(880, 400); [console]::beep(1175, 600)
    try
    {
        Add-Type -AssemblyName PresentationFramework
        [System.Windows.MessageBox]::Show("Oracle gave you the free server computer!`n`nPublic IP: $ip`n`n(Also in oracle-computer.txt.)", 'Wally''s Journey') | Out-Null
    }
    catch { }
    Wait-Enter 'Press Enter to close'
    exit 0
}

$have = Find-Computer
if ($have -ne $null) { Report-Computer $have }

# 3. What to ask for: the network's public subnet, Ubuntu 24.04 for ARM, the SSH key
$subnets = @(Get-OciData @('network', 'subnet', 'list', '--compartment-id', $tenancy, '--all'))
$subnet = $subnets | Where-Object { $_.'display-name' -like 'public subnet*Wallys Journey*' } | Select-Object -First 1
if ($subnet -eq $null)
{
    Say 'Couldn''t find the "public subnet-Wallys Journey Network" (step 2). Subnets there:'
    $subnets | ForEach-Object { Say ('  ' + $_.'display-name') }
    Stop-Here 'Make the network as in step 2 (or tell Claude the name above), then start this again.'
}
Say ('Subnet: ' + $subnet.'display-name')

$images = @(Get-OciData @('compute', 'image', 'list', '--compartment-id', $tenancy, '--operating-system', 'Canonical Ubuntu',
                      '--operating-system-version', '24.04', '--shape', $shape, '--all'))
$image = $images | Where-Object { $_.'display-name' -notlike '*Minimal*' } |
         Sort-Object -Property 'time-created' -Descending | Select-Object -First 1
if ($image -eq $null) { Stop-Here 'Couldn''t find Oracle''s Ubuntu 24.04 picture for ARM computers.' }
Say ('Image: ' + $image.'display-name')

if (-not (Test-Path "$keyFile.pub"))
{
    if (-not (Get-Command ssh-keygen -ErrorAction SilentlyContinue))
    {
        Stop-Here 'Windows'' ssh-keygen is missing: Settings > System > Optional features > add "OpenSSH Client", then start this again.'
    }
    Say "Making the SSH key: $keyFile (keep this file - it's how you get into the computer)"
    & ssh-keygen -q -t rsa -b 4096 -N '""' -C 'wallys-journey' -f $keyFile
    if (-not (Test-Path "$keyFile.pub")) { Stop-Here 'ssh-keygen didn''t make the key.' }
}

$shapeFile = Join-Path $here 'oracle-shape.json'
Set-Content -Path $shapeFile -Value '{"ocpus": 1, "memoryInGBs": 6}' -Encoding Ascii

# 4. Keep asking: one availability domain a try, a minute apart
Say "Asking every $WaitSeconds seconds, one availability domain at a time. Leave this window open (and the PC awake)."
$try = 0
$slowDown = 120   # (seconds to wait when Oracle asks to slow down: 2 minutes, longer each time it asks again)
while ($true)
{
    foreach ($ad in $ads)
    {
        $try++
        # (--no-retry: one question a try. Without it Oracle's tool asks up to 7 more times by itself, waiting longer
        # each time - Brennan 2026-11-27 20:46: tries came 2 minutes apart, and Oracle asked to slow down after 3)
        $r = Invoke-Oci @('compute', 'instance', 'launch', '--no-retry', '--availability-domain', $ad, '--compartment-id', $tenancy,
                   '--shape', $shape, '--shape-config', "file://$shapeFile", '--image-id', $image.id,
                   '--subnet-id', $subnet.id, '--assign-public-ip', 'true', '--display-name', $name,
                   '--ssh-authorized-keys-file', "$keyFile.pub")
        if ($r.Ok -or ($r.Text -notmatch 'capacity'))
        {
            # (made - or an answer that isn't "no room": look whether it exists before anything else)
            $have = Find-Computer
            if ($have -ne $null) { Report-Computer $have }
        }
        if ($r.Text -match 'capacity')
        {
            Say "Try $try ($ad): no room yet."
            $slowDown = 120
        }
        elseif ($r.Text -match 'TooManyRequests|429')
        {
            Say ("Try $try ($ad): Oracle asked to slow down - waiting " + [int]($slowDown / 60) + " minutes.")
            Start-Sleep -Seconds $slowDown
            $slowDown = [Math]::Min($slowDown * 2, 600)
        }
        elseif ($r.Text -match 'LimitExceeded')
        {
            Say "Try $try ($ad): Oracle says the free limit is used up. What it said:"
            Stop-Here $r.Text
        }
        else
        {
            Say "Try $try ($ad): an answer this script doesn't know. What Oracle said:"
            Stop-Here $r.Text
        }
        Start-Sleep -Seconds $WaitSeconds
    }
}
