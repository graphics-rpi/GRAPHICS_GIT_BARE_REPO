#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

using std::string;
using std::set;
using std::ifstream;
using std::ofstream;
using std::cout;
using std::endl;
using std::vector;

bool isNumber(string line) {

	if(line == "0")
		return true;
	return (atoi(line.c_str()));
}

void translate(string filename, string outputfile = "")
{
	ifstream input;
	ofstream output;
	vector <string> tokens;
	vector <string> unique;
	vector <string> animals;
	vector <int> edgelist;
	string token, name;
	int temp = -1, pos = 0;

	input.open(filename.c_str());

	if(outputfile == "")
		output.open("translated.txt");
	else
		output.open(outputfile.c_str());

		input >> token;

	while(!input.eof())
	{
		tokens.push_back(token);
		input >> token;
	}

	input.close();

	for(int i = 0; i < tokens.size(); ++i)
	{
		name = tokens[i];
		if(i + 1 < tokens.size()){
			if(tokens[i+1] == "0")
			{
				animals.push_back(tokens[i]);
			}
		}

		if(!isNumber(name))
		{
			if( std::find(unique.begin(), unique.end(), name) == unique.end() )
			{
				unique.push_back(name);
				output << "# " << name << " " << name;
				output << " 0" << " 0" << " true" << " info" << " 0";
				output << " " << temp;
				output << endl;
			}
		}
		else
		{
			temp = atoi(name.c_str());
		}
	}

	edgelist.push_back(0);
	for(int i = 1; i < tokens.size(); ++i)
	{

		if(isNumber(tokens[i]))
			continue;

		if(find(animals.begin(), animals.end(), tokens[i]) == animals.end())
		{
			pos = distance(unique.begin(), find(unique.begin(), unique.end(), tokens[i]));
			edgelist.push_back(pos);
		}
		else
		{
			output << "+ " << edgelist[0] << " " << edgelist[edgelist.size()-1] << " derp" << '\n';
			for(unsigned int k = edgelist.size()-1; k > 1; --k)
			{
				output << "+ " << edgelist[k] << " " << edgelist[k-1] << " derp" << '\n';
			}
			edgelist.clear();
			edgelist.push_back(distance(unique.begin(), find(unique.begin(), unique.end(), tokens[i])));
		}

	}

	output << "+ " << edgelist[0] << " " << edgelist[edgelist.size()-1] << " derp" << '\n';
	for(unsigned int k = edgelist.size()-1; k > 1; --k)
	{
		output << "+ " << edgelist[k] << " " << edgelist[k-1] <<  " derp" << '\n';
	}
	edgelist.clear();
}

void genrandom(int numnodes, int numedges, string filename = "")
{
	ofstream output;

	if(filename == "")
		output.open("randfile.txt");
	else
		output.open(filename.c_str());

	for(int i = 0; i < numnodes; ++i)
	{
		output << "# " << i << " " << i;
		output << " 0" << " 0" << " true" << " info" << " 0";
		output << " 0";
		output << endl;
	}

	 /* initialize random seed: */
	srand ( time(NULL) );
	int first, second;

	for(int i = 0; i < numedges; ++i)
	{
		  /* generate secret number: */
		first = rand() % numnodes;
		second = rand() % numnodes;
		output << "+ " << first << " " << second << " derp" << '\n';
	}

}

int main(int argc, char **argv)
{

	if(string(argv[1]) == "-random" && argc > 3)
	{
		genrandom(atoi(argv[2]), atoi(argv[3]));
	}
	else if(string(argv[1]) == "-translate" && argc > 3)
	{
		if(argc == 3)
			translate(argv[2]);
		else if(argc == 4)
			translate(argv[2], argv[3]);
	}
	else
	{
		cout << "Invalid arguments: \n\t";
		cout << "Usage: [-random | -translate] [numnodes,numedges]\n "
				"\t       [input file, (optional) output file]\n";
	}


}
