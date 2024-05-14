import sys

print('File to write: ', sys.argv[1])

fileData =  '// Generated file - do not edit\n'
fileData += '\n'
fileData += '#include <string.h>\n'
fileData += '#include "resources.h"\n'
fileData += '\n'

files = []


filesToBlit = sys.argv[2:len(sys.argv): 1]

for i, fileToBlit in enumerate(filesToBlit):
    print('File to blit: ', fileToBlit)
    fileData += f'static const unsigned char resource_{i}_buffer[] = ' + '{'
    fileData += '\t'

    with open(fileToBlit, "rb") as blitFile:
        fileToBlitBytes = bytearray(blitFile.read())
        files.append((fileToBlit, len(fileToBlitBytes)))

        for j, byte in enumerate(fileToBlitBytes):
            if j % 16 == 0:
                fileData += "\n\t"

            fileData += str(byte).rjust(3, ' ')
            fileData += ', '
    
    fileData += '\n'
    fileData += "};\n\n"



fileData += "static const InternalResource resource_list[] = {\n"

for i, (name, size) in enumerate(files):
    fileData += "\t{ " + f'"{name}", resource_{i}_buffer, {size}' + " },\n"

fileData += '\t{ 0, 0, 0 }\n'
fileData += '};\n'
fileData += '\n'
fileData += 'const InternalResource* getResource(const char* filename)\n'
fileData += '{\n'
fileData += '\tfor (const InternalResource* rv = resource_list; rv->buffer; ++rv) {\n'
fileData += '\t\tif (strcmp(rv->filename, filename) == 0) return rv;\n'
fileData += '\t}\n'
fileData += '\treturn NULL;\n'
fileData += '}\n'

text_file = open(sys.argv[1], "w")
n = text_file.write(fileData)
text_file.close()
