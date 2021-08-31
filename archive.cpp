#include "archive.h"

#include <iostream>

#include <archive.h>
#include <archive_entry.h>

class Archive {};

Archive::Archive(std::string fn) {
  archive = self.archive_read_new();
  archive_read_support_filter_all(archive);
  archive_read_support_format_all(archive);
}
