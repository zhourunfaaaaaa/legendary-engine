param(
  [string]$Proxy = "http://127.0.0.1:7897",
  [string]$NodeHost = "",
  [int]$NodePort = 0,
  [switch]$IncludeLogs,
  [int]$HttpTimeoutSec = 8,
  [int]$TcpTimeoutMs = 3000
)

$ErrorActionPreference = "Continue"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$report = Join-Path $scriptDir ("campus-net-report-{0}.txt" -f (Get-Date -Format "yyyyMMdd-HHmmss"))

function Add-Line {
  param([string]$Text = "")
  Add-Content -LiteralPath $report -Value $Text -Encoding UTF8
}

function Add-Section {
  param([string]$Name)
  Add-Line ""
  Add-Line ("===== {0} =====" -f $Name)
  Write-Host ("== {0} ==" -f $Name)
}

function Run-Block {
  param(
    [string]$Name,
    [scriptblock]$Block
  )
  Add-Section $Name
  try {
    $output = & $Block 2>&1 | Out-String -Width 260
    Add-Line $output.TrimEnd()
  } catch {
    Add-Line ("ERROR: {0}" -f $_.Exception.Message)
  }
}

function Test-Url {
  param(
    [string]$Label,
    [string]$Uri,
    [string]$ProxyUrl = ""
  )

  try {
    $params = @{
      Uri = $Uri
      UseBasicParsing = $true
      TimeoutSec = $HttpTimeoutSec
      MaximumRedirection = 0
    }
    if ($ProxyUrl) {
      $params.Proxy = $ProxyUrl
    }

    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $response = Invoke-WebRequest @params
    $sw.Stop()
    "{0} OK {1} {2} {3}ms {4}" -f $Label, $response.StatusCode, $response.StatusDescription, $sw.ElapsedMilliseconds, $Uri
  } catch {
    $status = ""
    if ($_.Exception.Response) {
      try { $status = " HTTP=" + [int]$_.Exception.Response.StatusCode } catch {}
    }
    "{0} ERR {1}{2} {3}" -f $Label, $_.Exception.GetType().Name, $status, $_.Exception.Message
  }
}

function Test-TcpFast {
  param(
    [string]$HostName,
    [int]$Port,
    [int]$TimeoutMs = $TcpTimeoutMs
  )

  $client = New-Object System.Net.Sockets.TcpClient
  $remote = $null
  $ok = $false
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

function Redact-Line {
  param([string]$Text)
  $redacted = $Text
  $redacted = $redacted -replace '\b[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\b', '[uuid]'
  $redacted = $redacted -replace '\b(?:\d{1,3}\.){3}\d{1,3}\b', '[ipv4]'
  $redacted = $redacted -replace '\b(?:[a-zA-Z0-9-]+\.)+[a-zA-Z]{2,}\b', '[host]'
  return $redacted
}

Add-Line ("Campus Clash/TUN diagnostic report")
Add-Line ("Generated: {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
Add-Line ("Computer: {0}" -f $env:COMPUTERNAME)
Add-Line ("Proxy: {0}" -f $Proxy)
Add-Line ("HTTP timeout: {0}s" -f $HttpTimeoutSec)
Add-Line ("TCP timeout: {0}ms" -f $TcpTimeoutMs)
Add-Line "Privacy: this script does not read Clash profile/subscription files. Logs are included only with -IncludeLogs and are redacted."

Run-Block "Network adapters" {
  Get-NetAdapter |
    Select-Object Name, InterfaceDescription, Status, LinkSpeed, ifIndex |
    Format-Table -AutoSize
}

Run-Block "IP configuration" {
  Get-NetIPConfiguration |
    Select-Object InterfaceAlias, InterfaceIndex, IPv4Address, IPv6Address, IPv4DefaultGateway, DNSServer |
    Format-List
}

Run-Block "IPv4 interface metrics" {
  Get-NetIPInterface -AddressFamily IPv4 |
    Sort-Object InterfaceMetric, InterfaceAlias |
    Select-Object InterfaceAlias, InterfaceIndex, Dhcp, InterfaceMetric, ConnectionState |
    Format-Table -AutoSize
}

Run-Block "Important IPv4 routes" {
  Get-NetRoute -AddressFamily IPv4 |
    Where-Object {
      $_.DestinationPrefix -in @("0.0.0.0/0", "0.0.0.0/1", "128.0.0.0/1") -or
      $_.DestinationPrefix -like "198.18.*"
    } |
    Sort-Object RouteMetric, InterfaceMetric |
    Select-Object DestinationPrefix, NextHop, InterfaceAlias, RouteMetric, InterfaceMetric, ifIndex |
    Format-Table -AutoSize
}

Run-Block "DNS servers" {
  Get-DnsClientServerAddress |
    Select-Object InterfaceAlias, AddressFamily, ServerAddresses |
    Format-List
}

Run-Block "Windows proxy settings" {
  netsh winhttp show proxy
  Get-ItemProperty "HKCU:\Software\Microsoft\Windows\CurrentVersion\Internet Settings" |
    Select-Object ProxyEnable, ProxyServer, AutoConfigURL |
    Format-List
}

Run-Block "Clash related processes" {
  Get-Process |
    Where-Object { $_.ProcessName -match "clash|verge|mihomo|meta" } |
    Select-Object ProcessName, Id, Path |
    Format-Table -AutoSize
}

Run-Block "Local Clash listening ports" {
  Get-NetTCPConnection -State Listen |
    Where-Object {
      $_.LocalAddress -in @("127.0.0.1", "0.0.0.0", "::", "::1") -and
      $_.LocalPort -in @(7890,7891,7892,7893,7897,7898,7899,9090,9097,53)
    } |
    Sort-Object LocalPort |
    Select-Object LocalAddress, LocalPort, OwningProcess |
    Format-Table -AutoSize
}

Run-Block "Local proxy port tests" {
  foreach ($port in @(7897,7898,7899,9097)) {
    Test-TcpFast 127.0.0.1 $port
  }
}

Run-Block "DNS resolution tests" {
  foreach ($name in @("www.gstatic.com", "www.google.com", "github.com", "openai.com")) {
    "NAME $name"
    Resolve-DnsName $name -Type A -ErrorAction SilentlyContinue |
      Select-Object Name, Type, IPAddress, NameHost
  }
}

Run-Block "HTTP tests via explicit local proxy" {
  foreach ($url in @(
    "https://www.gstatic.com/generate_204",
    "https://www.google.com/generate_204",
    "https://www.youtube.com/generate_204",
    "https://github.com",
    "https://openai.com"
  )) {
    Write-Host ("Testing via proxy: {0}" -f $url)
    Test-Url -Label "PROXY" -Uri $url -ProxyUrl $Proxy
  }
}

Run-Block "HTTP tests without explicit proxy, TUN may still capture" {
  foreach ($url in @(
    "https://www.gstatic.com/generate_204",
    "https://www.google.com/generate_204",
    "https://github.com"
  )) {
    Write-Host ("Testing system path: {0}" -f $url)
    Test-Url -Label "SYSTEM" -Uri $url
  }
}

Run-Block "TCP egress tests, TUN may still capture" {
  foreach ($target in @("1.1.1.1", "8.8.8.8", "github.com", "www.google.com")) {
    foreach ($port in @(443, 80)) {
      "TARGET $target PORT $port"
      Test-TcpFast $target $port
    }
  }
}

if ($NodeHost -and $NodePort -gt 0) {
  Run-Block "Optional node reachability test" {
    "NODE $NodeHost PORT $NodePort"
    Resolve-DnsName $NodeHost -ErrorAction SilentlyContinue |
      Select-Object Name, Type, IPAddress, NameHost
    Test-TcpFast $NodeHost $NodePort
  }
} else {
  Add-Section "Optional node reachability test"
  Add-Line "Skipped. Run with -NodeHost your.node.host -NodePort 443 to test a node without exposing passwords."
}

if ($IncludeLogs) {
  Run-Block "Recent Clash error logs, redacted" {
    $logDir = Join-Path $env:APPDATA "io.github.clash-verge-rev.clash-verge-rev\logs"
    if (Test-Path $logDir) {
      Get-ChildItem -LiteralPath $logDir -Filter "*.log" |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 5 |
        ForEach-Object {
          Select-String -LiteralPath $_.FullName -Pattern "timeout|failed|error|refused|reset|tls|handshake|EOF|deadline|no such host|dns|connect" -SimpleMatch:$false |
            Select-Object -Last 80 |
            ForEach-Object { Redact-Line ("{0}:{1}: {2}" -f $_.Path, $_.LineNumber, $_.Line) }
        }
    } else {
      "No Clash Verge log directory found."
    }
  }
}

Add-Section "Done"
Add-Line ("Report path: {0}" -f $report)
Write-Host "Diagnostic complete:"
Write-Host $report
