import glob

filenamesList = glob.glob('*.c')
filenamesList += glob.glob('*.cpp')
filenamesList += glob.glob('*.h')
filenamesList += glob.glob('*.hpp')


filenamesList += glob.glob('test/*.c')
filenamesList += glob.glob('test/*.cpp')
filenamesList += glob.glob('test/*.h')
filenamesList += glob.glob('test/*.hpp')

filenamesList += glob.glob('tools/*.c')
filenamesList += glob.glob('tools/*.cpp')
filenamesList += glob.glob('tools/*.h')
filenamesList += glob.glob('tools/*.hpp')

print(filenamesList)



for filename in filenamesList:
    with open(filename, mode='r', encoding='shiftjis') as file:
        file_data = file.read()

    with open(filename, mode='w', encoding='utf8') as file:
        file.write(file_data)