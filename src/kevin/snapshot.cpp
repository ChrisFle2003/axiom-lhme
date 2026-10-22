#include "axiom/lhme/kevin_snapshot.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace {
constexpr char kMagic[] = "AXIOM-KEVIN-1\n";
std::uint64_t checksum(std::string_view bytes) { std::uint64_t hash=14695981039346656037ULL; for (const auto byte:bytes) { hash^=static_cast<unsigned char>(byte); hash*=1099511628211ULL; } return hash; }

bool write_string(std::ostream& out, const std::string& value) {
  const auto size = value.size();
  out << size << ':' << value << '\n';
  return static_cast<bool>(out);
}

bool read_string(std::istream& in, std::string& value) {
  std::size_t size = 0; char colon = 0;
  if (!(in >> size) || !(in >> colon) || colon != ':' || size > 4096) return false;
  value.resize(size); in.read(value.data(), static_cast<std::streamsize>(size));
  char newline = 0; in.get(newline); return static_cast<bool>(in) && newline == '\n';
}
}

namespace axiom::lhme {

SnapshotStatus save_snapshot(const KevinMemoryStore& store, const std::filesystem::path& path) {
  const auto temporary = path.string() + ".tmp";
  std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
  if (!out) return SnapshotStatus::IoError;
  const auto snapshot = store.snapshot();
  out << kMagic << snapshot.grids.size() << ' ' << snapshot.overrides.size() << '\n';
  for (const auto& grid : snapshot.grids) {
    if (!write_string(out, grid.identity.id) || !write_string(out, grid.technique.id) ||
        !write_string(out, grid.canonical_parameters)) return SnapshotStatus::IoError;
    out << grid.identity.version << ' ' << grid.technique.version << ' '
        << static_cast<unsigned>(grid.max_depth) << '\n';
  }
  for (const auto& item : snapshot.overrides) {
    if (!write_string(out, item.identity.id) || !write_string(out, item.value.convert_to<std::string>())) return SnapshotStatus::IoError;
    out << item.identity.version << ' ' << item.address.depth();
    for (const auto step : item.address.steps()) out << ' ' << unsigned(step.x) << ' ' << unsigned(step.y);
    out << '\n';
  }
  out.close(); if (!out) return SnapshotStatus::IoError;
  std::ifstream payload_input(temporary, std::ios::binary);
  const std::string payload{std::istreambuf_iterator<char>(payload_input), {}};
  if (!payload_input.good() && !payload_input.eof()) return SnapshotStatus::IoError;
  std::ofstream checksum_output(temporary, std::ios::binary | std::ios::app);
  checksum_output << "CHECKSUM " << checksum(payload) << '\n';
  checksum_output.close(); if (!checksum_output) return SnapshotStatus::IoError;
  std::error_code error; std::filesystem::rename(temporary, path, error);
  if (error) { std::filesystem::remove(temporary); return SnapshotStatus::IoError; }
  return SnapshotStatus::Confirmed;
}

SnapshotStatus load_snapshot(KevinMemoryStore& store, const std::filesystem::path& path) {
  std::ifstream source(path, std::ios::binary); if (!source) return SnapshotStatus::IoError;
  const std::string file{std::istreambuf_iterator<char>(source), {}};
  const auto marker=file.rfind("CHECKSUM ");
  if (marker==std::string::npos || marker==0 || file[marker-1] != '\n') return SnapshotStatus::InvalidSnapshot;
  std::uint64_t stored=0; std::istringstream checksum_input(file.substr(marker+9));
  if (!(checksum_input>>stored) || checksum(file.substr(0,marker)) != stored) return SnapshotStatus::InvalidSnapshot;
  std::istringstream in(file.substr(0,marker));
  std::string magic(sizeof(kMagic) - 1, '\0'); in.read(magic.data(), magic.size());
  if (magic != kMagic) return SnapshotStatus::InvalidSnapshot;
  std::size_t grids = 0, overrides = 0;
  if (!(in >> grids >> overrides) || grids > 100000 || overrides > 100000) return SnapshotStatus::InvalidSnapshot;
  in.ignore(1); KevinSnapshot snapshot;
  for (std::size_t i=0;i<grids;++i) {
    GridSpec grid; unsigned depth=0;
    if (!read_string(in,grid.identity.id) || !read_string(in,grid.technique.id) || !read_string(in,grid.canonical_parameters) ||
        !(in>>grid.identity.version>>grid.technique.version>>depth) || depth>10) return SnapshotStatus::InvalidSnapshot;
    grid.max_depth=static_cast<std::uint8_t>(depth); in.ignore(1); snapshot.grids.push_back(std::move(grid));
  }
  for (std::size_t i=0;i<overrides;++i) {
    GridIdentity identity; std::string number; std::size_t depth=0;
    if (!read_string(in,identity.id) || !read_string(in,number) || !(in>>identity.version>>depth) || depth>10) return SnapshotStatus::InvalidSnapshot;
    std::vector<GridStep> steps; for(std::size_t j=0;j<depth;++j){unsigned x,y; if(!(in>>x>>y)||x>7||y>7)return SnapshotStatus::InvalidSnapshot; steps.push_back({static_cast<std::uint8_t>(x),static_cast<std::uint8_t>(y)});}
    in.ignore(1); if (number.empty() || (number.size()>1 && number[0]=='0') || (number.size()>2 && number[0]=='-' && number[1]=='0')) return SnapshotStatus::InvalidSnapshot;
    BigInt value; try { value=BigInt{number}; } catch (...) { return SnapshotStatus::InvalidSnapshot; }
    const auto address=GridAddress::from_steps(steps); if(!address) return SnapshotStatus::InvalidSnapshot;
    snapshot.overrides.push_back({std::move(identity), *address, std::move(value)});
  }
  if (in.peek()!=std::char_traits<char>::eof()) return SnapshotStatus::InvalidSnapshot;
  return store.replace_snapshot(snapshot);
}

}  // namespace axiom::lhme
