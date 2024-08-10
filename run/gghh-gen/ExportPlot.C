#include <TFile.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <iostream>
#include <regex>
#include <algorithm>

namespace fs = std::filesystem;

void ExportPlot()
{
  gStyle->SetHistLineWidth(2);
  TCanvas *canvas = new TCanvas("canvas", "canvas", 3000, 3600);
  canvas->Divide(1, 2);

  std::vector<std::string> names;
  for(const auto &path : fs::directory_iterator("plot")) {
    if(std::regex_match(path.path().string(), std::regex(".*\\.root$"))) {
      names.emplace_back(path.path().filename().string());
      names.back().resize(names.back().size() - 5);  // Remove ".root".
    }
  }
  sort(names.begin(), names.end());

  std::vector<std::unique_ptr<TFile>> files;
  files.reserve(names.size());
  for(const std::string &name : names) {
    files.emplace_back(new TFile(("plot/" + name + ".root").c_str()));
  }

  canvas->cd(1);
  for(size_t i = 0; i < files.size(); ++i) {
    auto pT_H = (TH1 *)files[i]->Get("pT_H");
    pT_H->SetName(names[i].c_str());
    pT_H->SetLineColor(2 + i);
    pT_H->SetTitle("");
    pT_H->SetYTitle("Density");
    pT_H->SetStats(0);
    pT_H->DrawNormalized(i ? "Same" : "");
  }
  canvas->GetPad(1)->BuildLegend(0.7, 0.8, 0.9, 0.9)->Draw();

  canvas->cd(2);
  for(size_t i = 0; i < files.size(); ++i) {
    auto dR_b = (TH1 *)files[i]->Get("dR_b");
    dR_b->SetName(names[i].c_str());
    dR_b->SetLineColor(2 + i);
    dR_b->SetTitle("");
    dR_b->SetYTitle("Density");
    dR_b->SetStats(0);
    dR_b->DrawNormalized(i ? "Same" : "");
  }
  canvas->GetPad(1)->BuildLegend(0.7, 0.8, 0.9, 0.9)->Draw();

  canvas->Draw();
  canvas->SaveAs("ggHH.pdf");
  delete canvas;
}
