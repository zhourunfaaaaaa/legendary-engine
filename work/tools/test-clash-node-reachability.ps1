param(
  [string]$ClashConfig = "$env:APPDATA\io.github.clash-verge-rev.clash-verge-rev\clash-verge.yaml",
  [int]$TcpTimeoutMs = 3000,
  [int]$MaxTests = 80
)

$ErrorActionPreference = "Continue"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$report = Join-Path $scriptDir ("node-reachability-report-{0}.txt" -f (Get-Date -Format "yyyyMMdd-HHmmss"))

function Add-Line {
  param([string]$Text = "")
  Add-Content -LiteralPath $report -Value $Text -Encoding UTF8
}

function Test-TcpFast {
  param(
    [string]$HostName,
    [int]$Port,
    [int]$TimeoutMs = $TcpTimeoutMs
  )

  $client = New-Object System.Net.Sockets.TcpClient
  $ok = $false
  $remote = ""
  $message = ""
  $sw = [System.Diagnostics.Stopwatch]::StartNew()

  try {
    $async = $client.BeginConnect($HostName, $Port, $null, $null)
    if ($async.AsyncWaitHandle.WaitOne($TimeoutMs, $false)) {
      $client.EndConnect($async)
      $ok = $client.Connected
      if ($client.Client.RemoteEndPoint) {
        $remote = $client.Client.RemoteEndPoint.ToString()
      }
    } else {
      $message = "timeout after ${TimeoutMs}ms"
    }
  } catch {
    $message = $_.Exception.Message
  } finally {
    $sw.Stop()
    $client.Close()
  }

  [pscustomobject]@{
    Host = $HostName
    Port = $Port
    TcpSucceeded = $ok
    RemoteEndPoint = $remote
    ElapsedMs = $sw.ElapsedMilliseconds
    Message = $message
  }
}

function Get-ClashNodeEndpoints {
  param([string]$Path)

  $nodes = New-Object System.Collections.Generic.List[object]
  $current = $null
  $inProxies = $false

  foreach ($line in Get-Content -LiteralPath $Path) {
    if ($line -match '^proxies:\s*$') {
      $inProxies = $true
      continue
    }
    if (-not $inProxies) {
      continue
    }
    if ($line -match '^[a-zA-Z0-9_-]+:\s*$' -and $line -notmatch '^proxies:\s*$') {
      break
    }
    if ($line -match '^- name:\s*(.+)$') {
      if ($current -and $current.server -and $current.port) {
        $nodes.Add([pscustomobject]$current)
      }
      $current = @{
        type = ""
        server = ""
        port = 0
        network = ""
      }
      continue
    }
    if (-not $current) {
      continue
    }
    if ($line -match '^\s+type:\s*(\S+)\s*$') {
      $current.type = $matches[1]
    } elseif ($line -match '^\s+server:\s*(\S+)\s*$') {
      $current.server = $matches[1].Trim("'`"")
    } elseif ($line -match '^\s+port:\s*(\d+)\s*$') {
      $current.port = [int]$matches[1]
    } elseif ($line -match '^\s+network:\s*(\S+)\s*$') {
      $current.network = $matches[1]
    }
  }

  if ($current -and $current.server -and $current.port) {
    $nodes.Add([pscustomobject]$current)
  }

  $nodes |
    Where-Object { $_.server -and $_.port } |
    Sort-Object server, port, type, network -Unique |
    Select-Object -First $MaxTests
}

Add-Line "Clash node reachability report"
Add-Line ("Generated: {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
Add-Line ("Config: {0}" -f $ClashConfig)
Add-Line ("TCP timeout: {0}ms" -f $TcpTimeoutMs)
Add-Line "Privacy: this script extracts only node type/server/port/network. It does not print uuid, password, cipher, alterId, subscription URL, or private keys."
Add-Line "Important: run this with Clash TUN/DNS hijack disabled. If DNS answers are 198.18.x.x, the test is captured by Clash fake-ip and cannot prove real node reachability."
Add-Line ""

if (-not (Test-Path -LiteralPath $ClashConfig)) {
  Add-Line "ERROR: Clash config not found."
  Write-Host "Config not found: $ClashConfig"
  exit 1
}

$nodes = @(Get-ClashNodeEndpoints -Path $ClashConfig)
Add-Line ("Unique endpoints tested: {0}" -f $nodes.Count)
Add-Line ""

foreach ($node in $nodes) {
  Write-Host ("Testing {0}:{1} [{2} {3}]" -f $node.server, $node.port, $node.type, $node.network)
  Add-Line ("===== {0}:{1} type={2} network={3} =====" -f $node.server, $node.port, $node.type, $node.network)

  try {
    $answers = @(Resolve-DnsName $node.server -Type A -ErrorAction Stop)
    $fakeIpSeen = $false
    foreach ($answer in $answers) {
      if ($answer.IPAddress) {
        Add-Line ("DNS A: {0}" -f $answer.IPAddress)
        if ($answer.IPAddress -like "198.18.*") {
          $fakeIpSeen = $true
        }
      }
    }
    if ($fakeIpSeen) {
      Add-Line "WARNING: DNS returned Clash fake-ip 198.18.x.x. Disable TUN/DNS hijack and rerun; TCP result below is not a direct campus-to-node test."
    }
  } catch {
    Add-Line ("DNS ERROR: {0}" -f $_.Exception.Message)
  }

  $tcp = Test-TcpFast -HostName $node.server -Port $node.port
  Add-Line ("TCP Host={0} Port={1} Succeeded={2} Remote={3} ElapsedMs={4} Message={5}" -f $tcp.Host, $tcp.Port, $tcp.TcpSucceeded, $tcp.RemoteEndPoint, $tcp.ElapsedMs, $tcp.Message)
  Add-Line ""
}

Add-Line "Interpretation:"
Add-Line "- Hotspot succeeds but campus fails at TCP: likely IP/port/path blocking."
Add-Line "- Campus TCP succeeds but Clash node delay times out: likely protocol handshake/fingerprint/UDP-specific blocking."
Add-Line "- Campus DNS differs from hotspot: possible DNS filtering or poisoned resolution."
Add-Line ""
Add-Line ("Report path: {0}" -f $report)

Write-Host "Diagnostic complete:"
Write-Host $report
