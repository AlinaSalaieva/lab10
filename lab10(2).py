import os
import shutil

current_directory = os.getcwd()
files = os.listdir(current_directory)

print("Список файлів поточного каталогу:")
for file in files:
    if os.path.isfile(file):
        print(file)

destination_folder = os.path.join(current_directory, 'R_FILES')
os.makedirs(destination_folder, exist_ok=True)

for file in files:
    if os.path.isfile(file) and file.lower().startswith('s'):
        shutil.copy(file, destination_folder)
        print(f"Файл {file} скопійовано у папку {destination_folder}")

print("Копіювання завершено")

