import sys

with open(r'Src/AddOnCommandTest.cpp', 'rb') as f:
    data = f.read()

# Remove anything not in ASCII range 0-127
clean_data = bytes(b for b in data if b < 128)

with open(r'Src/AddOnCommandTest.cpp', 'wb') as f:
    f.write(clean_data)
