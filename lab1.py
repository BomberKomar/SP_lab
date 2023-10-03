import re

Vowels='[eyuioaаеєиіїоуюя]'

def main():
    file_name = input("Enter your filename: \n")
    with open(file_name, 'r', encoding='utf-8') as file:
        file_content = file.read()
        words = re.findall(f'(?<!\w){Vowels}+(?!\w)', file_content, re.IGNORECASE)

        unique_words = set(word.lower() for word in words)
        
        print(set(unique_words))

if __name__ == "__main__":
    main()