//
// Created by jan on 12/10/18.
//

#ifndef LFNS_TXTFILEREADER_H
#define LFNS_TXTFILEREADER_H

#include <string>
#include <vector>

namespace io {
    class TxtFileReader {
    public:
        TxtFileReader(std::string txt_file_name);
        virtual ~TxtFileReader();

    protected:
        std::vector<std::string> _txt_file_lines;

        std::vector<std::string> _readLineWithCaption(std::string Caption, std::string delimiter);
        std::vector<std::string> _readStrippedDelimitterdLine(std::string line, std::string delimiter);
        std::vector<std::string> _splitInTwo(std::string line, std::string delimiter);
        std::vector<std::string> _readCaptionLines(std::string caption);
    };
}


#endif //LFNS_TXTFILEREADER_H
