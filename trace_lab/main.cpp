#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <sstream>
#include <stdio.h>

using namespace std;

class InstructionSet {
public:
        InstructionSet(string input){
                ifstream f;
                f.open(input.c_str());
		string buf;
		while(f >> buf){
			sr.push_back(stoi(buf));
		}
		f.close();
        }

        int length(){
		return sr.size();
	}

	// Instruction count
	int ic(){
		return sr.size()/3;
	}

	vector<int> sr;
};

// Just looking for RaW
// xyz
// aaa <- up to rs spaces of separation
// yxz = data dependency hazard
// No compiling required

int main (int argc, char *argv[]) {

	if(argv[1]==NULL || argv[2]==NULL || argv[3]==NULL || argv[4]==NULL || argv[5]!=NULL){
		cout << "Uses three args: File in, file out, MIPS pipeline stages, cycles per read stage" <<endl;
		return 1;
	}

	string fname = argv[1];
	string outname = argv[2];
	string mips_len = argv[3];
	string read_stages = argv[4];

	try{
		InstructionSet trace(fname);
	}
	catch(const std::exception &e){
		cout<<"Bad input file"<<endl;
		return 1;
	}
	try{
		ofstream outfile;
		outfile.open(outname);
		outfile.close()
	}
	catch(const std:exception &e){
		cout<<"Bad output file"<<endl;
		return 1;
	}
	try{
		int n = stoi(mips_len);
	}
	catch(const std::logic_error &e){
		cout<<"Invalid parameter for pipeline length: "<<mips_len<<endl;
		return 1;
	}
	try{
		int rs=stoi(read_stages);
	}
	catch(const std::logic_error &e){
		cout<<"Invlid parameter for read stages: "<<read_stages<<endl;
		return 1;
	}

	//InstructionSet trace("../gcc-1K-randomized.trace");
	InstructionSet trace(fname);

	// n= mips pipeline length in mips-n
	int n=stoi(mips_len);

	// Read stages
	int rs=stoi(read_stages);

	int instruction=0;
	int instruction_count = trace.ic();
	int stalls[instruction_count]={0};
	int cycle = 1;
	int cycles_of_instr[instruction_count]={0};

	int stall_instances = 0;
	int stall_cycles=0;
	while(instruction<trace.ic()){
		for(int i = 1; i<=rs; i++){
			if(trace.sr[instruction*3+1]==trace.sr[instruction*3-i*3]){
				stall_instances++;
				stalls[instruction]=rs-i+1;
				cycle+=rs-i+1;
				stall_cycles+=rs-i+1;
				break;
			}
		}
		cycles_of_instr[instruction]=cycle;
		instruction++;
		cycle++;
	}

	string trace_record = "";

	for(int i = 0;i<instruction;i++){
		trace_record.append("Instruction ").append(to_string(i+1)).append(", cycle ").append(to_string(cycles_of_instr[i])).append(": 1");
		for(int j=0; j<stalls[i]; j++){
			trace_record.append(".");
		}
		for(int j=2; j<n+1;j++){
			trace_record.append(to_string(j));
		}
		trace_record.append("\n");

	}

	string trace_metrics = "";
	//TODO: Add precision control
        trace_metrics.append("Pipeline: \n\t").append(to_string(n)).append(" stages\n").append("\tOperand read stage: ").append(to_string(rs)).append("\n\tResult available stage: ").append(to_string(n)).append("\n\n").append("Pipeline simulation:\n\t").append(to_string(trace.length()/3)).append(" instructions traced\n\t").append(to_string(trace.length()/3 + n)).append(" with ideal pipeline\n\t").append(to_string(cycle-1)).append(" cycles (with stalls)\n\t").append(to_string(stall_cycles)).append(" stall cycles\n\t").append(to_string(((float) stall_instances)/((float) instruction))).append(" avg. stalls per instruction").append("\n\tIdeal Speedup: ").append(to_string(((float) instruction*n)/((float) (instruction+n-1)))).append("\n\tActual Speedup: ").append(to_string( ((float) (instruction*n)) /((float) (instruction+n+stall_cycles-1)))).append("\n\n");

	ofstream outfile;
	outfile.open(outname.append(".trace"));
	if(!outfile.write(trace_metrics.append(trace_record).c_str(),trace_metrics.append(trace_record).length())){
		cout<<"File could not be written (output folder does not exist?)"<<endl;
		return 1;
	}
	outfile.close();

	cout<<"Success! Wrote trace to "<<outname<<endl;

	return 0;
}
