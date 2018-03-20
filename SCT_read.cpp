#include "stdafx.h"
#include "SCT_read.h"
#include "C:\root_v5.34.36\include\TVector.h"
std::vector<std::vector<float>> SCT_read::time_averging(std::vector<std::vector<float>> data)
{
	std::vector<float> ta_vd, ta_va, ta_ad, ta_aa, ta_vdreg, ta_vareg;
	float vd = 0, va = 0, ad = 0, aa = 0, n = 0, vdreg = 0, vareg = 0;
	//	std::cout << "Input vector has size " << data.size() << " with " << data.at(0).size() << " columns" << std::endl;
	for (int i = 0; i < data.size(); i++)
	{
		if (data.at(i).size() < 5)continue;
		vd += data.at(i).at(1);

		ad += data.at(i).at(2);
		try {
			va += data.at(i).at(3);
			aa += data.at(i).at(4);
			vdreg += data.at(i).at(8);
			vareg += data.at(i).at(6);
			n++;
		}
		catch (std::exception& e) {};
		if (i % 6 == true) {
			if (n != 0) {
				ta_vd.push_back(vd / n);
				ta_vareg.push_back(vareg / n);
				ta_ad.push_back(ad / n);
				ta_vdreg.push_back(vdreg / n);
				ta_va.push_back(va / n);
				ta_aa.push_back(aa / n);

			}
			n = 0;
			vd = 0;
			va = 0;
			aa = 0;
			ad = 0;
			vdreg = 0;
			vareg = 0;
		}
	}
	std::vector <std::vector<float>> avgs(6);
	avgs.at(0) = ta_vd;
	avgs.at(1) = ta_ad;
	avgs.at(2) = ta_va;
	avgs.at(3) = ta_aa;
	avgs.at(4) = ta_vdreg;
	avgs.at(5) = ta_vareg;
	//	std::cout << "Readout vectors have size " << ta_vd.size() << std::endl;
	return avgs;
}
std::vector<TH1F*> SCT_read::TempLoss(std::vector<std::vector<float>> data, std::string board_label, const long init_time)
{
	TH1F* Temp = new TH1F("temp", "temp", 100, 0, 20);
	TH1F* Tloss = new TH1F((board_label + "_tloss").c_str(), ("Expected Temperature for chip " + board_label + " from initial; Temperature (K)").c_str(), 100, 0, 25);
	TH1F* TGain = new TH1F((board_label + "_act_tloss").c_str(), ("Measured Temperature for chip " + board_label + "from initial; Teperature(K)").c_str(), 100, 0, 25);
	TH1F* TDiff = new TH1F((board_label + "_diffs_of_loss").c_str(), ("Diffences of Temperature Increase approach for chip " + board_label + ";  Teperature diffrence (K)").c_str(), 25, -1, 1);
	float tinit = data.at(1).at(10);
	const float Ts = 20, hs = 10.88, A = 0.0075;
	for (int i = 0; i < data.size(); i++)
	{
		//if (data.at(i).at(10) == 0) data.at(i).at(10) = 100;
		if (data.at(i).size() == 0)continue;
		if (data.at(i).at(10) == 0)continue;
		float tdiff = data.at(i).at(10);
		float tmdiff = (data.at(i).at(1)*data.at(i).at(2) + data.at(i).at(3)*data.at(i).at(4)) / (A*hs) + Ts;
		long time_spot = data.at(i).at(0) - init_time;
		//std::cout << "Temp diff measured " << tdiff << " K" << " versus what it actually is" <<data.at(i).at(10)<<std::endl;
		time_spot = time_spot / 10;
		float diffs = tdiff - tmdiff;
		Temp->Fill(data.at(i).at(10));
		Tloss->Fill(tmdiff);
		TGain->Fill(tdiff);
		TDiff->Fill(diffs);
	}
	std::vector<TH1F*> Temps(4);
	Temps.at(0) = Tloss;
	Temps.at(1) = TGain;
	Temps.at(2) = TDiff;
	Temps.at(3) = Temp;
	Tloss->Delete();
	TGain->Delete();
	//tinit->clear;
	return Temps;
}
int SCT_read::scan_dat_file(fstream* inf, int boardnumber, std::string time_code, bool aon, bool lastof, int rn, int lastline, std::vector<TH1F*> ttemps, std::vector<TH1F*> avg_hists)
{
	std::string readout = "";
	int wc = 0, line = 0, tline = 0;
	bool hasread = false;
	std::vector <std::string> columns;
	std::vector <TH1F*> hists;
	std::vector <std::vector <float> > vals;
	std::vector <float> lvec;
	//fstream err;
	//err.open("error_file.txt");
	//vals.resize(11);
	//std::vector <float> vd, t, id, va, ia, vdraw, vdreg, varaw, vareg, mgnd, T;
	bool esccondition = inf->is_open();
	//std::cout << "Ready to begin looping, Start Signal is " <<time_code << std::endl;
	while (esccondition) {
		while (std::getline(*inf, readout))
		{
			tline++;
			if (tline < lastline)continue;
			//std::cout << tline << std::endl;
			//	if (readout.find(time_code.c_str()) == std::string::npos &&wc==0 &&hasread==0) std::cout << '\r' << "Looking for value at line " << tline << std::flush;
			if (readout.find(time_code.c_str()) != std::string::npos) {
				wc = 1;
				hasread = true;
				//std::cout << "\n Start signal " << time_code << " sent" << "\n Line is " <<tline <<"\n Line reads " <<readout << std::endl;
				line = tline;
			}
			if (readout.find("Start") != std::string::npos && readout.find(time_code.c_str()) == std::string::npos) wc = 0;
			//while (wc == 0) std::cout << "Still looking for start signal" << std::endl;
			//if (hasread && !wc) esccondition = 0;
			if (wc == 1)//&& readout.compare(time_code)==0 && readout.compare( "#** t(s),v0Mon(V),i0Mon(mA),v1Mon(V),i1Mon(mA),VDDA_RAW(mV),VDDA_REG(mV),VDDD_RAW(mV),VDDD_REG(mV),MOD_GND(mV),Temperature(C),IrradStatus(On = 1),DoseRate(kRad/hr),TotalDose(MRad) **")==0 && readout.compare("#** -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- **")==0)
			{

				line++;
				columns.clear();
				std::istringstream temp(readout);
				std::string temp2;
				while (std::getline(temp, temp2, ' ')) columns.push_back(temp2);
				//std::vector <float>lvec;
				for (int i = 0; i < columns.size(); i++) {
					//std::cout << columns.at(i) << std::endl;
					try {
						lvec.push_back(std::stof(columns.at(i).c_str()));
					}
					catch (const std::exception& e) {
						continue;

					}
					//	std::cout << "found " <<lvec.size() <<" columns" <<std::endl; 
				}
				if (lvec.size() <14 || columns.size() == 0) {
					//	std::cout << "vectors unfilled on line " << line <<"\n line reads " <<readout << std::endl;
					continue;
				}
				vals.push_back(lvec);
				lvec.clear();
				if (boardnumber == 84 && vals.size() >= 44) {
					wc = 0;
					continue;
				}
				//if (boardnumber == 84) std::cout << "\r Reading code is " << wc << std::flush;

			}
			//			if(wc==0) continue;

			//std::cout << vals.at(2).size() << std::endl; //having this line in causes the columns request to fail....why???????


			if (wc == 0 && hasread == true || inf->eof())esccondition = false;
		}
	}
	//std::cout << " \n Having escaped the carven of the terrible while loop, our hero travels forth" << std::endl;
	/*	try {
	hists.push_back(new TH1F(("vd" + std::to_string(rn)).c_str(), "Voltage in Digital Channel; Time (s); Voltage (V)", vals.size(), 0, 10 * vals.size()));
	//std::cout << vals.at(0).size() << std::endl;

	for (int i = 0; i < vals.size(); i++) {
	try {
	vals.at(i).at(0) = vals.at(i).at(0) - vals.at(0).at(0);
	//std::cout << " \r Time of envent is at " << vals.at(i).at(0) << "seconds from start" << std::flush;
	hists.at(0)->SetBinContent(i, vals.at(i).at(1));
	//std::cout <<"There are "<< vals.at(i).size() <<" variables at play" << std::endl;
	}

	catch (std::exception& e) { //std::cout << " \n Error in element " <<i  <<"of type " <<e.what()<< std::endl;
	continue;
	}
	}
	}
	catch (std::exception& e) {};
	//	if (boardnumber == 48) std::cout << "Sample Current " << vals.at(10).at(2) << std::endl;
	try {
	hists.push_back(new TH1F(("id" + std::to_string(rn) + std::to_string(boardnumber)).c_str(), "Current in Digital Channel; Time (s); Current (mA)", vals.size(), 0, 10 * vals.size()));
	for (int i = 0; i < vals.size(); i++) {
	try { hists.at(1)->SetBinContent(i, 1000 * vals.at(i).at(2)); }
	catch (...) {
	continue;
	}
	}
	}
	catch (std::exception& e) { std::cout << e.what() << std::endl; }
	hists.push_back(new TH1F(("pd" + std::to_string(rn) + std::to_string(boardnumber)).c_str(), "Power in Digital Channel; Time (s); Power(mW)", vals.size(), 0, 10 * vals.size()));
	for (int i = 0; i < vals.size(); i++) {
	try {
	//vals.at(i).at(0) = vals.at(i).at(0) - vals.at(0).at(0);
	hists.at(2)->SetBinContent(i, vals.at(i).at(1)*vals.at(i).at(2) * 1000);
	}
	catch (std::exception& e) { continue; }
	}*/
	/*if (aon = true) {
	//hists.push_back(new TH1F(("va"+std::to_string(rn) + std::to_string(boardnumber)).c_str(), "Voltage in Analog Channel; Time (s); Voltage (V)", vals.size(), 0, 10 * vals.size()));
	//std::cout << vals.at(0).size() << std::endl;

	for (int i = 0; i < vals.size(); i++) {
	try {
	vals.at(i).at(0) = vals.at(i).at(0) - vals.at(0).at(0);
	//std::cout << " \r Time of envent is at " << vals.at(i).at(0) << "seconds from start" << std::flush;
	hists.at(3)->SetBinContent(i, vals.at(i).at(3));
	//std::cout << vals.at(0).at(i) << std::endl;
	}

	catch (std::exception& e) {
	//std::cout << " \n Error in element " << i << "of type " << e.what() << std::endl;
	continue;
	}
	}
	hists.push_back(new TH1F(("ia"+std::to_string(rn) + std::to_string(boardnumber)).c_str(), "Current in Analog Channel; Time (s); Current (mA)", vals.size(), 0, 10 * vals.size()));
	for (int i = 0; i < vals.size() - 1; i++) {
	try { hists.at(4)->SetBinContent(i, 1000 * vals.at(i).at(4)); }
	catch (...) {
	continue;
	}
	}
	hists.push_back(new TH1F(("pa"+std::to_string(rn)+std::to_string(boardnumber)).c_str(), "Power in Analog Channel; Time (s); Power(mW)", vals.size(), 0, 10 * vals.size()));
	for (int i = 0; i < vals.size(); i++) {
	try {
	//vals.at(i).at(0) = vals.at(i).at(0) - vals.at(0).at(0);
	hists.at(5)->SetBinContent(i, vals.at(i).at(3)*vals.at(i).at(4) * 1000);
	}
	catch (std::exception& e) { continue; }
	}
	}
	*/
	int index = vals.size();
	TVectorF T(index), C(index), V(index);
	int j = 0;
	for (int i = 0; i < vals.size(); i++) {
		try {
			if (vals.at(i).at(10) <= 0) { //checks for faulty Temperatures and discards them
				continue;
			}
			T[j] = vals.at(i).at(10); //use of j to avoid holes in the vector while parsing
			C[j] = vals.at(i).at(2);
			V[j] = vals.at(i).at(1);
			j++;

		}
		catch (std::exception& e) {
			std::cout << 'r' << e.what() << std::flush;
			continue;  //should not throw an exception, but getting out of range errors sill on vals
		}
	}
	//	tempgraphs.push_back(new TGraph(C, T)); //creates current graphs
	//	std::cout << "\n Temp is: " << T[index / 2] << " C on the chip" << std::endl;
	//	tempgraphs.push_back(new TGraph(V, T));
	//for (int i = 0; i < hists.size(); i++) hists.at(i)->Write();
	//***NOTE: PD & PA have older variable names from attempting to plot Powers instead of currents, Too lazy to change ***
	//	PAall.push_back(hists.at(4)); //analog current
	//	PDall.push_back(hists.at(1)); //Digital current
	//std::cout << "\n" <<tempgraphs.size() <<" Temperature Graphs recorded" << std::endl; //sanity check
	//	f->Write();
	//f->Close();
	//if (vals.at(1).size() == 14) {
	std::vector<std::vector<float>> av = time_averging(vals);

	try {

		for (int i = 0; i < av.size(); i++) {
			for (int j = 0; j < av.at(i).size(); j++) avg_hists.at(i)->Fill(av.at(i).at(j));
		}
	}
	catch (std::exception& e) { std::cout << "Unable to perform time averaging on run " << rn << "\n Returned error " << e.what() << std::endl; }
	av.~vector();

	try {
		std::vector<TH1F*> temps = TempLoss(vals, std::to_string(boardnumber), vals.at(0).at(0));


		for (int k = 0; k < temps.size(); k++) {
			try {
				ttemps.at(k)->Add(temps.at(k));
				if (rn == 0)ttemps.at(k)->SetNameTitle(temps.at(k)->GetName(), temps.at(k)->GetTitle());
			}

			catch (std::exception& e) { std::cout << e.what() << std::endl; };
		}

		temps.~vector();
	}
	catch (std::exception& e) { std::cout << "Unable to perform Temp Analysis on run " << rn << "\n Returned error " << e.what() << std::endl; }
	//}
	T.Delete();
	C.Delete();
	V.Delete();
	columns.~vector();
	vals.~vector();
	//hists.~vector();
	//av.~vector();
	//if (lastof) {
	//for (int i = 0; i < avg_hists.size(); i++) avg_hists.at(i)->Write(0,1,0);
	//}
	//	avg_hists.~vector();
	//get rid of memory leak possiblity
	return line;
}

void SCT_read::scan_irrad_dat_file(std::fstream* inf, TFile* f, int boardnumber) {
	std::string line;
	std::vector<std::string> header;
	std::vector <std::vector <float>> data;
	while (inf->is_open()) {
		int i = 0;
		std::cout << boardnumber << std::endl;
		while (getline(*inf, line)) {
			i++;
			if (line == "") continue;
			std::stringstream breakline(line);
			std::string subline;
			std::vector<float> linedata;
			while (getline(breakline, subline, ' ') ){
				if (i == 1) {
					std::stringstream hline(subline);
					std::string label;
					while (getline(hline, label, ',')) {
						if (label.find("t") != std::string::npos || label.find("(") != std::string::npos)header.push_back(label);
					}
				}
				else {
					try {
						linedata.push_back(std::stof(subline));
					}
					catch (std::invalid_argument& a) { std::cout << "Invalid argument passed to stof. \n Value passed is " << subline << std::endl; }
				}
			}
			data.push_back(linedata);
			linedata.~vector();
			
		}
		if (inf->eof())inf->close();
	}
	std::vector<TH1F*> hists(header.size()-1);
	std::vector<std::string> titles(header.size() - 1);
	titles.at(0) = "Voltage in Digital Channel; TID (kRad);  Voltage (V)";
	titles.at(1) = "Current in Digital Channel; TID (kRad); Current (mA)";
	titles.at(2) = "Voltage in Analog Channel; TID (kRad);  Voltage (V)";
	titles.at(3) = "Current in Analog Channel; TID (kRad); Current (mA)";
	titles.at(4) = "Raw Voltage Supplied to Analog Channel; TID (kRad); Voltage (V)";
	titles.at(5) = "Regulated Voltage of Analog Channel; TID (kRad); Voltage (V)";
	titles.at(6) = "Raw Voltage Supplied to Digital Channel; TID (kRad); Voltage (V)";
	titles.at(7) = "Regulated Voltage of Digital Channel; TID (kRad); Voltage (V)";
	titles.at(8) = "Grounding Voltage; TID (kRad); Voltage (V)";
	titles.at(9) = "Temperature; TID (kRad); Temerature (C)";
	titles.at(10) = "";
	titles.at(11) = "Dose rate; Time (10 s); Dose Rate (kRad/hr)";
	titles.at(12) = "TID; Time (10 s); TID (MRad)";
	for (int i = 0; i < hists.size(); i++) {
		if (titles.at(i) != "")hists.at(i) = new TH1F((header.at(i + 1) + std::to_string(boardnumber)).c_str(), titles.at(i).c_str(), 100, 0, 3000);
		else hists.at(i) = new TH1F();
	}
	for (int i = 0; i < hists.size(); i++)
	{
		if (header.at(i+1).find("A") != std::string::npos) {
			for (int k = 0; k < data.size(); k++) {
				std::cout << "fixing Amp problems on line " << k << std::endl;
				std::cout << "This line has size " << data.at(k).size() << std::endl;
				if (i < data.at(k).size()-1 && k<data.size() && data.at(k).size()>hists.size()) data.at(k).at(i+1) = data.at(k).at(i+1)*1000;
				std::cout << i << std::endl;
			}
		}
		else if (header.at(i+1).find("m") != std::string::npos) {
			std::cout << "second check" << std::endl;
			for (int j = 0; j < data.size(); j++) {
				if (i < data.at(j).size()) data.at(j).at(i + 1) = 0.001;
			}
		}
		else std::cout << i << std::endl; 
		if (titles.at(i).find("Time") != std::string::npos) {
			std::cout << "within  a filling approach" << std::endl;
			for (int j = 0; j < data.size(); j++) {
				if (i < data.at(j).size()) hists.at(i)->SetBinContent(j, data.at(j).at(i + 1));
			}
		}
		else {
			for (int j = 0; j < data.size(); j++) {
				float fixed = 0;
				int count = 0;
				bool av = 0;
				//std::cout << "Am I within here?" << std::endl;
				try {
				//	std::cout << j << std::endl;
					if (j >= 1 && data.at(j).at(13) == data.at(j - 1).at(13)) {
						fixed += data.at(j).at(i + 1); //averages over the variables at the same TID 
						count++;
						av = 1;
					}


					else if (av == 1) {
						fixed = fixed / count;
						hists.at(i)->SetBinContent(data.at(j - 1).at(13) / 1000, fixed);
						av = 0;
					}
					else hists.at(i)->SetBinContent(data.at(j).back() / 1000, data.at(j).at(i + 1));
				}
				catch (std::exception& e) { std::cout << j << std::endl; }
			}
			hists.at(i)->Write();
		}
	}
	std::vector<TH1F*>temps=TempLoss(data, std::to_string(boardnumber), data.at(1).at(0));
	std::vector<std::vector<float>> avg = time_averging(data);
	std::vector<TH1F*> further_hists(temps.size() + avg.size());
	for (int i = 0; i < temps.size(); i++) further_hists.at(i) = (TH1F*)temps.at(i)->Clone();
	for (int i = 0; i < avg.size(); i++) {
		further_hists.at(i + temps.size()) = new TH1F(("avg_" + std::to_string(i)).c_str(), ("avg_" + std::to_string(i)).c_str(), 100, 0, 2000);
		for (int j = 0; j < avg.at(0).size(); j++) further_hists.at(i + temps.size())->Fill(avg.at(i).at(j));
	}
	for (int i = 0; i < further_hists.size(); i++) further_hists.at(i)->Write();
	f->Close();

}

