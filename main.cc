#include <iostream>
#include <fstream>

#include "Poison.h"
#include "BackwardChaining.h"
#include "ForwardChaining.h"

using std::cout;
using std::endl;
using std::cin;
using std::ofstream;

ofstream fout;

int main(int argc, char* argv[])
{
	try
	{
		fout.open("chaining-log.txt");
		if( !fout.is_open() )
		{
			throw string("Cannot open output file for commands.");
		}

		// *************************************************************
		// ***** backward chaining ... diagnose the poison
		// *************************************************************
		BackwardChaining BackwardChainingInstance("BC-Rules.txt");

		if(BackwardChainingInstance.PoisonDiagnosed.name == BackwardChaining::poisonNotDiagnosed)
		{
			cout << endl << "##### WARNING: ";
			throw("No poison was not diagnosed");
		}

		// *************************************************************
		// ***** forward chaining ... recommend treatment
		// *************************************************************
		ForwardChaining ForwardChainingInstance("FC-Rules.txt");
		// ForwardChainingInstance.Trigger("Arsenic");
		ForwardChainingInstance.Trigger(BackwardChainingInstance.PoisonDiagnosed.name);
	}
	catch(string message)
	{
		cout << endl << message;
	}

	cout << endl << endl << "Exiting . . ." << endl;
}
