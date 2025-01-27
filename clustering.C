//鳴ったファイバーがどれほど連続しているのか、連続したchの平均位置を追加したクラスタリングの1次元ヒストグラムを作成する。
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

int clustering(int run, string filename="") {
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

  TString histTitleX = Form("run%02d:continuationX", run);
  TString histTitleY = Form("run%02d:continuationY", run);
  TString histTitle2X = Form("run%02d:locationX", run);
  TString histTitle2Y = Form("run%02d:locationY", run);
  TString histTitle3 = Form("run%02d:location", run);

  TH1D* histX = new TH1D("histX", histTitleX, chX+1, 0, chX+1);
  TH1D* histY = new TH1D("histY", histTitleY, chY+1, 0, chY+1);
  TH1D* histX2 = new TH1D("histX2", histTitle2X, (chX+1)*2, 0, chX+1);
  TH1D* histY2 = new TH1D("histY2", histTitle2Y, (chY+1)*2, 0, chY+1);
  TH2D* hist3 = new TH2D("hist3", histTitle3, chX, chXmin, chXmin+chX, chY, chYmin, chYmin+chY);


  int n = tree->GetEntries();
  for(int i=0; i<n; i++){
    tree->GetEntry(i);
    vector<int> brightX, brightY;
    for(int j=0; j<32; j++){
      if(mppc_adc_data[j] > thre[j]){
	brightX.push_back(1);
      }else{
	brightX.push_back(0);
      }
      if(mppc_adc_data[j+32] > thre[j+32]){
	brightY.push_back(1);
      }else{
	brightY.push_back(0);
      }
    }
    std::vector<std::tuple<int, double>> resultsX = findContinuousOnes(brightX);
    std::vector<std::tuple<int, double>> resultsY = findContinuousOnes(brightY);
    for (const auto& [count, avgIndex] : resultsX) {
      histX -> Fill(count); //X座標で、ファイバーがどれほど連続して光ったかを数えてFill
      histX2 -> Fill(avgIndex); //X座標で、連続して光ったファイバー群の位置の平均を取って数えてFill
    }
    for (const auto& [count, avgIndex] : resultsY) {
      histY -> Fill(count);
      histY2 -> Fill(avgIndex);
    }
  }

TCanvas *c1 = new TCanvas("c1","", 1000, 500);
  c1->Divide(2,2);

  c1->cd(1);
  histX->Draw();
  c1->cd(2);
  histX2->Draw();

  c1->cd(3);
  histY->Draw();  
  c1->cd(4);
  histY2->Draw();

  if(filename!=""){
    c1->Print(filename.c_str());
  }
  return 0;
}
