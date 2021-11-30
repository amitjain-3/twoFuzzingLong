with open('temp.txt') as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    if not i%8:
        print(lines[i][:-1])
