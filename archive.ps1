
class Config {
	[string]$name
	[string]$unicode
	[string]$arch
	[string]$triplet; `
`
	Config([string]$name, [string]$unicode, [string]$arch, [string]$triplet) {
		$this.name = $name
		$this.unicode = $unicode
		$this.arch = $arch
		$this.triplet = $triplet
	}
}

$configs = @(
	[Config]::new("x86-unicode", "ON", "Win32", "x86-windows-static"),
	[Config]::new("x86-ansi", "OFF", "Win32", "x86-windows-static"),
	[Config]::new("x64-unicode", "ON", "x64", "x64-windows-static"),
	[Config]::new("x64-ansi", "OFF", "x64", "x64-windows-static")
)

$jobs = foreach ($config in $configs) {
    $name = $config.name
    $unicode = $config.unicode
    $arch = $config.arch
    $triplet = $config.triplet

    Start-Job {
        cmake -S . -B "build/$using:name" `
            -DCMAKE_BUILD_TYPE=Release `
            "-DUNICODE=$using:unicode" `
            "-A $using:arch" `
            "-DCMAKE_TOOLCHAIN_FILE=C:\Program Files\Microsoft Visual Studio\18\Professional\VC\vcpkg\scripts\buildsystems\vcpkg.cmake" `
            "-DVCPKG_TARGET_TRIPLET=$using:triplet"
    }
}

Write-Host "Configuring..."
$jobs | Wait-Job | Receive-Job | Out-Null

$jobs = foreach ($config in $configs) {
	$name = $config.name
	Start-Job {
		cmake --build "build/$using:name" --config Release
	}
}

Write-Host "Building..."
$jobs | Wait-Job | Receive-Job | Out-Null

Write-Host "Archiving..."
foreach ($config in $configs) {
	$name = $config.name
	Compress-Archive -Path "build/$name/$name" -DestinationPath "build/nscj.zip" -Update
}