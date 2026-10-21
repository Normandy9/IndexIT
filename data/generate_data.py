import os

def write_file(path, content):
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

os.makedirs('D:\\IndexIT_project\\data\\small_set', exist_ok=True)
os.makedirs('D:\\IndexIT_project\\data\\medium_set', exist_ok=True)

# Small set
write_file('D:\\IndexIT_project\\data\\small_set\\file1.txt', 'Hello world. This is a simple test document.')
write_file('D:\\IndexIT_project\\data\\small_set\\file2.txt', 'The quick brown fox jumps over the lazy dog. World of foxes.')
write_file('D:\\IndexIT_project\\data\\small_set\\file3.csv', 'id,name\n1,hello\n2,world')
write_file('D:\\IndexIT_project\\data\\small_set\\file4.md', '# Title\nMarkdown document with some text.')
write_file('D:\\IndexIT_project\\data\\small_set\\file5.log', 'INFO: System started.\nERROR: Something went wrong in the world.')

# Medium set
for i in range(200):
    write_file(f'D:\\IndexIT_project\\data\\medium_set\\doc_{i}.txt', f'This is document number {i}. It has some random words like search engine and indexing.')

print("Test data generated.")
