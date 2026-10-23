"""Run deterministic C++ sampling tests without a connected Arduino."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
sources = [root / "src/Sampling.cpp", root / "tests/sampling/sampling_test.cpp"]
includes = [root / "tests/sampling/fakes", root / "src"]

with tempfile.TemporaryDirectory(prefix="fbl-sampling-tests-") as directory:
    build = Path(directory)
    if os.name == "nt":
        vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
        installation = subprocess.check_output(
            [str(vswhere), "-latest", "-products", "*", "-requires",
             "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
            text=True).strip()
        if not installation:
            raise SystemExit("MSVC C++ Build Tools not found.")
        vcvars = Path(installation) / "VC/Auxiliary/Build/vcvars64.bat"
        commands = [f'@call "{vcvars}" >nul', '@if errorlevel 1 exit /b 1']
        for name, defines in [("software", "/DFBL_SAMPLING_SOFTWARE_ONLY"),
                              ("avr", "/DARDUINO_AVR_UNO /DF_CPU=16000000UL")]:
            arguments = " ".join(f'/I"{p}"' for p in includes)
            arguments += " " + " ".join(f'"{p}"' for p in sources)
            commands += [f'cl /nologo /EHsc /W4 {defines} {arguments} /Fe:{name}.exe',
                         '@if errorlevel 1 exit /b 1', f'{name}.exe',
                         '@if errorlevel 1 exit /b 1']
        script = build / "test.cmd"
        script.write_text("\n".join(commands) + "\n", encoding="utf-8")
        subprocess.run(["cmd", "/d", "/c", str(script)], cwd=build, check=True)
    else:
        compiler = shutil.which("c++")
        if not compiler:
            raise SystemExit("A C++11 compiler is required.")
        for name, defines in [("software", ["-DFBL_SAMPLING_SOFTWARE_ONLY"]),
                              ("avr", ["-DARDUINO_AVR_UNO", "-DF_CPU=16000000UL"])]:
            executable = build / name
            subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", *defines,
                            *(f"-I{p}" for p in includes), *(str(p) for p in sources),
                            "-o", str(executable)], check=True)
            subprocess.run([str(executable)], check=True)
