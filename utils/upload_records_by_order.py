import os
import shutil
import time

"""
Clone recordings from a given folder to the mp3 sd card
The source must be organized in directories
Note: The script uploads all the recordings into the same directory (main), due to technical difficulties
"""

def clone_directory(src, dst):
    files_to_copy = []
    # Loop through all subdirectories and files in the source directory
    for dirpath, dirnames, filenames in os.walk(src):
        for filename in filenames:
            files_to_copy.append(os.path.join(dirpath, filename))

    files_to_copy = sorted(files_to_copy, key=lambda x: os.path.basename(x))
    for file in files_to_copy:
        dst_file = os.path.join(dst, os.path.basename(file))
        print(dst_file)
        shutil.copy2(file, dst_file)
        time.sleep(0.5)

src_dir = r""
dst_dir = "D:\\"

clone_directory(src_dir, dst_dir)
