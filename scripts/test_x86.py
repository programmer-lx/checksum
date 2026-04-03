import subprocess
import sys
import os
import argparse
from pathlib import Path
import shutil
import platform

CURRENT_OS = str.lower(platform.system())
SDE_NAME = str("")
if CURRENT_OS == "windows":
    SDE_NAME = "sde"
elif CURRENT_OS == "linux":
    SDE_NAME = "sde64"
else:
    print("Error: unknown os.")
    sys.exit(1)

def run_command(command, env=None):
    command = [str(c) for c in command]
    print(f"\n[RUNNING] {' '.join(command)}")
    
    current_env = os.environ.copy()
    if env:
        current_env.update(env)
        
    subprocess.run(command, check=True, env=current_env)

def main():
    parser = argparse.ArgumentParser(description="X86 Testing with Intel SDE (Sapphire Rapids)")
    parser.add_argument("--test_mode", choices=["min", "max"], default="min", help="min: Debug/Clang, max: All configs/compilers")
    parser.add_argument("--compiler", choices=["msvc", "gcc", "clang"], default="gcc")
    args = parser.parse_args()

    # 路径定义
    script_dir = Path(__file__).parent.resolve()
    project_root = script_dir.parent
    build_base = project_root / "build" / f"{CURRENT_OS}_x86"

    # 查找 SDE 可执行文件
    sde_bin = shutil.which(str(SDE_NAME))

    if not sde_bin:
        print("Error: 'sde or sde64' not found in PATH. Please check SDE installation.")
        sys.exit(1)
    sde_bin = os.path.abspath(sde_bin)
    print(f"Using SDE found at: {sde_bin}")

    # 编译器矩阵
    configs = []

    if CURRENT_OS == "windows":
        configs = [
            ("MinGW-GCC", "x86_64-w64-mingw32-gcc", "x86_64-w64-mingw32-g++", "mingw"),
            ("Clang-17", "clang", "clang++", "clang17"),
            ("MSVC", "cl", "cl", "msvc")
        ]
        if args.test_mode == "min":
            if args.compiler == "msvc":
                configs = [configs[2]]
            elif args.compiler == "gcc":
                configs = [configs[0]]
            elif args.compiler == "clang":
                configs = [configs[1]]
            else:
                print("Error: invalid compiler.")
                sys.exit(1)
    
    if CURRENT_OS == "linux":
        configs = [
            ("Clang-17", "clang-17", "clang++-17", "clang17"),
            ("GCC-13", "gcc-13", "g++-13", "gcc13")
        ]
        if args.test_mode == "min":
            if args.compiler == "gcc":
                configs = [configs[1]]
            elif args.compiler == "clang":
                configs = [configs[0]]
            else:
                print("Error: invalid compiler.")
                sys.exit(1)

    # 编译选项矩阵
    build_options = ["Release"]
    if args.test_mode == "max":
        build_options += ["Debug"]

    for name, c_comp, cxx_comp, subdir in configs:
        for build_cfg in build_options:
            current_build_dir = build_base / f"{subdir}_{build_cfg}"
            
            print(f"\n{'='*60}\nTarget: {name} | Config: {build_cfg}\n{'='*60}")

            # 1. configure
            config_args = [
                "cmake", "-S", project_root, "-B", current_build_dir,
                "-G", "Ninja Multi-Config",
                f"-DCMAKE_C_COMPILER={c_comp}",
                f"-DCMAKE_CXX_COMPILER={cxx_comp}",
                "-DCKS_BUILD_TESTS=ON",
                f"-DCMAKE_CROSSCOMPILING_EMULATOR={sde_bin};-future;--"
            ]

            if CURRENT_OS == "linux":
                config_args.append("-DCMAKE_EXE_LINKER_FLAGS=-static");

            run_command(config_args)

            # 2. 编译
            run_command(["cmake", "--build", current_build_dir, "--config", build_cfg])

            # 3. 测试
            ctest_bin = shutil.which("ctest")
            if not ctest_bin:
                print("Error: 'ctest' not found in PATH.")
                sys.exit(1)

            run_command([
                ctest_bin, "--output-on-failure", "--test-dir", current_build_dir, "-C", build_cfg
            ])

    print("\n[SUCCESS] All X86 tests passed.")

if __name__ == "__main__":
    main()