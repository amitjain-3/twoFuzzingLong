#!/usr/bin/python 
import random
import string
sample_str = "900000000032100000000000000000000000000000000000000000000000654000000000000000000000000000000000000" 
sample_list = list(sample_str)
print(len(sample_list))

def Cloning(li1):
    li_copy = li1[:]
    return li_copy

with open('input_data_generator.txt', 'w') as file:
    for i in range(0,100):
        random_character = random.choice(string.ascii_letters)
        random_first_number = random.randint(7,9)
        random_first_characer = random.choice(string.ascii_letters)
        random_number = random.randint(0,9)
        random_index_number = random.randint(0,98)
        random_index_character = random.randint(0,98)
        new_list_to_write = Cloning(sample_list)
        new_list_to_write[random_index_character] = random_character
        new_list_to_write[random_index_number] = random_number
        if i%2 == 0: 
            new_list_to_write[0] = random_first_number
        else: 
            new_list_to_write[0] = random_first_characer
        random_input_string = ','.join(str(v) for v in new_list_to_write)
        random_input_string = random_input_string.replace(',', '')
        file.write(random_input_string + '\n')




    


with open('input_data_generator.txt') as input_file:
    lines = input_file.readlines()

for line in lines: 
    new_list = list(line)
    print(new_list)
    print("\n")
    #for element in list: 
    #    print(element + '')
    #print("\n")