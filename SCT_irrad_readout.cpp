// SCT_irrad_readout.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\..\SCT_analysis\SCT_analysis\SCT_read.h"
#include "C:\root_v5.34.36\include\TCanvas.h"
#include "C:\root_v5.34.36\include\TLegend.h"

int main()
{
	std::string input;
	std::cout << "input file name for file of files and path: ";
	std::cin >> input;
	std::cout << std::endl;
	std::ifstream file_names (input.c_str());
	std::string readin;
	std::vector<std::vector<TH1F*>> all_hists;
	std::vector <TH1F*> htest(12);
	std::vector<std::string>cname;
	for (int i = 0; i < 12; i++) {
		htest.at(i) = new TH1F("", "", 200, 0, 30000);
		for (int j = 0; j < 200; j++) htest.at(i)->SetBinContent(j, 0);
		if(i ==1) htest.at(i)->GetYaxis()->SetRangeUser(0, 200);
		else if (i == 3) htest.at(i)->GetYaxis()->SetRangeUser(0, 70);
		else if (i == 9) htest.at(i)->GetYaxis()->SetRangeUser(0, 30);
		else htest.at(i)->GetYaxis()->SetRangeUser(0, 2);
	}
	all_hists.push_back(htest);
	while (file_names.is_open() && !file_names.eof()) {
		while (getline(file_names, readin)) {
			std::istringstream subin(readin), split_tcode, cda;
			std::string temp, file_name, time_code, temp_time_code, temptations,cda2;
			bool chip = 0;
			int chip_n;
			while (getline(subin, temp, '_')) {
				std::cout << temp << std::endl;
				if (temp.find(".dat") != std::string::npos) file_name = readin;
				if (chip == 1) {
					cda.str(temp);
					while (getline(cda, cda2, '.')) {
						if (cda2.find("dat") == std::string::npos) {
							chip_n = std::stoi(temp);
							chip = 0;
						}
					}
				}
				if (temp.find("chip") != std::string::npos) chip = 1;
				if (temp.find("2018") != std::string::npos) temp_time_code = temp;
			}
			split_tcode.str(temp_time_code);
			while (getline(split_tcode, temptations, '_')) time_code = temptations + " ";
			std::fstream log(file_name.c_str());
			TFile* chip_file = new TFile(("Chip_" + std::to_string(chip_n)+"_raw.root").c_str(), "RECREATE");
			SCT_read readout;
			all_hists.push_back(readout.scan_irrad_dat_file(&log, chip_file, chip_n));
			//else SCT_read::scan_dat_file(&log, chip_file, chip_n, time_code, aon, lastof, rn);
			cname.push_back(std::to_string(chip_n));
		}
	}
	std::vector<TCanvas*> canvi(all_hists.at(0).size());
	std::vector<TLegend*> leg(all_hists.at(0).size());
	for (int i = 0; i < canvi.size(); i++) {
		canvi.at(i) = new TCanvas();
		leg.at(i) = new TLegend(0.7, 0.7, 0.9, 0.9);
	}
	for (int i = 0; i < all_hists.size(); i++) {
		for (int j = 0; j < all_hists.at(i).size(); j++) {
			canvi.at(j)->cd();
			std::string title = all_hists.at(1).at(j)->GetTitle(), xtitle= all_hists.at(1).at(j)->GetXaxis()->GetTitle(), ytitle= (all_hists.at(1).at(j)->GetXaxis()->GetTitle());
			all_hists.at(0).at(j)->SetTitle(title.c_str());
			all_hists.at(0).at(j)->GetXaxis()->SetTitle(xtitle.c_str());
			all_hists.at(0).at(j)->GetYaxis()->SetTitle(ytitle.c_str());
			all_hists.at(i).at(j)->SetStats(0);
			all_hists.at(i).at(j)->SetLineColor(i + 1);
			all_hists.at(i).at(j)->Draw("same ][");
			if (i > 0) {
				leg.at(j)->AddEntry(all_hists.at(i).at(j), cname.at(i-1).c_str());
				leg.at(j)->Draw();
			}
		}
	}
	canvi.at(0)->Print("All_together_now_raw.pdf[");
	for (int i = 0; i < canvi.size(); i++)canvi.at(i)->Print("All_together_now_raw.pdf");
	canvi.back()->Print("All_together_now_raw.pdf]");
	return 0;
}

