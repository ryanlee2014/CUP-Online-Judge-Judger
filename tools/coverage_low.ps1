param(
    [string]$Path,
    [int]$Top = 30
)

$lines = Get-Content $Path
$results = @()
$file = ""
$lf = 0
$lh = 0
$daCount = 0
$daHit = 0

foreach ($line in $lines) {
    if ($line -like "SF:*") {
        $file = $line.Substring(3)
        $lf = 0
        $lh = 0
        $daCount = 0
        $daHit = 0
        continue
    }
    if ($line -like "LF:*") {
        $lf = [int]$line.Substring(3)
        continue
    }
    if ($line -like "LH:*") {
        $lh = [int]$line.Substring(3)
        continue
    }
    if ($line -like "DA:*") {
        $parts = $line.Substring(3).Split(",")
        if ($parts.Length -ge 2) {
            $daCount++
            if ([int]$parts[1] -gt 0) {
                $daHit++
            }
        }
        continue
    }
    if ($line -eq "end_of_record") {
        if ($file -ne "") {
            if ($lf -le 0) {
                $lf = $daCount
                $lh = $daHit
            }
            if ($lf -gt 0) {
                $rate = [math]::Round(($lh / $lf) * 100, 1)
                $results += [pscustomobject]@{
                    Rate = $rate
                    File = $file
                    Lines = $lf
                    Hits = $lh
                }
            }
        }
    }
}

$results = $results | Where-Object { $_.Rate -lt 100 }
$results | Sort-Object Rate | Select-Object -First $Top | ForEach-Object {
    "{0}|{1}|{2}/{3}" -f $_.Rate, $_.File, $_.Hits, $_.Lines
}
