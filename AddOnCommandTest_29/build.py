import subprocess
import os

def run():
    vcvars = r'D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat'
    cmake = r'D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
    
    # Simple cmd execution
    cmd = f'call "{vcvars}" && "{cmake}" -B Build -G "Visual Studio 17 2022" -A x64 && "{cmake}" --build Build --config Release'
    
    print(f"Running: {cmd}")
    result = subprocess.run(cmd, shell=True)
    
    if result.returncode == 0:
        print("[SUCCESS] Build complete.")
        src = os.path.abspath(os.path.join("Build", "Release", "AddOnCommandTest.apx"))
        dst = os.path.abspath("C:\\Program Files\\GRAPHISOFT\\ARCHICAD 27\\애드온\\AddOnCommandTest.apx")
        print(f"Copying from {src} to {dst}")
        try:
            # Use /Y to overwrite without asking
            subprocess.run(['copy', '/Y', src, dst], shell=True, check=True)
            print("[DONE] Copied.")
        except Exception as e:
            print(f"[ERROR] Copy failed: {e}")
    else:
        print(f"[ERROR] Build failed with code {result.returncode}")

if __name__ == "__main__":
    run()
