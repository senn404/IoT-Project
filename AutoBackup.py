import subprocess
from datetime import datetime

def has_changes():
    # Check for changes (untracked files or modifications)
    result = subprocess.run(
        ['git', 'status', '--porcelain'],
        stdout=subprocess.PIPE, 
        stderr=subprocess.DEVNULL,
        text=True
    )
    return bool(result.stdout.strip())

def auto_commit_push():
    if has_changes():
        now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        commit_message = f"Auto commit at {now}"
        
        try:
            subprocess.run(['git', 'add', '.'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
            subprocess.run(['git', 'commit', '-m', commit_message], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
            
            # Lấy tên branch hiện tại
            branch_result = subprocess.run(
                ['git', 'branch', '--show-current'],
                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,text=True, check=True
            )
            branch = branch_result.stdout.strip()
            
            subprocess.run(['git', 'push', 'origin', branch], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
            print(f"Backup successful!'")
        
        except subprocess.CalledProcessError as e:
            print("❌ Error: ", e)

while True:
    auto_commit_push();
