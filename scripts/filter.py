with open('data/temp.txt') as f:
    lines = f.readlines()

lines = [[float(i.strip()) for i in l.split(',')] for l in lines]

def myFunc(e):
  return e[4]



for i, line in enumerate(lines):
    if not i%8:
        temp = sorted(lines[i:i+8], key=myFunc)
        print(", ".join([str(l) for l in temp[0]]))

