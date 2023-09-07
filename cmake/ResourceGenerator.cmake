function(generate_resources_file)
    message(STATUS "File name to generate ${ARGV0}")

    set(generatedFile "// Generated file - do not edit

#include <string.h>
#include \"resources.h\"

")

    set(i 1)
    while(NOT ${i} STREQUAL ${ARGC})
        file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${ARGV${i}} currentFileData HEX)

        set(generatedFile "${generatedFile}
static const unsigned char resource_${i}_buffer[] = {")


        set(elementCounter 0)
        set(byteCounter 0)
        string(LENGTH ${currentFileData} hexLength)
        while(NOT byteCounter STREQUAL hexLength)
            string(SUBSTRING ${currentFileData} ${byteCounter} 2 byteHex)
            set(generatedFile "${generatedFile} 0x${byteHex}, ")

            if (elementCounter STREQUAL 16)
                set(generatedFile "${generatedFile} \n\t")
            endif()
            # Wildly too slow to be practical. Probably took 15 minutes to run.
            #math(EXPR byteDecimal "0x${byteHex}" OUTPUT_FORMAT DECIMAL)
            #set(generatedFile "${generatedFile} ${byteDecimal}, ")

            math(EXPR byteCounter "${byteCounter}+2")
            math(EXPR elementCounter "${elementCounter}+1")
            
            #message(STATUS "${hexLength} ${byteCounter}")
        endwhile()

        set(generatedFile "${generatedFile}
}
")
        message(STATUS "${ARGV${i}}")
        math(EXPR i "${i}+1")
    endwhile()



    set(generatedFile "${generatedFile}
static const InternalResource resource_list[] = {")

    
    set(i 1)
    while(NOT ${i} STREQUAL ${ARGC})
        file(SIZE ${CMAKE_CURRENT_SOURCE_DIR}/${ARGV${i}} currentFileSize)
        set(generatedFile "${generatedFile}
	{ \"${ARGV${i}}\", resource_${i}_buffer, ${currentFileSize} },")
        
        message(STATUS "${ARGV${i}}")
        math(EXPR i "${i}+1")
    endwhile()

    
    set(generatedFile "${generatedFile}
	{ 0, 0, 0 }
};

const InternalResource* getResource(const char* filename)
{
	for (const InternalResource* rv = resource_list; rv->buffer; ++rv) {
		if (strcmp(rv->filename, filename) == 0) return rv;
	}
	return NULL;
}

")

    file(WRITE ${ARGV0} ${generatedFile})
endfunction()