import random

numbers = [random.randint(1, 1000) for _ in range(10000)]

with open('numbers.txt', 'w') as f:
    f.write("1000000\n") 
    for num in numbers:
        f.write(f"{num}\n")

print("Generated numbers.txt with 1000000 numbers")
