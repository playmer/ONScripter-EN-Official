message(STATUS "Arg Count: ${CMAKE_ARGC}")

set(i 0)
while(NOT ${i} STREQUAL ${CMAKE_ARGC})
    message(STATUS "Arg${i} ${CMAKE_ARGV${i}}")
    math(EXPR i "${i}+1")
endwhile()


message(STATUS "File name to generate ${CMAKE_ARGV4}")

set(generatedFile "// Generated file - do not edit\n\n#include <string.h>\n#include \"resources.h\"\n\n")

set(i 5)
while(NOT ${i} STREQUAL ${CMAKE_ARGC})
    message(STATUS "File name to blit ${CMAKE_ARGV${i}}")

    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_ARGV${i}} currentFileData HEX)
    string(REGEX MATCHALL "([A-Fa-f0-9][A-Fa-f0-9])" currentFileDataList ${currentFileData})

    set(generatedFile "${generatedFile}\nstatic const unsigned char resource_${i}_buffer[] = {\n\t")

    set(elementCounter 0)
    foreach(byteHex IN LISTS currentFileDataList)
        math(EXPR byteDecimal "0x${byteHex}" OUTPUT_FORMAT DECIMAL)
        string(APPEND generatedFile "${byteDecimal}, ")

        math(EXPR elementCounter "${elementCounter}+1")
        if (elementCounter GREATER 16)
            set(generatedFile "${generatedFile} \n\t")
            set(elementCounter 0)
        endif()
    endforeach()

    set(generatedFile "${generatedFile}\n}\n")
    message(STATUS "${CMAKE_ARGV${i}}")
    math(EXPR i "${i}+1")
endwhile()

set(generatedFile "${generatedFile}\nstatic const InternalResource resource_list[] = {")


set(i 5)
while(NOT ${i} STREQUAL ${CMAKE_ARGC})
    file(SIZE ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_ARGV${i}} currentFileSize)
    set(generatedFile "${generatedFile}\n\t{ \"${CMAKE_ARGV${i}}\", resource_${i}_buffer, ${currentFileSize} },")
    
    message(STATUS "${CMAKE_ARGV${i}}")
    math(EXPR i "${i}+1")
endwhile()

set(generatedFile "${generatedFile}\n\t{ 0, 0, 0 }\n};\n
const InternalResource* getResource(const char* filename)
{
	for (const InternalResource* rv = resource_list; rv->buffer; ++rv) {
		if (strcmp(rv->filename, filename) == 0) return rv;
	}
	return NULL;
}\n\n")

file(WRITE ${CMAKE_ARGV4} ${generatedFile})