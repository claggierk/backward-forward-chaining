#ifndef __POISON_H__
#define __POISON_H__

#include <string>
#include <vector>

using std::string;
using std::vector;

class Poison
{
	public:
		Poison()
		{}
		~Poison()
		{}

		string name;
		vector<string> symptoms;
	private:
};

#endif