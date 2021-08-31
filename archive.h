#ifndef ARCHIVE_H_
#define ARCHIVE_H_

#include <functional>
#include <iostream>

#include <archive.h>

class Archive {
  std::string filename;
  archive *archive;

public:
  Archive(std::string fn);
  void process_data(std::function<bool(std::string_view)>);
};

#endif // ARCHIVE_H_
