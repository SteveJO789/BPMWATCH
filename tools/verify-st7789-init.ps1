$ErrorActionPreference = 'Stop'

$targets = @(
    'firmware/tests/st7789_display_test/src/main.cpp',
    'firmware/slave/src/DisplayUI.cpp'
)

$failed = $false

foreach ($path in $targets) {
    $source = Get-Content -Raw -Encoding UTF8 $path
    $checks = [ordered]@{
        'manual reset pulse' = $source -match 'digitalWrite\([^,]+,\s*LOW\)'
        'SPI mode 3 init' = $source -match '\.init\(240,\s*240,\s*SPI_MODE3\)'
        '8 MHz display transaction speed' =
            ($source -match '(?m)(?:DISPLAY_SPI_HZ|kDisplaySpiHz)\s*=\s*8000000') -and
            ($source -match '\.setSPISpeed\((?:DISPLAY_SPI_HZ|kDisplaySpiHz)\)')
    }

    foreach ($check in $checks.GetEnumerator()) {
        if (-not $check.Value) {
            Write-Error "$path missing contract: $($check.Key)" -ErrorAction Continue
            $failed = $true
        }
    }

    $resetIndex = $source.IndexOf('LOW)')
    $initIndex = $source.IndexOf('.init(240, 240, SPI_MODE3)')
    $speedIndex = $source.IndexOf('.setSPISpeed(')
    if ($resetIndex -lt 0 -or $initIndex -lt 0 -or $speedIndex -lt 0 -or
        $resetIndex -gt $initIndex -or $initIndex -gt $speedIndex) {
        Write-Error "$path must reset, initialize in SPI_MODE3, then set 8 MHz speed" -ErrorAction Continue
        $failed = $true
    }
}

if ($failed) {
    exit 1
}

Write-Output 'ST7789 initialization contract verified for display test and DisplayUI.'
