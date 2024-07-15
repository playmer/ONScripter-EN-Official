// A simple program to create resources.cpp
// Usage: embed [input1 internalname1 input2...]

#include <stdio.h>
#include <vector>

struct item {
    const char *file;
    int len;
    item(): file(0), len(0) {};
    item(const char *f, int l): file(f), len(l) {};
};

int main(int argc, char** argv)
{
    std::vector<item> names;

    puts("// Generated file - do not edit");
    puts("");
    puts("#include <string.h>");
    puts("#include \"resources.h\"");
    for (int i = 1, j = 1; i < argc; i += 1, ++j) {
        FILE* f = fopen(argv[i], "rb");
        if (f) {
            int len = 0, c;
            printf("\nstatic const unsigned char resource_%d_buffer[] = {", j);
            while ((c = getc(f)) != EOF) {
                if (len) putchar(',');

                if (len++ % 16 == 0) printf("\n\t");else putchar(' ');

                printf("%3d", c);
            }
            fclose(f);
            puts("\n};");
            names.emplace_back(argv[i], len);
        }
    }

    printf("\nstatic const InternalResource resource_list[] = {");
    int i = 1;
    
    for (auto iptr = names.begin(); iptr != names.end(); ++iptr, ++i) {
        printf("\n\t{ \"%s\", resource_%d_buffer, %d },",
               iptr->file, i, iptr->len);
    }

    puts("\n\t{ 0, 0, 0 }\n};");
    puts("");
    puts("const InternalResource* getResource(const char* filename)");
    puts("{");
    puts("\tfor (const InternalResource* rv = resource_list; rv->buffer; ++rv) {");
    puts("\t\tif (strcmp(rv->filename, filename) == 0) return rv;");
    puts("\t}");
    puts("\treturn NULL;");
    puts("}");
}
