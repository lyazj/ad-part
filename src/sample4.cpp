#include "adenv.h"
#include "adsamp.h"
#include "adfs.h"
#include "adjet.h"
#include <stdio.h>
#include <ctype.h>
#include <zlib.h>
#include <err.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <stdexcept>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

using namespace std;

class Sampler {

public:
  Sampler(const string &dstdir, size_t nfrag);
  ~Sampler();

  // Must be added sequentially.
  void set_sample(const string &name, size_t label);
  void add_sample_file(const string &path);
  void sample();
  void save_sample_names(const string &path);
  double get_ratio();
  double set_ratio(double r);
  size_t get_nevent();
  size_t set_nevent(size_t n);

private:
  static unordered_map<string, size_t> expected_sample_size_table;
  static unordered_map<string, double> sample_weight_table;

  const string dstdir;
  size_t nfrag, frag_size, ifrag, ievent;
  double ratio;
  size_t nevent;
  vector<string> sample_names;
  vector<size_t> sample_labels;
  vector<vector<string>> sample_files;
  vector<size_t> isample_files;
  vector<size_t> sample_sizes;
  vector<size_t> expected_sample_sizes;
  vector<double> sample_weights;
  vector<array<gzFile, 6>> ifiles;
  array<gzFile, 6> ofile;

  void output_sample(size_t i);
  void new_file(size_t i);
  void new_frag();
};

unordered_map<string, size_t> Sampler::expected_sample_size_table = {
  {"QCD"            , 51400000},
  {"WJetsToQQ"      , 571000  },
  {"WJetsToLNu"     , 128000  },
  {"ZJetsToQQ"      , 225000  },
  {"ZJetsToLL"      , 25800   },
  {"ZJetsToNuNu"    , 22600   },
  {"TTbar"          , 246000  },
  {"SingleTop"      , 19300   },
  {"WW"             , 14000   },
  {"TW"             , 24900   },
  {"ZW"             , 10400   },
  {"ZZ"             , 1250    },
  {"TZ"             , 486     },
  {"SingleHiggs"    , 1630    },
  {"WH"             , 289     },
  {"ZH"             , 151     },
  {"TTbarH"         , 862     },
  {"TTbarW"         , 655     },
  {"TTbarZ"         , 1390    },
  {"SingleHiggsToBB", 1       },  // [XXX]
  {"DiHiggsTo4B"    , 1       },  // [XXX]
};

unordered_map<string, double> Sampler::sample_weight_table = {
  {"QCD"            , 51400000},
  {"WJetsToQQ"      , 571000  },
  {"WJetsToLNu"     , 128000  },
  {"ZJetsToQQ"      , 225000  },
  {"ZJetsToLL"      , 25800   },
  {"ZJetsToNuNu"    , 22600   },
  {"TTbar"          , 246000  },
  {"SingleTop"      , 19300   },
  {"WW"             , 14000   },
  {"TW"             , 24900   },
  {"ZW"             , 10400   },
  {"ZZ"             , 1250    },
  {"TZ"             , 486     },
  {"SingleHiggs"    , 1630    },
  {"WH"             , 289     },
  {"ZH"             , 151     },
  {"TTbarH"         , 862     },
  {"TTbarW"         , 655     },
  {"TTbarZ"         , 1390    },
  {"SingleHiggsToBB", 1       },  // [XXX]
  {"DiHiggsTo4B"    , 1       },  // [XXX]
};

Sampler::Sampler(const string &dstdir_in, size_t nfrag_in)
  : dstdir(dstdir_in), nfrag(nfrag_in), frag_size(0), ifrag(0), ievent(0), ratio(0.0), nevent(0), ofile{NULL}
{
  if(mkdir(dstdir.c_str(), 0755) && errno != EEXIST) throw runtime_error(strerror(errno));
}

Sampler::~Sampler()
{
  for(auto &ifile : ifiles) {
    for(size_t i = 0; i < 6; ++i) {
      if(ifile[i]) gzclose(ifile[i]);
    }
  }
  for(size_t i = 0; i < 6; ++i) {
    if(ofile[i]) gzclose(ofile[i]);
  }
}

void Sampler::set_sample(const string &name, size_t label)
{
  sample_names.push_back(name);
  sample_labels.push_back(label);
  sample_files.emplace_back();
  isample_files.push_back(0);
  sample_sizes.push_back(0);
  expected_sample_sizes.push_back(expected_sample_size_table.at(name));
  sample_weights.push_back(sample_weight_table.at(name));
  ifiles.emplace_back(array<gzFile, 6>{NULL});
}

void Sampler::add_sample_file(const string &path)
{
  ADEvent event;
  gzFile file = gzopen(path.c_str(), "rb");
  if(file == NULL) {
    fprintf(stderr, "WARNING: error opening file: %s\n", path.c_str());
    return;
  }
  size_t size = 0;
  while(event.read(file)) ++size;
  gzclose(file);
  fprintf(stderr, "INFO: file size: %zu\n", size);
  if(size == 0) return;

  sample_files.back().push_back(path);
  sample_sizes.back() += size;
}

double Sampler::get_ratio()
{
  if(ratio) return ratio;
  if(sample_sizes.empty()) return ratio;
  ratio = 1.0 / 0.0;
  for(size_t i = 0; i < sample_sizes.size(); ++i) {
    double cur_ratio = (double)sample_sizes[i] / expected_sample_sizes[i];
    fprintf(stderr, "INFO: sampling ratio for %s: %lg\n", sample_names[i].c_str(), cur_ratio);
    ratio = min(ratio, cur_ratio);
  }
  return ratio;
}

double Sampler::set_ratio(double r)
{
  return ratio = min(get_ratio(), r);
}

size_t Sampler::get_nevent()
{
  double r = get_ratio();
  if(nevent) return nevent;
  for(size_t i = 0; i < sample_sizes.size(); ++i) {
    nevent += min<size_t>(sample_sizes[i], r * expected_sample_sizes[i] + 0.5);
  }
  return nevent;
}

size_t Sampler::set_nevent(size_t n)
{
  return nevent = min(get_nevent(), n);
}

void Sampler::sample()
{
  double r = get_ratio();
  fprintf(stderr, "INFO: overall sampling ratio: %lg\n", r);

  size_t total = 0;
  for(size_t i = 0; i < sample_sizes.size(); ++i) {
    total += sample_sizes[i] = min<size_t>(sample_sizes[i], r * expected_sample_sizes[i] + 0.5);
  }
  nevent = min(nevent, total);
  ievent = frag_size = (nevent + nfrag - 1) / nfrag;
  fprintf(stderr, "INFO: fragment size: %zu\n", frag_size);

  ADSampler sampler;
  for(size_t i = 0; i < sample_sizes.size(); ++i) {
    sampler.add_sample(sample_sizes[i], sample_weights[i]);
  }
  size_t id, i = nevent;
  while(i && (id = sampler.sample())) {
    --i;
    output_sample(id - 1);
  }
}

void Sampler::output_sample(size_t i)
{
  if(ievent == frag_size) {
    new_frag();
  }

  if(ifiles[i][0] == NULL) new_file(i);
  for(;;) {
    ADEvent evt;
    if(!evt.read(ifiles[i][0])) {
      new_file(i); continue;
    }
    evt.label = sample_labels[i];
    evt.write(ofile[0]);

    size_t njet = evt.njet;
    ADJet jet;
    ADParTOutput out;
    ADParTHidden hid;
    for(size_t j = 0; j < njet; ++j) {
      if(!jet.read(ifiles[i][1])) errx(1, "error reading jet");
      jet.write_without_particles(ofile[1]);
      if(!out.read(ifiles[i][4])) errx(4, "error reading out");
      out.write(ofile[4]);
      if(!hid.read(ifiles[i][5])) errx(5, "error reading hid");
      hid.write(ofile[5]);
    }

    size_t nlep = evt.nlep;
    ADLepton lep;
    for(size_t j = 0; j < nlep; ++j) {
      if(!lep.read(ifiles[i][2])) errx(2, "error reading lep");
      lep.write(ofile[2]);
    }

    size_t npho = evt.npho;
    ADPhoton pho;
    for(size_t j = 0; j < npho; ++j) {
      if(!pho.read(ifiles[i][3])) errx(3, "error reading pho");
      pho.write(ofile[3]);
    }

    break;
  }

  ++ievent;
}

void Sampler::new_file(size_t i)
{
  if(isample_files[i] == sample_files[i].size()) abort();

  for(size_t j = 0; j < 6; ++j) gzclose(ifiles[i][j]);
  const string &sample_file = sample_files[i][isample_files[i]];
  fprintf(stderr, "INFO: new input file: %s\n", sample_file.c_str());
  ifiles[i][0] = gzopen(sample_file.c_str(), "rb");
  ifiles[i][1] = gzopen((sample_file.substr(0, sample_file.length() - 10) + ".gz").c_str(), "rb");
  ifiles[i][2] = gzopen((sample_file.substr(0, sample_file.length() - 10) + "_leptons.gz").c_str(), "rb");
  ifiles[i][3] = gzopen((sample_file.substr(0, sample_file.length() - 10) + "_photons.gz").c_str(), "rb");
  ifiles[i][4] = gzopen((sample_file.substr(0, sample_file.length() - 10) + "_part_out.gz").c_str(), "rb");
  ifiles[i][5] = gzopen((sample_file.substr(0, sample_file.length() - 10) + "_part_hid.gz").c_str(), "rb");
  for(size_t j = 0; j < 6; ++j) {
    if(ifiles[i][j] == NULL) err(1, "gzopen");
  }
  ++isample_files[i];
}

void Sampler::new_frag()
{
  if(ifrag == nfrag) abort();

  for(size_t j = 0; j < 6; ++j) gzclose(ofile[j]);
  const string &frag_file = dstdir + "/"s + to_string(ifrag) + "_events.gz";
  fprintf(stderr, "INFO: new output file: %s\n", frag_file.c_str());
  ofile[0] = gzopen(frag_file.c_str(), "wb");
  ofile[1] = gzopen((frag_file.substr(0, frag_file.length() - 10) + ".gz").c_str(), "wb");
  ofile[2] = gzopen((frag_file.substr(0, frag_file.length() - 10) + "_leptons.gz").c_str(), "wb");
  ofile[3] = gzopen((frag_file.substr(0, frag_file.length() - 10) + "_photons.gz").c_str(), "wb");
  ofile[4] = gzopen((frag_file.substr(0, frag_file.length() - 10) + "_part_out.gz").c_str(), "wb");
  ofile[5] = gzopen((frag_file.substr(0, frag_file.length() - 10) + "_part_hid.gz").c_str(), "wb");
  for(size_t j = 0; j < 6; ++j) {
    if(ofile[j] == NULL) err(1, "gzopen");
  }
  ++ifrag;
  ievent = 0;
}

void Sampler::save_sample_names(const string &path)
{
  FILE *file = fopen(path.c_str(), "a");
  for(size_t i = 0; i < sample_names.size(); ++i) {
    fprintf(file, "%zu\t%s\n", i, sample_names[i].c_str());
  }
  fclose(file);
}

unordered_map<string, size_t> label_table = {
  // Signal
  // --------------------
  {"SingleHiggsToBB", 0},
  {"DiHiggsTo4B"    , 0},

  // QCD
  // --------------------
  {"QCD"            , 1},

  // VJets
  // --------------------
  {"WJetsToLNu"     , 2},
  {"WJetsToQQ"      , 2},
  {"ZJetsToLL"      , 2},
  {"ZJetsToNuNu"    , 2},
  {"ZJetsToQQ"      , 2},

  // TTbar
  // --------------------
  {"TTbar"          , 3},
};

const char *label_names[] = {
  "Signal",
  "QCD",
  "VJets",
  "TTbar",
};

int main(int argc, char *argv[])
{
  setlbf();

  if(argc != 4) {
    fprintf(stderr, "usage: %s <srcdir> <dsrdir> <nfrag>\n", get_invoc_short_name());
    return 1;
  }
  const char *srcdir = argv[1];
  const char *dstdir = argv[2];
  size_t nfrag = stoull(argv[3]);

  Sampler samplers[] = {
    {dstdir + "/"s + label_names[0], nfrag},
    {dstdir + "/"s + label_names[1], nfrag},
    {dstdir + "/"s + label_names[2], nfrag},
    {dstdir + "/"s + label_names[3], nfrag},
  };

  ADListDir src(srcdir, ADListDir::DT_DIR);
  vector<string> src_names = src.get_full_names();
  sort(src_names.begin(), src_names.end());
  for(const string &srcsamp : src_names) {
    const string &srcbase = basename(srcsamp);
    if(srcbase == "." || srcbase == "..") continue;
    auto label_it = label_table.find(srcbase);
    if(label_it == label_table.end()) continue;
    size_t label = label_it->second;
    fprintf(stderr, "INFO: adding [%zu] %s\n", label, srcsamp.c_str());
    samplers[label].set_sample(srcbase, label);

    ADListDir srclst(srcsamp);
    srclst.sort_by_numbers();
    for(const string &srcfile : srclst.get_full_names()) {
      if(srcfile.length() < 4 || srcfile.substr(srcfile.length() - 10) != "_events.gz") continue;
      fprintf(stderr, "INFO: adding %s\n", srcfile.c_str());
      samplers[label].add_sample_file(srcfile);
    }
  }

  size_t nevent = -1;
  remove((get_invoc_short_name() + ".txt"s).c_str());
  for(size_t i = 0; i < 4; ++i) {
    samplers[i].save_sample_names((get_invoc_short_name() + ".txt"s).c_str());
    printf("INFO: sampling events for %s: %zu\n", label_names[i], samplers[i].get_nevent());
    nevent = min(nevent, samplers[i].get_nevent());
  }
  fprintf(stderr, "INFO: minimal sampling events: %zu\n", nevent);
  for(size_t i = 0; i < 4; ++i) {
    samplers[i].set_nevent(nevent);
    samplers[i].sample();
  }
  return 0;
}
