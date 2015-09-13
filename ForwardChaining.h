#ifndef __FORWARD_CHAINING_H__
#define __FORWARD_CHAINING_H__

#include <fstream>
#include <string>
#include <vector>
#include <queue>

#include "ListElement.h"

using std::string;
using std::vector;
using std::ifstream;
using std::queue;

class ForwardChaining
{
	public:
		ForwardChaining(string rules="FC-Rules.txt");
		~ForwardChaining();

		static unsigned sVerboseOutput;

		void Trigger(string poison);

	private:
		void PopulateLists();
		void VerboseOutputLists(string outputFileName, string description, vector<string>* list);
		int IsInVariableList(const string& potentialVariable);
		int ProcessStatementClauses(unsigned clauseVariableListIndex, string foundVariable);
		bool TreatmentFound(const int& clauseVariableListIndex) const;

		string rulesFileName;
		ifstream rulesFile;

		queue<ConclusionQueueElement> conclusionQueue;
		vector<VariableElement> variableList;
		vector< vector<string> > conclusionList;
		vector<string> nonLeafConclusionList;
		vector< vector<ClauseVariableElement> > clauseVariableList;
};

#endif