#include <TFile.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>

void ExportPlot()
{
  auto file = new TFile("plot/DiHiggsTo4B.root");
  auto pT_H = (TH1 *)file->Get("pT_H");
  auto dR_b = (TH1 *)file->Get("dR_b");

  auto file_gghh = new TFile("../gghh-gen/plot/ggHH_kl_1_kt_1.root");
  auto pT_H_gghh = (TH1 *)file_gghh->Get("pT_H");
  auto dR_b_gghh = (TH1 *)file_gghh->Get("dR_b");

  gStyle->SetHistLineWidth(2);
  TCanvas *canvas = new TCanvas("canvas", "canvas", 3000, 3600);
  canvas->Divide(1, 2);

  canvas->cd(1);
  pT_H->SetName("HH");
  pT_H->SetLineColor(2);
  pT_H->SetTitle("");
  pT_H->SetStats(0);
  pT_H->Draw();
  pT_H_gghh->SetName("ggHH");
  pT_H_gghh->SetLineColor(3);
  pT_H_gghh->Draw("Same");
  canvas->GetPad(1)->BuildLegend(0.7, 0.8, 0.9, 0.9)->Draw();

  canvas->cd(2);
  dR_b->SetName("HH");
  dR_b->SetLineColor(2);
  dR_b->SetTitle("");
  dR_b->SetStats(0);
  dR_b->Draw();
  dR_b_gghh->SetName("ggHH");
  dR_b_gghh->SetLineColor(3);
  dR_b_gghh->Draw("Same");
  canvas->GetPad(2)->BuildLegend(0.7, 0.8, 0.9, 0.9)->Draw();

  canvas->Draw();
  canvas->SaveAs("HH.pdf");

  delete canvas;
  file_gghh->Close();
  file->Close();
}
