#include "Data.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

void check(bool ok,const std::string& message) { if(!ok) throw std::runtime_error(message); }
template<class F> void rejects(F fn,const std::string& message) {
  bool rejected=false; try { fn(); } catch(const std::exception&) { rejected=true; }
  check(rejected,message);
}
int main(int argc,char** argv) {
  std::filesystem::path temporary;
  try {
    check(argc==2,"Expected dataset root");
    check(vast::split("a,\"b,c\",\"d\"\"e\",",',')==std::vector<std::string>({"a","b,c","d\"e",""}),"CSV quoting/trailing field");
    check(vast::split("1\t@irvin\r",'\t')[1]=="@irvin","CRLF trimming");
    rejects([]{vast::split("\"unclosed",',');},"Malformed CSV rejected");
    rejects([]{vast::timestamp("2008-02-01T12:00");},"Reject unsupported month");
    rejects([]{vast::timestamp("2008-01-32T12:00");},"Reject invalid day");
    rejects([]{vast::timestamp("2008-01-01T24:00");},"Reject invalid hour");
    rejects([]{vast::timestamp("2008-01-01T12:00:60");},"Reject invalid seconds");
    vast::Hours hours;
    check(hours.after(0) && hours.after(6) && !hours.after(7) && !hours.after(19) && hours.after(20) && hours.after(23),"Shared hours boundaries");
    auto t=vast::timestamp("2008-01-31T19:59:30.000");
    check(t.day==31 && t.hour==19 && t.timeOfDay>19.99 && t.timeOfDay<20,"Timestamp precision");
    const auto d=vast::load(argv[1]);
    check(d.entities.size()==6016 && d.rankedPeople().size()==6000,"Entity and person counts");
    check(d.socialLinks.size()==29876 && d.otherLinks==12 && d.duplicateLinks==0,"Social filtering");
    check(d.badges.size()==10162 && d.network.size()==115414 && d.employees.size()==60,"M1 row counts");
    check(d.unmatchedRequests==0,"Source IP join complete");
    check(d.entities.at(1).name=="@irvin","Headerless first Flitter row preserved");
    check(d.rankedPeople().front()==5 && d.entities.at(5).inDegree==445,"Centrality regression");
    check(d.rankedEmployees().front()==12 && d.employees.at(12).afterClassified==11,"Classified entry ranking");
    std::uint64_t requests=0,badges=0,in=0,out=0;
    for(const auto& [id,e]:d.employees) { requests+=e.requests; badges+=e.badges; check(e.afterRequests<=e.requests,"Subset count"); }
    for(int id:d.rankedPeople()) { in+=d.entities.at(id).inDegree; out+=d.entities.at(id).outDegree; }
    check(requests==d.network.size() && badges==d.badges.size(),"Join preserves event counts");
    check(in==d.socialLinks.size() && out==in,"Directed degree conservation");
    int cityTotal=0; for(const auto& [name,n]:d.cities()) cityTotal+=n;
    check(cityTotal==6000,"City population conservation");
    const auto allNormal=vast::load(argv[1],{0,24});
    for(const auto& [id,e]:allNormal.employees) check(e.afterBadges==0 && e.afterRequests==0,"Configurable policy applies to both datasets");
    temporary=std::filesystem::temp_directory_path()/("vast-tests-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    vast::exportReports(d,temporary);
    for(const auto* name:{"employee_summary.csv","social_degrees.csv","city_population.csv","after_hours_badges.csv","after_hours_destinations.csv","findings.md"})
      check(std::filesystem::file_size(temporary/name)>0,std::string("Export: ")+name);
    rejects([&]{vast::load(temporary);},"Missing data produces error");
    std::filesystem::remove_all(temporary);
    std::cout << "Passed parser, time-boundary, joins, graph, policy, export and dataset regression checks.\n";
    return 0;
  } catch(const std::exception& e) {
    if(!temporary.empty()) std::filesystem::remove_all(temporary);
    std::cerr << "FAIL: " << e.what() << '\n'; return 1;
  }
}
