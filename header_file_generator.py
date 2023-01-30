import os
import re

"""
Build the header file for the arduino which contains all the addresses for the recordings and division to categories.
Inputs: source_path - the folder which contains the records divided into groups (by folders)
        header_file_name - The path to the header file to be compiled          
"""

do_not_touch_files = ['-01_000_-01_001_001_IndexerVolumeGuid', '000_001_WPSettings.dat', '001_002_IndexerVolumeGuid',
                      '002_-01_000_002_002_WPSettings.dat', '003_WPSettings.dat', '004_IndexerVolumeGuid',
                      'WPSettings.dat', 'IndexerVolumeGuid']

def build_header_file(source_path, header_file_name):
    # Create a list to store the names and values of all recordings
    recordings = []
    # Iterate over all subdirectories in the given directory
    for subdir, dirs, files in os.walk(source_path):
        for file in files:
            if file not in do_not_touch_files:
                # Extract the recording name and value from the file name
                match = re.match(r'(\d+)_(.*)\.mp3', file)
                if match:
                    value = int(match.group(1))
                    name = match.group(2)
                    recordings.append((name, value))
    # Sort the recordings by value
    recordings.sort(key=lambda x: x[1])
    # Create the header file
    with open(header_file_name, 'w') as f:
        # Write the constant variables for each recording
        for recording in recordings:
            f.write(f'const int8_t {recording[0]} = 0x{recording[1]:02X};\n')
        # Write the array of arrays for the recordings
        dirs = os.listdir(source_path)
        num_of_libraries = len(dirs)
        biggest_library_length = max([len(files) for _, _, files in os.walk(source_path)])
        f.write('\n')
        f.write(f'const int numOfLibraries = {num_of_libraries};\n')
        f.write(f'const int biggestLibraryLength = {biggest_library_length};\n')
        f.write('\n')
        f.write('const int8_t libraries[numOfLibraries][biggestLibraryLength] = {\n')
        for subdir, dirs, files in os.walk(source_path):
            if subdir != source_path:
                    f.write(f'  {{ //{os.path.basename(subdir)}\n')
                    for file in files:
                        match = re.match(r'(\d+)_(.*)\.mp3', file)
                        if match:
                            name = match.group(2)
                            f.write(f'    {name},\n')
                    f.write('  },\n')
        f.write('};\n')
        # Write the array for the lengths of each inner array
        f.write('const int8_t librariesLengths[numOfLibraries] = {\n')
        for subdir, dirs, files in os.walk(source_path):
            if subdir != source_path:
                f.write(f'  {len(files)},\n')
        f.write('};\n')

build_header_file(source_path=r"C:\Users\Yonatan.Sharabany\PycharmProjects\arduino\yomuledim\yomuledim\recordings",
                  header_file_name=r'C:\Users\Yonatan.Sharabany\PycharmProjects\arduino\yomuledim\yomuledim\recordings.h')