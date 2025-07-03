import subprocess
from datetime import datetime

def has_changes():
    # Check for changes (untracked files or modifications)
    result = subprocess.run(
        ['git', 'status', '--porcelain'],
        stdout=subprocess.PIPE, text=True
    )
    return bool(result.stdout.strip())

def auto_commit_push():
    if has_changes():
        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        commit_message = f"Auto commit at {now}"
        
        try:
            subprocess.run(['git', 'add', '.'], check=True)
            subprocess.run(['git', 'commit', '-m', commit_message], check=True)
            
            # Lấy tên branch hiện tại
            branch_result = subprocess.run(
                ['git', 'branch', '--show-current'],
                stdout=subprocess.PIPE, text=True, check=True
            )
            branch = branch_result.stdout.strip()
            
            subprocess.run(['git', 'push', 'origin', branch], check=True)
            print(f"Backup Su'")
        
        except subprocess.CalledProcessError as e:
            print("❌ Error: ", e)
    else:
        print("✅ Không có thay đổi nào để commit.")

while True:
    auto_commit_push();
