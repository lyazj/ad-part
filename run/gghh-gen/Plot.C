#include <classes/DelphesClasses.h>
#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TH1F.h>
#include <vector>
#include <set>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {

GenParticle *GetLastCopy(TClonesArray *particles, GenParticle *particle)
{
  for(;;) {
    Int_t d1 = particle->D1, d2 = particle->D2;
    if(d1 >= 0 && d1 == d2) {  // carbon copy
      particle = (GenParticle *)particles->At(d1);
      continue;
    }
    return particle;
  }
}

std::vector<GenParticle *> FindParticles(TClonesArray *particles, Int_t pid, GenParticle *mother = nullptr)
{
  std::set<GenParticle *> result_set;
  std::vector<GenParticle *> result;
  Int_t d1, d2, dd = 1;

  if(mother) {
    d1 = mother->D1, d2 = mother->D2;
    if(d1 < 0) return result;  // no daughter
    if(d1 == d2) throw std::invalid_argument("expect last copy");
    if(d2 < 0) d2 = d1;  // one daughter
    if(d1 > d2) {  // two daughters
      std::swap(d1, d2);
      dd = d2 - d1;
    }
  } else {
    d1 = 0, d2 = particles->GetEntries() - 1;
  }

  for(Int_t iparticle = d1; iparticle <= d2; iparticle += dd) {
    GenParticle *particle = (GenParticle *)particles->UncheckedAt(iparticle);
    if(std::abs(particle->PID) == pid) {  // found
      particle = GetLastCopy(particles, particle);
      if(!result_set.insert(particle).second) continue;  // duplicate
      result.push_back(particle);
    }
  }
  return result;
}

}

void Plot(const char *inpath, const char *outpath)
{
  auto infile = new TFile(inpath);
  auto tree = (TTree *)infile->Get("Delphes");
  auto particles = new TClonesArray("GenParticle");
  tree->SetBranchAddress("Particle", &particles);

  {
    auto outdir = fs::path(outpath).parent_path();
    if(!outdir.empty()) fs::create_directories(outdir);
  }
  auto outfile = new TFile(outpath, "RECREATE");
  auto pT_H = new TH1F("pT_H", ";p_{T}^{H} [GeV];Events", 50, 0, 1500);
  auto dR_b = new TH1F("dR_b", ";#DeltaR^{b};Events", 50, 0, 3);

  for(Long64_t ientry = 0; tree->GetEntry(ientry); ++ientry) {
    std::vector<GenParticle *> HH = FindParticles(particles, 25);
    if(HH.size() != 2) {
      std::cerr << "WARNING: Gen-Higgs number: " << HH.size() << std::endl;
      continue;
    }
    bool isHH4b = true;
    std::vector<std::vector<GenParticle *>> bbbb(2);
    for(size_t i = 0; i < 2; ++i) {
      bbbb[i] = FindParticles(particles, 5, HH[i]);
      if(bbbb[i].size() != 2) {
        std::cerr << "WARNING: Gen-b number: " << bbbb[i].size() << std::endl;
        isHH4b = false; break;
      }
    }
    if(!isHH4b) continue;  // [XXX]
    for(size_t i = 0; i < 2; ++i) {
      pT_H->Fill(HH[i]->PT);
      dR_b->Fill(bbbb[i][0]->P4().DeltaR(bbbb[i][1]->P4()));
    }
  }
  infile->Close();

  outfile->cd();
  pT_H->Write();
  dR_b->Write();
  outfile->Close();
}
