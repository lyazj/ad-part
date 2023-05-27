#pragma once

#include <vector>
#include <string>

struct stat;

class ADListDir {

private:
  std::string dirpath;
  std::vector<std::string> entnames;

public:
  ADListDir(const std::string &dirpath_in = ".", int accept = DT_ALL);

  static const int DT_BLK;
  static const int DT_CHR;
  static const int DT_DIR;
  static const int DT_FIFO;
  static const int DT_LNK;
  static const int DT_REG;
  static const int DT_SOCK;
  static const int DT_UNKNOWN;
  static const int DT_ALL;

  std::vector<std::string> get_names() const { return entnames; }
  std::vector<std::string> get_full_names() const;

  ADListDir &sort_by_numbers();

};

class ADStat {

private:
  struct stat *sbuf;

public:
  ADStat(const char *path);
  ~ADStat();

  bool exists() const { return sbuf; }
  bool isreg() const;
  bool isdir() const;
  bool islnk() const;

  size_t size() const;

  explicit operator bool() const { return exists(); }

};
