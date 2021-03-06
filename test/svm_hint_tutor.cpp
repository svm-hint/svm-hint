#include <iostream>
#include <vector>
#include <TChain.h>
#include <TFile.h>
#include <cmath>
#include <cstdlib>
#include <TRandom3.h>
#include "svm.h"
#include "libsvm_container.h"
#include "csvc_interface.h"
#include "timer.h"
using namespace std;
int main(){
  srand(1);  
  int  nvar = 4;
  /* SVM containers: in this example we will use auto split functionality therefore we will not use seperate containers for training and test - giving the right numbers here will only affect performance / you can give an estimation as well */
  svm_container svm(nvar,40000);
  svm_container svm_eval(nvar,20000);
  /* debugging histograms */  
  std::vector<TH1D*> bkg_hist_dump(nvar);  
  std::vector<TH1D*> sig_hist_dump(nvar);  
  TString name1, name2;
  for(Long_t ind = 0; ind < nvar; ind++ ){
    name1="feature_" ; name2 = "Background distribution of feature ";
    bkg_hist_dump.at(ind) = new TH1D (name1+ind+"bkg" , name2+ind, 110 , -5., 50.);
    name1= TString("feature_") ; name2 = TString("Signal distribution of feature ");
    sig_hist_dump.at(ind) = new TH1D (name1+ind+"sig" , name2+ind, 110 , -5., 50.);
  } 
  vector<double> vars(nvar);
  double weight = 0.; 
  size_t nbkg_tot  = 0;
  size_t nbkg_eval = 0;
  int sep = 0; 
  TFile *f1 = new TFile("svm_testtrain.root"); 
  TTree *traintest = (TTree*) f1->Get("tree");
  traintest->SetBranchAddress("var1",  &vars.at(0));
  traintest->SetBranchAddress("var2",  &vars.at(1));
  traintest->SetBranchAddress("var3",  &vars.at(2));
  traintest->SetBranchAddress("var4",  &vars.at(3));
  traintest->SetBranchAddress("sep",   &sep);
  traintest->SetBranchAddress("weight",&weight);
  int nentries = (int)traintest->GetEntries();
  for(int ievt = 0; ievt < nentries; ievt++){
    traintest->GetEntry(ievt);
    svm.set_feature(vars.at(0)); // first var
    svm.set_feature(vars.at(1)); // second var 
    svm.set_feature(vars.at(2)); // third var 
    svm.set_feature(vars.at(3)); // fourth var 
    svm.set_event(weight);
    if(sep == -1) nbkg_tot++;
  }
  f1->Close();
  TFile *f2 = new TFile("svm_eval.root"); 
  TTree *eval = (TTree*) f2->Get("tree");
  eval->SetBranchAddress("var1",  &vars.at(0));
  eval->SetBranchAddress("var2",  &vars.at(1));
  eval->SetBranchAddress("var3",  &vars.at(2));
  eval->SetBranchAddress("var4",  &vars.at(3));
  eval->SetBranchAddress("sep",   &sep);
  eval->SetBranchAddress("weight",&weight);
  nentries = (int)eval->GetEntries();
  for(int ievt = 0; ievt < nentries; ievt++){
    eval->GetEntry(ievt);
    svm_eval.set_feature(vars.at(0)); // first var
    svm_eval.set_feature(vars.at(1)); // second var 
    svm_eval.set_feature(vars.at(2)); // third var 
    svm_eval.set_feature(vars.at(3)); // fourth var 
    svm_eval.set_event(weight);
    if(sep == -1){
      nbkg_eval++;
      bkg_hist_dump.at(0)->Fill(vars.at(0), weight);
      bkg_hist_dump.at(1)->Fill(vars.at(1), weight);
      bkg_hist_dump.at(2)->Fill(vars.at(2), weight);
      bkg_hist_dump.at(3)->Fill(vars.at(3), weight);
    } else {
      sig_hist_dump.at(0)->Fill(vars.at(0), weight);
      sig_hist_dump.at(1)->Fill(vars.at(1), weight);
      sig_hist_dump.at(2)->Fill(vars.at(2), weight);
      sig_hist_dump.at(3)->Fill(vars.at(3), weight);
    }
  }
  TFile * tut = new TFile("tutorial_debug.root","RECREATE");
  for(Long_t ind = 0; ind < nvar; ind++ ){
    bkg_hist_dump.at(ind)->Write();
    sig_hist_dump.at(ind)->Write();
  } 
  tut->Close();
  /* The numbers should be given to the svm_interface */
  size_t nsamp_tot = svm.svm_cont->size();
  size_t nsig_tot  = nsamp_tot - nbkg_tot;
  size_t neval_tot = svm_eval.svm_cont->size();
  std::cout << " Total event number in the svm container: " << svm.svm_cont->size()      << 
    " total event number in the svm evaluation container: " << svm_eval.svm_cont->size() << std::endl;
  svm_interface * csvc = new csvc_interface(nsamp_tot,nbkg_tot,nsig_tot);
  svm_analyze stop;
  std::cout << svm.weights->size() <<std::endl;
  stop.set_filename("tutorial.root");
  stop.set_svm_interface(csvc);
  stop.setup_svm(svm);
  stop.set_eval(svm_eval,nbkg_eval);
  timer prob;
  //  stop.Obtain_probabilities(1, 1000.,0.575);// c , gamma
  prob.stop("svm training and test takes: ");
  timer tmain; 
  tmain.start();
  stop.Scan_parameters();
  tmain.stop("scanning parameters takes: ");
  return 0;
}
