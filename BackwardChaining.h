#ifndef __BACKWARD_CHAINING_H__
#define __BACKWARD_CHAINING_H__

#include <fstream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::ifstream;

#include "ListElement.h"
#include "Poison.h"

class BackwardChaining
{
	public:
		BackwardChaining(string rules="BC-Rules.txt");
		~BackwardChaining();

		static unsigned sVerboseOutput;
		static string poisonNotDiagnosed;

		Poison PoisonDiagnosed;

	private:
		void PopulateLists();
		void PopulateVariableListWithTestData();
		void ObtainConclusion();
		void VerboseOutputLists(string outputFileName, string description, vector<string>* list);
		int IsInConclusionList(const int initialIndex, const string& checkConclusion);
		void ProcessStatementClauses(const int statementIndex);
		void OutputResults(const int conclusion);
		void PopulateNonLeafConclusion(const int statementIndex);
		bool DiagnosisFound(const int statementIndex);
		string GetSpecificPoison(const int specificPoisonIndex) const;
		void DeepCopySymptoms(const int specificClauseIndex, vector<string>& poisonDiagnose) const;

		string rulesFileName;
		ifstream rulesFile;

		vector<int> conclusionStack;
		vector<VariableElement> variableList;
		vector<string> conclusionList;
		vector<string> nonLeafConclusionList;
		vector<string> uniqueConclusionList;
		vector<string> poisonConclusionList;
		vector< vector<ClauseVariableElement> > clauseVariableList;
};

#endif
