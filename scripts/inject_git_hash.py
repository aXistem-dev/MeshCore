Import("env")
import subprocess

def get_git_hash():
    try:
        result = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            capture_output=True, text=True, check=True
        )
        return result.stdout.strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        return "unknown"

git_hash = get_git_hash()
env.Append(CPPDEFINES=[("GIT_COMMIT_HASH", f'\\"{git_hash}\\"')])
print(f"Git commit hash: {git_hash}")
