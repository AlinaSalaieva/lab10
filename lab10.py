from collections import Counter
import os

file_path = 'start.txt'
if not os.path.isfile(file_path):
    print(f"Файл {file_path} не знайдено.")
else:
    with open(file_path, 'r', encoding='utf-8') as file:
        text = file.read().lower()

    ukrainian_letters = 'абвгґдежзийїклмнопрстуфхцчшщьюя'
    letter_counts = Counter(c for c in text if c in ukrainian_letters)

    print(f"Частота літер: {letter_counts}")

    output_file_path = 'fin.txt'
    try:
        with open(output_file_path, 'w', encoding='utf-8') as file:
            for letter, count in letter_counts.items():
                file.write(f'{letter}: {count}\n')
        print("Частоти літер записані у файл fin.txt")
    except Exception as e:
        print(f"Помилка при запису у файл: {e}")

if os.path.isfile(output_file_path):
    with open(output_file_path, 'r', encoding='utf-8') as file:
        content = file.read()
        print(f"Вміст файлу fin.txt:\n{content}")
else:
    print(f"Файл {output_file_path} не був створений.")



