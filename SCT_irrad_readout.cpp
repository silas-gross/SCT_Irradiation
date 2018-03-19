// SCT_irrad_readout.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SCT_read.h"

int main()
{
	std::ifstream file_names ("log_testing_files.txt");
	std::string readin;
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
			TFile* chip_file = new TFile(("Chip_" + std::to_string(chip_n)+".root").c_str(), "RECREATE");
			SCT_read readout;
			readout.scan_irrad_dat_file(&log, chip_file, chip_n);
			//else SCT_read::scan_dat_file(&log, chip_file, chip_n, time_code, aon, lastof, rn);
		}
	}
	return 0;
}

