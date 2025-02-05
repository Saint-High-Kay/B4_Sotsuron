//連続して光ったクラスター数と各チャンネルにおけるクラスター数の分布を規格化して作成する。
#include <iostream>
#include <vector>
#include <tuple> // For std::tuple

std::vector<std::tuple<int, double>> findContinuousOnes(const std::vector<int>& arr) {
    std::vector<std::tuple<int, double>> results;
    int start = -1;
    double sum = 0.0;
    int count = 0;

    for (size_t i = 0; i <= arr.size(); ++i) {
        if (i < arr.size() && arr[i] == 1) {
            if (start == -1) start = i; // 新しい連続の開始点を記録
            sum += i;
            ++count;
        } else {
            if (start != -1) { // 連続が終了した場合
                double avgIndex = sum / count;
                results.emplace_back(count, avgIndex);
                start = -1;
                sum = 0.0;
                count = 0;
            }
        }
    }
    return results;
}

int clusteringN(int run, string filename="") {
  gStyle->SetOptStat(0);
  TString datafile = Form("/data/B4_beam2024/root/%04d.root", run);
  TString threfile = "threshold.txt";
  TString histTitle = Form("run%02d", run);  

  std::ifstream fin(threfile);
  double thre[64];
  for(int i=0;i<64;i++){
    fin >> thre[i];
  }

  TFile *file = TFile::Open(datafile, "READ");
  TTree *tree = (TTree *)file->Get("tree");
  int mppc_adc_data[64];

  tree->SetBranchAddress("mppc_adc", mppc_adc_data);

  int chX = 32;
  int chXmin = 0;
  int chXoff = 0;
  int chY = 32;
  int chYmin = 0;
  int chYoff = 32;

  TString histTitleX = Form("run%02d:BlueX_RedY_continuation", run);
  TString histTitleY = Form("run%02d:Y_continuation", run);
  TString histTitle2X = Form("run%02d:BlueX_RedY_location", run);
  TString histTitle2Y = Form("run%02d:Y_location", run);
//  TString histTitle3 = Form("run%02d:location", run);

//前半2つは連続、後半2つは位置の1次元ヒストグラム
  TH1D* histX = new TH1D("histX", histTitleX, chX+1, 0, chX+1);
  TH1D* histY = new TH1D("histY", histTitleY, chY+1, 0, chY+1);
  TH1D* histX2 = new TH1D("histX2", histTitle2X, chX+1, 0, chX+1);
  TH1D* histY2 = new TH1D("histY2", histTitle2Y, chY+1, 0, chY+1);
//  TH2D* hist3 = new TH2D("hist3", histTitle3, chX, chXmin, chXmin+chX, chY, chYmin, chYmin+chY);

//スレショルドは、平均光電子数のデータを加工したものを使用する。
  ifstream faho("/home/rinrin/Beamtest_after/geant4/line-param_after201.txt");
  int a, b;
  double c, d;
  vector<double> threshold, inc, sec;
  while (faho >> a >> b >> c >> d)
  {
    inc.push_back(d);
    sec.push_back(c);
  }
  faho.close();

  double thre_sine;
  vector<double> thre_photon;
  ifstream sine("/home/rinrin/geant4/g4sim/EfficiencySim/analyze/txt/thre_photon.txt");
  while (sine >> thre_sine)
  {
    thre_photon.push_back(thre_sine);
  }
  sine.close();

  int n = tree->GetEntries();
  for(int i=0; i<n; i++){
    tree->GetEntry(i);
    vector<int> brightX, brightY;
    for(int j=0; j<32; j++){
      if(mppc_adc_data[j] > (((thre_photon.at(j)) - sec.at(j)) / inc.at(j))){
	brightX.push_back(1);
      }else{
	brightX.push_back(0);
      }
      if(mppc_adc_data[j+32] > (((thre_photon.at(j + 32)) - sec.at(j + 32)) / inc.at(j + 32))){
	brightY.push_back(1);
      }else{
	brightY.push_back(0);
      }
    }
    std::vector<std::tuple<int, double>> resultsX = findContinuousOnes(brightX);
    std::vector<std::tuple<int, double>> resultsY = findContinuousOnes(brightY);
    for (const auto& [count, avgIndex] : resultsX) {
      histX -> Fill(count); 
      histX2 -> Fill(avgIndex); 
    }
    for (const auto& [count, avgIndex] : resultsY) {
      histY -> Fill(count);
      histY2 -> Fill(avgIndex);
    }
  }

TCanvas *c1 = new TCanvas("c1","", 1000, 500);

// 規格化（積分値を1にする）
 histX->Scale(1.0 / histX->Integral());
 histY->Scale(1.0 / histY->Integral());
 histX2->Scale(1.0 / histX2->Integral());
 histY2->Scale(1.0 / histY2->Integral());

// ヒストグラムの見た目を調整
 histX->SetLineColor(kBlue);
 histX->SetLineWidth(3); 
 histY->SetLineColor(kRed); 
 histY->SetLineWidth(2);
 histX2->SetLineColor(kBlue);
 histX2->SetLineWidth(3); 
 histY2->SetLineColor(kRed); 
 histY2->SetLineWidth(2);

// Y軸の範囲を調整
 histX->GetYaxis()->SetRangeUser(0, 1.2 * std::max(histX->GetMaximum(),　histY->GetMaximum()));
 histX2->GetYaxis()->SetRangeUser(0, 1.2 * std::max(histX2->GetMaximum(),　histY2->GetMaximum()));


  c1->Divide(2,1);

  c1->cd(1);
  histX->Draw("hist");
  c1->cd(2);
  histX2->Draw("hist");
  c1->cd(3);
  histY->Draw("hist");  
  c1->cd(4);
  histY2->Draw("hist");

  if(filename!=""){
    c1->Print(filename.c_str());
  }
  return 0;
}
