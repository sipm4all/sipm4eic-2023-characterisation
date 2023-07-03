#include "make_iv_scan.C"
#include "ureadout_dcr_get.C"

namespace database {

std::string fields[8] = {
  "run", "quality", "step", "setup", "iv-mux-1", "iv-mux-2", "dcr-chip-2", "dcr-chip-3" 
};

std::vector<std::string> channels = {
  "A1", "A2", "A3", "A4",
  "B1", "B2", "B3", "B4",
  "C1", "C2", "C3", "C4"
}; 

std::string basedir = ".";
std::map<std::string, std::map<std::string, std::map<std::string, std::string>>> boards;

void read_database(std::string fname);
void dump_filenames(std::string fname);
std::map<std::string, std::string> get_filename(std::string board, std::string channel, std::string step);
TGraphErrors *get_dcr_vbias_scan(std::string board, std::string channel, std::string step);
TGraphErrors *get_iv_scan(std::string board, std::string channel, std::string step);

void
dump_filenames(std::string fname)
{
  ofstream fout(fname);
  for (auto &val1 : boards) {
    auto board = val1.first;
    for (auto &val2 : val1.second) {
      auto step = val2.first;
      for (auto &channel : channels) {
        auto fnames = get_filename(board, channel, step);
        for (auto fname : fnames)
          fout << fname.second << std::endl;
      }
    }
  }
  fout.close();
}

TGraphErrors *
get_dcr_vbias_scan(std::string board, std::string channel, std::string step)
{
  auto fname = get_filename(board, channel, step);
  if (fname["dcr-vbias"].empty()) return nullptr;
  return ureadout_dcr_get(fname["dcr-vbias"], "bias_voltage", "dead_rate");
}

TGraphErrors *
get_iv_scan(std::string board, std::string channel, std::string step)
{
  auto fname = get_filename(board, channel, step);
  if (fname["iv"].empty() || fname["iv-open"].empty()) return nullptr;
  return make_iv_scan(fname["iv"], fname["iv-open"]);
}


std::map<std::string, std::string>
get_filename(std::string board, std::string channel, std::string step)
{
  if (!boards.count(board)) return {};
  if (!boards[board].count(step)) return {};
  
  auto ivrun = boards[board][step]["iv-run"];
  auto ivsetup = boards[board][step]["iv-setup"];
  auto ivmux = boards[board][step]["iv-mux"];
  std::string ivfname = basedir + "/" + ivrun + "/" + ivsetup + "/iv/HAMA3_sn0_mux" + ivmux + "/HAMA3_sn0_243K_" + channel + ".ivscan.csv";
  std::string ivofname = basedir + "/" + ivrun + "/" + ivsetup + "/iv/HAMA3_sn0_mux" + ivmux + "/HAMA3_sn0_243K_OPEN-" + channel + ".ivscan.csv";

  auto dcrrun = boards[board][step]["dcr-run"];
  auto dcrsetup = boards[board][step]["dcr-setup"];
  auto dcrchip = boards[board][step]["dcr-chip"];

  std::string dcrthrfname = basedir + "/" + dcrrun + "/" + dcrsetup + "/dcr/HAMA3-chip" + dcrchip + "/rate/threshold_scan/chip" + dcrchip + "-" + channel + ".ureadout_dcr_scan.tree.root";
  std::string dcrbiasfname = basedir + "/" + dcrrun + "/" + dcrsetup + "/dcr/HAMA3-chip" + dcrchip + "/rate/vbias_scan/chip" + dcrchip + "-" + channel + ".ureadout_dcr_scan.tree.root";

  return { {"iv", ivfname} , {"iv-open", ivofname} , {"dcr-threshold", dcrthrfname} , {"dcr-vbias", dcrbiasfname} };
}

void
read_database(std::string fname)
{
  std::ifstream ifs(fname);
  std::string line;
  std::string board;
  while (std::getline(ifs, line)) {
    if (line[0] == '#' || line[0] == ' ') continue;
    std::stringstream ss(line);
    std::map<std::string, std::string> linedata;
    std::string data;
    for (int i = 0; i < 8; ++i) {
      ss >> data;
      linedata[fields[i]] = data;
    }
    
    auto run = linedata["run"];
    auto step = linedata["step"];
    auto setup = linedata["setup"]; 

    board = linedata["iv-mux-1"];
    boards[board][step]["iv-run"] = run;
    boards[board][step]["iv-setup"] = setup;
    boards[board][step]["iv-mux"] = "1";

    board = linedata["iv-mux-2"];
    boards[board][step]["iv-run"] = run;
    boards[board][step]["iv-setup"] = setup;
    boards[board][step]["iv-mux"] = "2";

    board = linedata["dcr-chip-2"];
    boards[board][step]["dcr-run"] = run;
    boards[board][step]["dcr-setup"] = setup;
    boards[board][step]["dcr-chip"] = "2";

    board = linedata["dcr-chip-3"];
    boards[board][step]["dcr-run"] = run;
    boards[board][step]["dcr-setup"] = setup;
    boards[board][step]["dcr-chip"] = "3";
    
  }

}

}