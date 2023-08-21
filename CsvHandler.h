#ifndef __CSV_HANDLER_H__
#define __CSV_HANDLER_H__

#include <string>
#include <vector>


class CsvHandler
{
public:
    CsvHandler(const char* filename);
    ~CsvHandler();

    void Write(const std::vector<std::string>& values);
    std::vector<std::string> Read(size_t valuesToRead);

private:
    std::string filename;
    std::vector<std::vector<std::string>> linesToEntries;
};

#endif
