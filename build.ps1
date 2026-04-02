#==================================================================
# Build, test & archive nscj for all configurations
#==================================================================

param(
    [Parameter(Mandatory=$true)]
    [ValidateSet("Archive", "Test")]
    [string]$Mode
)

class Config {
    [string]$name
    [string]$unicode
    [string]$arch
    [string]$triplet

    Config([string]$name, [string]$unicode, [string]$arch, [string]$triplet) {
        $this.name    = $name
        $this.unicode = $unicode
        $this.arch    = $arch
        $this.triplet = $triplet
    }
}

$configs = @(
    [Config]::new("x86-unicode", "ON",  "Win32", "x86-windows-static"),
    [Config]::new("x86-ansi",    "OFF", "Win32", "x86-windows-static"),
    [Config]::new("x64-unicode", "ON",  "x64",   "x64-windows-static"),
    [Config]::new("x64-ansi",    "OFF", "x64",   "x64-windows-static")
)

$buildType  = if ($Mode -eq "Archive") { "Release" } else { "Debug" }
$buildRoot  = if ($Mode -eq "Archive") { "build/release" } else { "build/debug" }

#------------------------------------------------------------------
# Configure
#------------------------------------------------------------------
$jobs = foreach ($config in $configs) {
    $name    = $config.name
    $unicode = $config.unicode
    $arch    = $config.arch
    $triplet = $config.triplet

    Start-Job {
        cmake -S . -B "$using:buildRoot/$using:name" `
            "-DCMAKE_BUILD_TYPE=$using:buildType" `
            "-DUNICODE=$using:unicode" `
            "-A $using:arch" `
            "-DCMAKE_TOOLCHAIN_FILE=C:\Program Files\Microsoft Visual Studio\18\Professional\VC\vcpkg\scripts\buildsystems\vcpkg.cmake" `
            "-DVCPKG_TARGET_TRIPLET=$using:triplet"
    }
}

Write-Host "Configuring ($buildType)..."
$jobs | Wait-Job | Receive-Job | Out-Null

#------------------------------------------------------------------
# Build
#------------------------------------------------------------------
$jobs = foreach ($config in $configs) {
    $name = $config.name
    Start-Job {
        cmake --build "$using:buildRoot/$using:name" --config $using:buildType
    }
}

Write-Host "Building..."
$jobs | Wait-Job | Receive-Job | Out-Null

#------------------------------------------------------------------
# Archive or Test
#------------------------------------------------------------------
if ($Mode -eq "Archive") {
    Write-Host "Archiving..."
    foreach ($config in $configs) {
        $name = $config.name
        Write-Host "Adding $buildRoot/$name/$name to archive..."
        Compress-Archive -Path "$buildRoot/$name/$name" -DestinationPath "build/nscj.zip" -Update
    }
} else {
    Write-Host "Testing..."
    $jobs = foreach ($config in $configs) {
        $name = $config.name
        Start-Job {
            ctest --test-dir "$using:buildRoot/$using:name" -C $using:buildType --output-on-failure
        }
    }
    $jobs | Wait-Job | Receive-Job
}

Write-Host "Done."