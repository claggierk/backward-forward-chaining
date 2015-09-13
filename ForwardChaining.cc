#include <fstream>
#include <iostream>
#include <sstream>

#include "ForwardChaining.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::cin;
using std::ostringstream;

extern ofstream fout;

unsigned ForwardChaining::sVerboseOutput = 1;

// --------------------------------------------------------------------------------------------------
ForwardChaining::ForwardChaining(string rules) : rulesFileName(rules)
{
	PopulateLists();
}

// --------------------------------------------------------------------------------------------------
ForwardChaining::~ForwardChaining()
{}

// --------------------------------------------------------------------------------------------------
void ForwardChaining::Trigger(string poison)
{
	int variableListIndexOfPoison = IsInVariableList(poison);
	if(variableListIndexOfPoison == -1)
	{
		throw string("##### WARNING: Your triggered poison is not in the variable list!");
	}

	ConclusionQueueElement cqe;
	cqe.variableName = poison;
	cqe.variableListIndex = variableListIndexOfPoison;
	cqe.clauseVariableListIndex = 0;

	conclusionQueue.push(cqe);

	// set the current size of the conclusionQueue
	int sizeOfConclusionQueue = conclusionQueue.size();

	int clauseVariableStatementIndex = 0;
	
	while(conclusionQueue.size() > 0)
	{
		clauseVariableStatementIndex = ProcessStatementClauses(conclusionQueue.front().clauseVariableListIndex, conclusionQueue.front().variableName);
		if(clauseVariableStatementIndex == -1)
		{
			conclusionQueue.pop();
			continue;
		}

		conclusionQueue.front().clauseVariableListIndex = clauseVariableStatementIndex;

		if(sizeOfConclusionQueue != conclusionQueue.size())
		{
			// dealing with non-leaf conclusions
			sizeOfConclusionQueue = conclusionQueue.size();
			continue;
		}

		else
		{
			// check to see if conditions were met
			bool treatmentFound = TreatmentFound(clauseVariableStatementIndex);

			if(treatmentFound)
			{
				cout << "\n *********** Size of queue: " << conclusionQueue.size();
				fout << "\n *********** Size of queue: " << conclusionQueue.size();
				// check to see if any of the treatments found are really non-leaf conclusions
				for(unsigned nonLeafConclusion = 0; nonLeafConclusion < nonLeafConclusionList.size(); nonLeafConclusion++)
				{
					for(unsigned conclusion = 0; conclusion < conclusionList.at(clauseVariableStatementIndex).size(); conclusion++)
					{
						if(conclusionList.at(clauseVariableStatementIndex).at(conclusion) == nonLeafConclusionList.at(nonLeafConclusion))
						{
							cout << endl << " ----- INFO: Non-leaf conclusion located";
						}
					}
				}

				fout << endl 
					<< endl << " ****************************************************"
					<< endl << " ***** Treatment(s) *********************************";
				
				cout << endl 
					<< endl << " ****************************************************"
					<< endl << " ***** Treatment(s) *********************************";
				for(unsigned int conclusion = 0; conclusion < conclusionList.at(clauseVariableStatementIndex).size(); conclusion++)
				{
					cout << endl << "   " << conclusionList.at(clauseVariableStatementIndex).at(conclusion);
					fout << endl << "   " << conclusionList.at(clauseVariableStatementIndex).at(conclusion);
				}
				cout << endl << " ****************************************************";
				fout << endl << " ****************************************************";
				break;
			}
			else
			{
				// update the searching conclusionQueue.front().clauseVariableListIndex
				conclusionQueue.front().clauseVariableListIndex = conclusionQueue.front().clauseVariableListIndex + 1;
			}
		}

	}
}

// --------------------------------------------------------------------------------------------------
void ForwardChaining::PopulateLists()
{
	rulesFile.open(rulesFileName.c_str());

	string currentWord = "";
	if(rulesFile.is_open())
	{
		cout << endl << endl;
		fout << endl << endl;
		cout << endl << "***********************************"
			<< endl << "***** Processing FC-Rules.txt *****"
			<< endl << "***********************************";
		fout << endl << "***********************************"
			<< endl << "***** Processing FC-Rules.txt *****"
			<< endl << "***********************************";

		rulesFile >> currentWord;
		int ifLineNumber = 1;

		int counter = -1;

		// each iteration of the while loop takes care of:
		// 1 IF statement
		// 1 THEN statement
		// Every IF statement can have multiple variables (but at least 1)
		// Every THEN statement can have multiple variable (but at least 1)
		while(! rulesFile.eof())
		{
			if(currentWord != "IF")
			{
				cout << endl << "##### ERROR: Line " << ifLineNumber << " does not begin with: IF";
				cout << endl << "             It Begins with: " << currentWord;
				return;
			}

			string pastVariableWithoutNOT = "";
			string currentVariableWithoutNOT = "";
			rulesFile >> currentWord;
			vector<ClauseVariableElement> currentClause;

			// initialize currentClause to nothing
			currentClause.clear();

			counter++;
			// extract all of the IF variables
			while(currentWord != "THEN")
			{
				// ignore all "&"
				if(currentWord == "&")
				{
					rulesFile >> currentWord;
					continue;
				}

				// add currentWord to the currentClause
				ClauseVariableElement cve;
				cve.clauseName = currentWord;
				cve.clauseValue = -1;
				currentClause.push_back(cve);
 
				bool addToVariableList = true;
				// search for the new potential variable in the variable list
				for(unsigned variable = 0; variable < variableList.size(); variable++)
				{
					pastVariableWithoutNOT = variableList.at(variable).variableName;
					// remove the "!" preceding the past variable if its there
					if(variableList.at(variable).variableName.at(0) == '!')
					{
						pastVariableWithoutNOT = "";
						// start at index 1 (ignoring the "!")
						for(unsigned character = 1; character < variableList.at(variable).variableName.size(); character++)
						{
							pastVariableWithoutNOT = pastVariableWithoutNOT + variableList.at(variable).variableName.at(character);
						}
					}

					currentVariableWithoutNOT = currentWord;
					// remove the "!" preceding the current variable if its there
					if(currentWord.at(0) == '!')
					{
						currentVariableWithoutNOT = "";
						for(unsigned character = 1; character < currentWord.size(); character++)
						{
							currentVariableWithoutNOT = currentVariableWithoutNOT + currentWord.at(character);
						}
					}

					// only add unique variables
					if(currentVariableWithoutNOT == pastVariableWithoutNOT)
					{
						addToVariableList = false;
						break;
					}
				}

				// if the for loop ends WITHOUT changing addToVariableList to false... then the variable is unique
				if(addToVariableList)
				{
					variableList.push_back(VariableElement(currentWord, -1));
				}

				rulesFile >> currentWord;
			}
			// add current clauses to clauseVariableList
			clauseVariableList.push_back(currentClause);

			// Right now, currentWord is "THEN" ... grab the next word ...
			rulesFile >> currentWord;
			
			vector<string> currentConclusions;
			if(rulesFile.eof())
			{
				// if the very last THEN statement in the rules is only ONE variable long... this is needed
				// definitely a corner case
				currentConclusions.push_back(currentWord);
			}

			while(currentWord != "IF" && !rulesFile.eof())
			{
				// ignore all "&"
				if(currentWord == "&")
				{
					rulesFile >> currentWord;
					continue;
				}

				currentConclusions.push_back(currentWord);
				rulesFile >> currentWord;
			}
			// add current conclusions to conclusion list
			conclusionList.push_back(currentConclusions);

			// update line number
			ifLineNumber = ifLineNumber + 3;

			// Process the next IF/THEN statement
			// currentWord has "IF" in it right now...
		}
		
		rulesFile.close();

		cout << endl << "Extracted " << variableList.size() << " \"IF\" variables. (variables)";
		cout << endl << "Extracted " << conclusionList.size() << " \"THEN\" variables. (conclusions)" << endl;

		fout << endl << "Extracted " << variableList.size() << " \"IF\" variables. (variables)";
		fout << endl << "Extracted " << conclusionList.size() << " \"THEN\" variables. (conclusions)" << endl;
	}
	else
	{
		ostringstream oss;
		oss << "##### ERROR: Unable to open " << rulesFileName;
		throw(oss.str());
	}

	// in forward chaining...
	// if a variable is in both an "IF CLAUSE" and a "THEN CLAUSE"...
	// it belongs in the variable list only!
	// therefore, if the conclusion and variable list have a common variable... remove it from the variable list
	for(unsigned variable = 0; variable < variableList.size(); variable++)
	{
		for(unsigned int currentConclusionList = 0; currentConclusionList < conclusionList.size(); currentConclusionList++)
		{
			unsigned conclusion = 0;
			for(vector<string>::iterator iter = conclusionList.at(currentConclusionList).begin(); iter != conclusionList.at(currentConclusionList).end(); iter++)
			{
				if(variableList.at(variable).variableName == conclusionList.at(currentConclusionList).at(conclusion))
				{
					cout << endl << " ----- INFO: variable list and conclusion list have " << conclusionList.at(currentConclusionList).at(conclusion) << " in common."
						<< endl << "             Removing from the conclusion list";
					
					fout << endl << " ----- INFO: variable list and conclusion list have " << conclusionList.at(currentConclusionList).at(conclusion) << " in common."
						<< endl << "             Removing from the conclusion list";
					nonLeafConclusionList.push_back(conclusionList.at(currentConclusionList).at(conclusion));
					// dont delete it
					// conclusionList.at(currentConclusionList).erase(iter);
					// check the next variable
					break;
				}
				conclusion++;
			}
		}
	}

	if(ForwardChaining::sVerboseOutput)
	{
		// output the variable list
		ofstream variableListFile("FC-VariableList.txt");
		cout << endl << "FC-Variable List:" << endl;
		fout << endl << "FC-Variable List:" << endl;
		for(unsigned variable = 0; variable < variableList.size(); variable++)
		{
			cout << variable + 1 << " " << variableList.at(variable).variableName << endl;
			fout << variable + 1 << " " << variableList.at(variable).variableName << endl;
			variableListFile << variableList.at(variable).variableName << ": " << variableList.at(variable).variableValue << endl;
		}
		variableListFile.close();
		cout << endl;
		fout << endl;

		// output the conclusion list
		ofstream conclusionListFile("FC-ConclusionList.txt");
		for(unsigned currentConclusioList = 0; currentConclusioList < conclusionList.size(); currentConclusioList++)
		{
			cout << "Treatments: ";
			fout << "Treatments: ";
			conclusionListFile << "Treatments: ";
			for(unsigned conclusion = 0; conclusion < conclusionList.at(currentConclusioList).size(); conclusion++)
			{
				cout << conclusionList.at(currentConclusioList).at(conclusion) << " & ";
				fout << conclusionList.at(currentConclusioList).at(conclusion) << " & ";
				conclusionListFile << conclusionList.at(currentConclusioList).at(conclusion) << " ";
			}
			cout << endl;
			fout << endl;
			conclusionListFile << endl;
		}
		conclusionListFile.close();
		cout << endl;
		fout << endl;

		// output the non-leaf conclusion list
		VerboseOutputLists(string("FC-NonLeafConclusionList.txt"), string("Non Leaf Conclusion List:"), &nonLeafConclusionList);
		cout << endl;
		fout << endl;

		// output the clause variable list
		ofstream clauseVariableListFile("FC-ClauseVariableList.txt");
		for(unsigned clause = 0; clause < clauseVariableList.size(); clause++)
		{
			cout << "Clause " << clause + 1 << ":";
			fout << "Clause " << clause + 1 << ":";
			clauseVariableListFile << "Clause " << clause + 1 << ":" << endl;
			for(unsigned variable = 0; variable < clauseVariableList.at(clause).size(); variable++)
			{
				cout << clauseVariableList.at(clause).at(variable).clauseName << " ";
				fout << clauseVariableList.at(clause).at(variable).clauseName << " ";
				clauseVariableListFile << "   " << clauseVariableList.at(clause).at(variable).clauseName << ": " << clauseVariableList.at(clause).at(variable).clauseValue << endl;
			}
			cout << endl;
			fout << endl;
			clauseVariableListFile << endl;
		}
		clauseVariableListFile.close();
		cout << endl;
		fout << endl;
	}
}

// --------------------------------------------------------------------------------------------------
void ForwardChaining::VerboseOutputLists(string outputFileName, string description, vector<string>* list)
{
	ofstream outputFile(outputFileName.c_str());
	cout << endl << description << endl;
	fout << endl << description << endl;
	for(unsigned conclusion = 0; conclusion < list->size(); conclusion++)
	{
		//cout << conclusion + 1 << " " <<  poisonConclusionList.at(conclusion) << endl;
		outputFile << list->at(conclusion) << endl;
		fout << list->at(conclusion) << endl;
	}
	outputFile.close();
}

// --------------------------------------------------------------------------------------------------
int ForwardChaining::IsInVariableList(const string& potentialVariable)
{
	// search the variable list for key
	for(unsigned variable = 0; variable < variableList.size(); variable++)
	{
		if(variableList.at(variable).variableName == potentialVariable)
		{
			cout << endl << " ----- INFO: Found: " << potentialVariable << " in the variable list at index: " << variable;
			fout << endl << " ----- INFO: Found: " << potentialVariable << " in the variable list at index: " << variable;
			variableList.at(variable).variableValue = 1;
			return variable;
		}
	}

	// return -1 if the potentialVariable is in fact NOT a variable
	return -1;
}

// --------------------------------------------------------------------------------------------------
int ForwardChaining::ProcessStatementClauses(unsigned clauseVariableListIndex, string foundVariable)
{
	cout << endl << " ---------------------------------------------------------------------"
		<< endl << " ----- INFO: Searching the clause variable list for: " << foundVariable;
	fout << endl << " ---------------------------------------------------------------------"
		<< endl << " ----- INFO: Searching the clause variable list for: " << foundVariable;
	
	int foundStatementIndex = -1;
	string statementClauseVariableWithoutNOT = "";
	// always start at the statement...
	for(unsigned statement = clauseVariableListIndex; statement < clauseVariableList.size(); statement++)
	{
		for(unsigned clause = 0; clause < clauseVariableList.at(statement).size(); clause++)
		{
			statementClauseVariableWithoutNOT = clauseVariableList.at(statement).at(clause).clauseName;
			if(statementClauseVariableWithoutNOT.at(0) == '!')
			{
				// start at index 1 ... thus ignoring the '!'
				string tempStatementClauseVariableWithoutNOT = "";
				for(unsigned character = 1; character < statementClauseVariableWithoutNOT.size(); character++)
				{
					tempStatementClauseVariableWithoutNOT = tempStatementClauseVariableWithoutNOT + statementClauseVariableWithoutNOT.at(character);
				}
				statementClauseVariableWithoutNOT = tempStatementClauseVariableWithoutNOT;
			}

			// we found a clause that has one of its variables as the one we are looking for...
			if(foundVariable == statementClauseVariableWithoutNOT)
			{
				cout << endl << " ----- INFO: Found: " << foundVariable << " at Statement: " << statement << " --- Clause: " << clause;
				fout << endl << " ----- INFO: Found: " << foundVariable << " at Statement: " << statement << " --- Clause: " << clause;
				foundStatementIndex = statement;
				break;
			}
		}
		if(foundStatementIndex != -1)
		{
			break;
		}
	}

	if(foundStatementIndex == -1)
	{
		return foundStatementIndex;
	}

	//otherwise...
	// deal with every clause in this statement
	for(unsigned clause = 0; clause < clauseVariableList.at(foundStatementIndex).size(); clause++)
	{
		statementClauseVariableWithoutNOT = clauseVariableList.at(foundStatementIndex).at(clause).clauseName;
		cout << "\n ***** Deal with " << statementClauseVariableWithoutNOT;
		fout << "\n ***** Deal with " << statementClauseVariableWithoutNOT;
		if(statementClauseVariableWithoutNOT.at(0) == '!')
		{
			// start at index 1 ... thus ignoring the '!'
			string tempStatementClauseVariableWithoutNOT = "";
			for(unsigned character = 1; character < statementClauseVariableWithoutNOT.size(); character++)
			{
				tempStatementClauseVariableWithoutNOT = tempStatementClauseVariableWithoutNOT + statementClauseVariableWithoutNOT.at(character);
			}
			statementClauseVariableWithoutNOT = tempStatementClauseVariableWithoutNOT;
		}

		// **************************************************************************************************
		// first check if its in the conclusion list as well ... if it is ... we need to add it to the queue
		// or in other words... check to see if its a non-leaf conclusion ... if it is... add it to the queue
		bool isNonLeafConclusion = false;
		for(unsigned nonLeafConclusion = 0; nonLeafConclusion < nonLeafConclusionList.size(); nonLeafConclusion++)
		{
			cout << "\n ***** statementClauseVariableWithoutNOT: " << statementClauseVariableWithoutNOT << " ??? " << nonLeafConclusionList.at(nonLeafConclusion) << " = nonLeafConclusionList.at(nonLeafConclusion): ";
			fout << "\n ***** statementClauseVariableWithoutNOT: " << statementClauseVariableWithoutNOT << " ??? " << nonLeafConclusionList.at(nonLeafConclusion) << " = nonLeafConclusionList.at(nonLeafConclusion): ";
			if(statementClauseVariableWithoutNOT == nonLeafConclusionList.at(nonLeafConclusion))
			{
				cout << endl << " ----- INFO: Located a non leaf conclusion."
					<< endl << "             Statement: " << foundStatementIndex << " --- Clause: " << clause
					<< endl << "             Adding \"" <<  nonLeafConclusionList.at(nonLeafConclusion) << "\" to the queue";
				fout << endl << " ----- INFO: Located a non leaf conclusion."
					<< endl << "             Statement: " << foundStatementIndex << " --- Clause: " << clause
					<< endl << "             Adding \"" <<  nonLeafConclusionList.at(nonLeafConclusion) << "\" to the queue";
				
				ConclusionQueueElement cqe;
				cqe.variableName = nonLeafConclusion;
				cqe.variableListIndex = 0;
				
				conclusionQueue.push(cqe);
				isNonLeafConclusion = true;
			}
		}
		if(isNonLeafConclusion)
		{
			continue;
		}

		unsigned variableIndex = 0;
		bool foundPopulatedInVariableList = false;
		// **************************************************************************************************
		// next check to see if any of the clause variables have already been populated in the variable list
		for(unsigned variable = 0; variable < variableList.size(); variable++)
		{
			if(statementClauseVariableWithoutNOT == variableList.at(variable).variableName)
			{
				// we found that they have the same name... now check the value
				variableIndex = variable;
				if(variableList.at(variable).variableValue != -1)
				{
					cout << endl << " ----- INFO: Retreived data from variable list concerning: " << variableList.at(variable).variableName << ": " << variableList.at(variable).variableValue;
					fout << endl << " ----- INFO: Retreived data from variable list concerning: " << variableList.at(variable).variableName << ": " << variableList.at(variable).variableValue;
					foundPopulatedInVariableList = true;
					// the variable has already been populated in the variable list
					if(clauseVariableList.at(foundStatementIndex).at(clause).clauseName.at(0) == '!')
					{
						int updateVal = variableList.at(variable).variableValue;
						if(updateVal == 0)
						{
							clauseVariableList.at(foundStatementIndex).at(clause).clauseValue = 1;
							cout << "\n *** STORING: " << clauseVariableList.at(foundStatementIndex).at(clause).clauseValue;
							fout << "\n *** STORING: " << clauseVariableList.at(foundStatementIndex).at(clause).clauseValue;
						}
						else if(updateVal == 1)
						{
							clauseVariableList.at(foundStatementIndex).at(clause).clauseValue = 0;
							cout << "\n *** STORING: " << clauseVariableList.at(foundStatementIndex).at(clause).clauseValue;
							fout << "\n *** STORING: " << clauseVariableList.at(foundStatementIndex).at(clause).clauseValue;
						}
						else
						{
							throw string("##### ERROR: Unknown value stored in the Forward Chaining variables list");
						}
					}
					else
					{
						clauseVariableList.at(foundStatementIndex).at(clause).clauseValue = variableList.at(variable).variableValue;
					}
					break;
				}
			}
		}
		if(foundPopulatedInVariableList)
		{
			continue;
		}

		// **************************************************************************************************
		// Lastly ... ask the user
		cout << endl 
			<< endl << " ---------------------------------------------------"
			<< endl << " ----- " << statementClauseVariableWithoutNOT << "?"
			<< endl << " ---------------------------------------------------"
			<< endl
			<< endl << "  Yes"
			<< endl << "  No"
			<< endl
			<< endl << " Response: ";
		fout << endl 
			<< endl << " ---------------------------------------------------"
			<< endl << " ----- " << statementClauseVariableWithoutNOT << "?"
			<< endl << " ---------------------------------------------------"
			<< endl
			<< endl << "  Yes"
			<< endl << "  No"
			<< endl
			<< endl << " Response: ";
		string response;
		cin >> response;

		if(response != "Yes" && response != "No")
		{
			throw string("##### ERROR: Invalid response to knowledge question (answer only \"Yes\" or \"No\")");
		}
		int updatedVariable = 0;
		if(response == "Yes")
		{
			updatedVariable = 1;
		}
		else if(response == "No")
		{
			updatedVariable = 0;
		}
		cout << endl << "\n You answered: \"" << response << "\"";
		fout << endl << "\n You answered: \"" << response << "\"";

		// need to update the variable list and the clause variable list
		variableList.at(variableIndex).variableValue = updatedVariable;
		if(clauseVariableList.at(foundStatementIndex).at(clause).clauseName.at(0) == '!')
		{
			int updateVal = variableList.at(variableIndex).variableValue;
			if(updateVal == 0)
			{
				updatedVariable = 1;
			}
			else if(updateVal == 1)
			{
				updatedVariable = 0;
			}
		}
		clauseVariableList.at(foundStatementIndex).at(clause).clauseValue = updatedVariable;
	}

	return foundStatementIndex;
}

bool ForwardChaining::TreatmentFound(const int& clauseVariableListIndex) const
{
	bool treatmentFound = true;
	cout << endl << " ----- INFO: Treatment Check: ";
	fout << endl << " ----- INFO: Treatment Check: ";
	for(unsigned clause = 0; clause < clauseVariableList.at(clauseVariableListIndex).size(); clause++)
	{
		cout << endl << "   clauseVariableList[" << clauseVariableListIndex << "][" << clause << "] = " << clauseVariableList.at(clauseVariableListIndex).at(clause).clauseValue;
		fout << endl << "   clauseVariableList[" << clauseVariableListIndex << "][" << clause << "] = " << clauseVariableList.at(clauseVariableListIndex).at(clause).clauseValue;
		if(clauseVariableList.at(clauseVariableListIndex).at(clause).clauseValue != 1)
		{
			treatmentFound = false;
			break;
		}
	}

	return treatmentFound;
}
