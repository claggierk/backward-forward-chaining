#include <fstream>
#include <iostream>
#include <sstream>

#include "BackwardChaining.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::cin;
using std::ostringstream;

extern ofstream fout;

unsigned BackwardChaining::sVerboseOutput = 1;
string BackwardChaining::poisonNotDiagnosed = "Indeterminable";

// --------------------------------------------------------------------------------------------------
BackwardChaining::BackwardChaining(string rules) : rulesFileName(rules)
{
	PoisonDiagnosed.name = BackwardChaining::poisonNotDiagnosed;
	PopulateLists();

	// --------------------------- FOR TESTING PURUPSES ---------------------------
	// Look at this function if uncommented and configure variables in variable list
	//      to achieve a specific poison
	// PopulateVariableListWithTestData();
	// ----------------------------------------------------------------------------
	ObtainConclusion();
}

// --------------------------------------------------------------------------------------------------
BackwardChaining::~BackwardChaining()
{}

// --------------------------------------------------------------------------------------------------
void BackwardChaining::PopulateLists()
{
	rulesFile.open(rulesFileName.c_str());

	string currentWord = "";
	if(rulesFile.is_open())
	{
		cout << endl << endl;
		fout << endl << endl;
		cout << endl << "***********************************"
			<< endl << "***** Processing BC-Rules.txt *****"
			<< endl << "***********************************";
		fout << endl << "***********************************"
			<< endl << "***** Processing BC-Rules.txt *****"
			<< endl << "***********************************";
		rulesFile >> currentWord;
		int ifLineNumber = 1;

		// each iteration of the while loop takes care of:
		// 1 IF statement
		// 1 THEN statement
		// Every IF statement can have multiple variables (but at least 1)
		// Every THEN statement can have only ONE variable
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

			// currentWord = THEN right now
			rulesFile >> currentWord;
			if(currentWord == "Poison")
			{
				rulesFile >> currentWord;
				if(currentWord != "=")
				{
					cout << endl << "##### ERROR: Line " << ifLineNumber + 1 << ":"
						 << endl << "             A non \"=\" stands after \"Poison\"";
					fout << endl << "##### ERROR: Line " << ifLineNumber + 1 << ":"
						 << endl << "             A non \"=\" stands after \"Poison\"";
				}

				// this is a poison
				rulesFile >> currentWord;
				poisonConclusionList.push_back(currentWord);
				conclusionList.push_back("Poison");
			}
			else
			{
				// extract the NON-POISON conclusion to conclusion list
				conclusionList.push_back(currentWord);

				// extract the NON-POISON conclusion to the poison specific conclusion list
				poisonConclusionList.push_back(currentWord);

				// extract the NON Leaf conclusions
				nonLeafConclusionList.push_back(currentWord);
			}

			bool unique = true;
			for(unsigned uniqueConclusion = 0; uniqueConclusion < uniqueConclusionList.size(); uniqueConclusion++)
			{
				if(conclusionList.back() == uniqueConclusionList.at(uniqueConclusion))
				{
					unique = false;
					break;
				}
			}
			if(unique)
			{
				uniqueConclusionList.push_back(conclusionList.back());
			}

			// add current clauses to clauseVariableList
			clauseVariableList.push_back(currentClause);

			// extract the IF into currentWord
			rulesFile >> currentWord;

			// update line number
			ifLineNumber = ifLineNumber + 3;
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
	// therefore, if the conclusion and variable list have a common variable... remove it from the conclusion list
	for(unsigned conclusion = 0; conclusion < conclusionList.size(); conclusion++)
	{
		unsigned variable = 0;
		for(vector<VariableElement>::iterator iter = variableList.begin(); iter != variableList.end(); iter++)
		{
			//cout << endl << "conclusionList[" << conclusion << "] = " << conclusionList.at(variable) << " ??? variableList[" << variable<< "] = " << variableList.at(variable).variableName;
			if(conclusionList.at(conclusion) == variableList.at(variable).variableName)
			{
				cout << endl << " ----- INFO: variable list and conclusion list have " << variableList.at(variable).variableName << " in common."
					<< endl << "             Removing from the variable list";
				fout << endl << " ----- INFO: variable list and conclusion list have " << variableList.at(variable).variableName << " in common."
					<< endl << "             Removing from the variable list";
				variableList.erase(iter);
				// check the next variable
				break;
			}
			variable++;
		}
	}

	if(BackwardChaining::sVerboseOutput)
	{
		cout << endl;
		fout << endl;
		// output the variable list
		ofstream variableListFile("BC-VariableList.txt");
		cout << endl << "Variable List:" << endl;
		fout << endl << "Variable List:" << endl;
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
		VerboseOutputLists(string("BC-ConclusionList.txt"), string("Conclusion List:"), &conclusionList);
		cout << endl;
		fout << endl;

		// output the non-leaf conclusion list
		VerboseOutputLists(string("BC-NonLeafConclusionList.txt"), string("Non Leaf Conclusion List:"), &nonLeafConclusionList);
		cout << endl;
		fout << endl;

		// output the unique conclusion list
		VerboseOutputLists(string("BC-UniqueConclusionList.txt"), string("Unique Conclusion List:"), &uniqueConclusionList);
		cout << endl;
		fout << endl;

		// output the poison specific conclusion list
		VerboseOutputLists(string("BC-PoisonConclusionList.txt"), string("Poison Conclusion List:"), &poisonConclusionList);
		cout << endl;
		fout << endl;

		// output the clause variable list
		ofstream clauseVariableListFile("BC-ClauseVariableList.txt");
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
void BackwardChaining::PopulateVariableListWithTestData()
{
	// a list of all the variables in the variable lists
	/*
	Low_Blood_Pressure: -1
	Holes_In_The_Skin: -1
	Coughing: -1
	Wheezing: -1
	VomitBlood: -1
	BloodyStool: -1
	Sneezing: -1
	Edema: -1
	Difficulty_Breathing: -1
	Shortness_Of_Breath: -1
	Drowsy: -1
	BlueLipsFingernails: -1
	Drooling: -1
	EyeIrritation: -1
	Blisters: -1
	Fever: -1
	IrritatedBurnedSkin: -1
	Peeling: -1
	Vomiting_(NonBlood): -1
	BiteSting: -1
	Collapse: -1
	Sweating: -1
	Diarrhea: -1
	Numbness: -1
	RapidHeartRate: -1
	Anemia: -1
	Constipation: -1
	YellowSkin: -1
	ThickenedSkin: -1
	LipsSwelling: -1
	NonLinearRash: -1
	SlowedBreathing: -1
	Altered_Heart_Rate: -1
	PinkCherryRedSkin: -1
	Linear_Red_Rash: -1
	Skin_Irritated_Swelling: -1
	Light_Red_Patches: -1
	Large_Fluid_Filled_Blisters: -1
	*/

	vector<VariableElement> testData;

	// add whatever variables you want!!!

	testData.push_back(VariableElement("Low_Blood_Pressure", 1));
	testData.push_back(VariableElement("Holes_In_The_Skin", 1));
	testData.push_back(VariableElement("Coughing", 1));
	testData.push_back(VariableElement("Wheezing", 0));
	testData.push_back(VariableElement("VomitBlood", 1));
	testData.push_back(VariableElement("BloodyStool", 1));
	testData.push_back(VariableElement("Sneezing", 0));
	testData.push_back(VariableElement("Edema", 0));
	testData.push_back(VariableElement("Difficulty_Breathing", 0));
	testData.push_back(VariableElement("Shortness_Of_Breath", 0));
	testData.push_back(VariableElement("Drowsy", 0));
	testData.push_back(VariableElement("BlueLipsFingernails", 0));
	testData.push_back(VariableElement("Drooling", 0));
	testData.push_back(VariableElement("EyeIrritation", 0));
	testData.push_back(VariableElement("Blisters", 0));
	testData.push_back(VariableElement("Fever", 0));
	testData.push_back(VariableElement("IrritatedBurnedSkin", 0));
	testData.push_back(VariableElement("Peeling", 0));
	testData.push_back(VariableElement("Vomiting_(NonBlood)", 0));
	testData.push_back(VariableElement("BiteSting", 0));
	testData.push_back(VariableElement("Collapse", 0));
	testData.push_back(VariableElement("Sweating", 0));
	testData.push_back(VariableElement("Diarrhea", 0));
	testData.push_back(VariableElement("Numbness", 0));
	testData.push_back(VariableElement("RapidHeartRate", 0));
	testData.push_back(VariableElement("Anemia", 0));
	testData.push_back(VariableElement("Constipation", 0));
	testData.push_back(VariableElement("YellowSkin", 0));
	testData.push_back(VariableElement("ThickenedSkin", 0));
	testData.push_back(VariableElement("LipsSwelling", 0));
	testData.push_back(VariableElement("NonLinearRash", 0));
	testData.push_back(VariableElement("SlowedBreathing", 0));
	testData.push_back(VariableElement("Altered_Heart_Rate", 0));
	testData.push_back(VariableElement("PinkCherryRedSkin", 0));
	testData.push_back(VariableElement("Linear_Red_Rash", 0));
	testData.push_back(VariableElement("Skin_Irritated_Swelling", 0));
	testData.push_back(VariableElement("Light_Red_Patches", 0));
	testData.push_back(VariableElement("Large_Fluid_Filled_Blisters", 0));

	for(unsigned testDatum = 0; testDatum < testData.size(); testDatum++)
	{
		for(unsigned variable = 0; variable < variableList.size(); variable++)
		{
			if(variableList.at(variable).variableName == testData.at(testDatum).variableName)
			{
				variableList.at(variable).variableValue = testData.at(testDatum).variableValue;
				break;
			}
		}
	}

	for(unsigned variable = 0; variable < variableList.size(); variable++)
	{
		cout << endl << " --- Variable[" << variableList.at(variable).variableName << "] = " << variableList.at(variable).variableValue;
	}
}

// --------------------------------------------------------------------------------------------------
void BackwardChaining::VerboseOutputLists(string outputFileName, string description, vector<string>* list)
{
	ofstream outputFile(outputFileName.c_str());
	cout << endl << description << endl;
	fout << endl << description << endl;
	for(unsigned conclusion = 0; conclusion < list->size(); conclusion++)
	{
		cout << conclusion + 1 << " " <<  poisonConclusionList.at(conclusion) << endl;
		fout << conclusion + 1 << " " <<  poisonConclusionList.at(conclusion) << endl;
		outputFile << list->at(conclusion) << endl;
	}
	outputFile.close();
}

// --------------------------------------------------------------------------------------------------
void BackwardChaining::ObtainConclusion()
{
	cout << endl << "Please enter one of the following conclusions as your goal:" << endl;
	fout << endl << "Please enter one of the following conclusions as your goal:" << endl;
	for(unsigned conclusion = 0; conclusion < uniqueConclusionList.size(); conclusion++)
	{
		cout << "   " << uniqueConclusionList.at(conclusion) << endl;
		fout << "   " << uniqueConclusionList.at(conclusion) << endl;
	}
	cout << "Response: ";
	fout << "Response: ";

	string conclusionRequest;
	cin >> conclusionRequest;

	bool validResponse = false;
	for(unsigned conclusion = 0; conclusion < uniqueConclusionList.size(); conclusion++)
	{
		if(conclusionRequest == uniqueConclusionList.at(conclusion))
		{
			validResponse = true;
			break;
		}
	}

	if(!validResponse)
	{
		throw string("##### ERROR: Invalid conclusion/goal request (This conclusion/goal cannot be found)");
	}

	cout << endl << "  ***** Your choice: " << conclusionRequest;
	fout << endl << "  ***** Your choice: " << conclusionRequest;

	// 2. Search the conclusion list for the FIRST instance of the conclusion's name
	int conclusionIndex = IsInConclusionList(0, conclusionRequest);

	// these are the same
	int clauseVariableIndex = conclusionIndex;

	// conclusion has been found...  placing the FOUND conclusion onto conclusionStack
	conclusionStack.push_back(clauseVariableIndex);

	// set the current size of the conclusionStack
	int sizeOfConclusionStack = conclusionStack.size();

	while(conclusionStack.size() > 0)
	{
		ProcessStatementClauses(conclusionStack.back());

		// if the size of the conclusion stack changed... then we need to process the new conclusion
		if(sizeOfConclusionStack != conclusionStack.size())
		{
			sizeOfConclusionStack = conclusionStack.size();
			continue;
		}

		// if the size of the conclusion stack did NOT change...
		// we need to pop away all of its contents one by one and update the clause variable list with the data we have gained about the non-leaf conclusion
		else
		{
			unsigned initialSize = conclusionStack.size();
			for(unsigned conclusion = conclusionStack.size()-1; conclusion < initialSize; conclusion--)
			{
				PopulateNonLeafConclusion(conclusionStack.at(conclusion));
				conclusionStack.pop_back();
			}

			OutputResults(clauseVariableIndex);
			bool diagnosisFound = DiagnosisFound(conclusionIndex);
			if(diagnosisFound)
			{
				string poison = GetSpecificPoison(conclusionIndex);
				PoisonDiagnosed.name = poison;
				DeepCopySymptoms(clauseVariableIndex, PoisonDiagnosed.symptoms);
				break;
			}
			else
			{
				string poison = GetSpecificPoison(conclusionIndex);
				cout << endl << "***** You have NOT been poisoned by: " << poison << " Poison *****"
					<< endl << "Searching for next potential poison...";
				fout << endl << "***** You have NOT been poisoned by: " << poison << " Poison *****"
					<< endl << "Searching for next potential poison...";
				// keep looking in the conclusion list
				conclusionIndex = IsInConclusionList(conclusionIndex + 1, conclusionRequest);
				if(conclusionIndex == -1)
				{
					cout << endl << "***** Diagnosis: None *****";
					fout << endl << "***** Diagnosis: None *****";
					break;
				}

				// these are the same
				clauseVariableIndex = conclusionIndex;

				// conclusion has been found...again  placing the FOUND conclusion onto conclusionStack
				conclusionStack.push_back(clauseVariableIndex);

				// set the current size of the conclusionStack
				sizeOfConclusionStack = conclusionStack.size();
			}
		}
	}

	// a poison has been diagnosed!!!
	if(PoisonDiagnosed.name != poisonNotDiagnosed)
	{
		cout << endl << " ****************************************************"
			<< endl << " ***** Diagnosis: " << PoisonDiagnosed.name << " Poison";

		fout << endl << " ****************************************************"
			<< endl << " ***** Diagnosis: " << PoisonDiagnosed.name << " Poison";
	}

	// the backward chaining algorithm has traversed the entire tree
	// and has NOT been able to diagnose a poison based on the symptoms
	else
	{
		cout << cout << endl << " ****************************************************"
			<< endl << " ***** Diagnosis: " << PoisonDiagnosed.name;
	}

	cout << endl << " ***** Symptoms: ";
	fout << endl << " ***** Symptoms: ";
	for(unsigned symptom = 0; symptom < PoisonDiagnosed.symptoms.size(); symptom++)
	{
		cout << endl << "                 " << symptom + 1 << ". " << PoisonDiagnosed.symptoms.at(symptom);
		fout << endl << "                 " << symptom + 1 << ". " << PoisonDiagnosed.symptoms.at(symptom);
	}
	cout << endl << " ****************************************************";
	fout << endl << " ****************************************************";
}

// --------------------------------------------------------------------------------------------------
int BackwardChaining::IsInConclusionList(const int initialIndex, const string& checkConclusion)
{
	for(unsigned conclusion = initialIndex; conclusion < conclusionList.size(); conclusion++)
	{
		if(checkConclusion == conclusionList.at(conclusion))
		{
			return conclusion;
		}
	}
	return -1;
}

// --------------------------------------------------------------------------------------------------
void BackwardChaining::ProcessStatementClauses(const int statementIndex)
{
	string localVariableName = "";
	int localVariableValue = -1;

	bool foundInConclusionList = false;
	bool askTheUser = false;
	bool isNot = false;

	// check all clauses
	for(unsigned clause = 0; clause < clauseVariableList.at(statementIndex).size(); clause++)
	{
		localVariableName = clauseVariableList.at(statementIndex).at(clause).clauseName;

		// remove the '!' to compare
		isNot = false;
		if(localVariableName.at(0) == '!')
		{
			cout << endl << " ----- INFO: Current variable has a \"!\"";
			fout << endl << " ----- INFO: Current variable has a \"!\"";
			isNot = true;
			string tempLocalVariableName = "";
			for(unsigned character = 1; character < localVariableName.size(); character++)
			{
				tempLocalVariableName = tempLocalVariableName + localVariableName.at(character);
			}
			localVariableName = tempLocalVariableName;
		}
		localVariableValue = clauseVariableList.at(statementIndex).at(clause).clauseValue;

		cout << "\n ***** Processing: clauseVariableList[" << poisonConclusionList.at(statementIndex) << "][" << localVariableName << "] = " << localVariableValue;
		fout << "\n ***** Processing: clauseVariableList[" << poisonConclusionList.at(statementIndex) << "][" << localVariableName << "] = " << localVariableValue;

		foundInConclusionList = false;
		askTheUser = false;

		// variable has not yet been updated (first check the variable list... if it is -1... then ask user)
		if(localVariableValue == -1)
		{
			// check to see if the clause is a conclusion
			for(unsigned conclusion = 0; conclusion < conclusionList.size(); conclusion++)
			{
				if(localVariableName == conclusionList.at(conclusion))
				{
					cout << endl << " ----- INFO: Located " << localVariableName << " in the conclusion list. (a non leaf conclusion)";
					fout << endl << " ----- INFO: Located " << localVariableName << " in the conclusion list. (a non leaf conclusion)";
					foundInConclusionList = true;
					int clauseVariableIndex = conclusion;

					// recursion --- is this better?
					//ProcessStatementClauses(clauseVariableIndex, 0);

					conclusionStack.push_back(clauseVariableIndex);
					break;
				}
			}
			// if the variable was found in the conclusion list... stop processing this variable, & move to the next one!
			if(foundInConclusionList)
			{
				continue;
			}

			// if the variable is NOT in the conclusion list... check to see if the user has already answered this symptom question
			if(!foundInConclusionList)
			{
				// check the variable list to see if it has already been found
				for(unsigned variable = 0; variable < variableList.size(); variable++)
				{
					// do the variable names match?
					if(localVariableName == variableList.at(variable).variableName)
					{
						cout << endl << " ----- INFO: Located " << localVariableName << "(" << variableList.at(variable).variableValue << ") in the variable list.";
						fout << endl << " ----- INFO: Located " << localVariableName << "(" << variableList.at(variable).variableValue << ") in the variable list.";
						// is the variable is NOT -1, then grab it!
						if(variableList.at(variable).variableValue != -1)
						{
							cout << endl << " ----- INFO: Copying from variable list";
							fout << endl << " ----- INFO: Copying from variable list";
							int updateVal = variableList.at(variable).variableValue;

							// deal with the '!' again...
							if(isNot)
							{
								if(updateVal == 0)
								{
									updateVal = 1;
								}
								else if(updateVal == 1)
								{
									updateVal = 0;
								}
								else
								{
									throw string("##### ERROR: Unknown value stored in the Backward Chaining variables list");
								}
							}
							clauseVariableList.at(statementIndex).at(clause).clauseValue = updateVal;
							break;
						}
						else
						{
							askTheUser = true;
						}
					}
				}
			}

			// need to ask the user for information on the symptom
			if(askTheUser)
			{
				cout << endl << "Does the patient have the following symptom?"
					<< endl << "   " << localVariableName
					<< endl
					<< endl << "  Yes"
					<< endl << "  No"
					<< endl
					<< endl << "Response: ";
				fout << endl << "Does the patient have the following symptom?"
					<< endl << "   " << localVariableName
					<< endl
					<< endl << "  Yes"
					<< endl << "  No"
					<< endl
					<< endl << "Response: ";

				string response;
				cin >> response;

				if(response != "Yes" && response != "No")
				{
					throw string("##### ERROR: Invalid response to symptom question (answer only \"Yes\" or \"No\")");
				}
				else
				{
					cout << endl << "\n You answered: \"" << response << "\"";
					fout << endl << "\n You answered: \"" << response << "\"";
					int updatedVariable = -1;
					if(response == "Yes")
					{
						updatedVariable = 1;
					}
					else if(response == "No")
					{
						updatedVariable = 0;
					}

					// need to update the variable list and the clause variable list
					for(unsigned variable = 0; variable < variableList.size(); variable++)
					{
						// do the variable names match?
						if(localVariableName == variableList.at(variable).variableName)
						{
							cout << endl << " ----- INFO: Updating variable list.";
							cout << endl << " ----- INFO: Updating variable list.";
							variableList.at(variable).variableValue = updatedVariable;

							// deal with the '!' again...
							if(isNot)
							{
								if(updatedVariable == 0)
								{
									updatedVariable = 1;
								}
								else if(updatedVariable == 1)
								{
									updatedVariable = 0;
								}
								else
								{
									throw string("##### ERROR: Unknown value stored in the variables list");
								}
							}

							clauseVariableList.at(statementIndex).at(clause).clauseValue = updatedVariable;
							break;
						}
					}
				}
			}
		}
	}
}

// --------------------------------------------------------------------------------------------------
void BackwardChaining::OutputResults(const int conclusion)
{
	cout << endl;
	fout << endl;
	for(unsigned variable = 0; variable < clauseVariableList.at(conclusion).size(); variable++)
	{
		cout << clauseVariableList.at(conclusion).at(variable).clauseName << ": " << clauseVariableList.at(conclusion).at(variable).clauseValue << endl;
		fout << clauseVariableList.at(conclusion).at(variable).clauseName << ": " << clauseVariableList.at(conclusion).at(variable).clauseValue << endl;
	}
}

// --------------------------------------------------------------------------------------------------
void BackwardChaining::PopulateNonLeafConclusion(const int statementIndex)
{
	// initialize
	bool nonLeafConclusionIs1 = true;
	int nonLeafConclusionValue = 1;

	for(unsigned variable = 0; variable < clauseVariableList.at(statementIndex).size(); variable++)
	{
		if(clauseVariableList.at(statementIndex).at(variable).clauseValue != 1)
		{
			nonLeafConclusionValue = 0;
			break;
		}
	}

	string localName = conclusionList.at(statementIndex);
	// update the clause variable list... for every the non leaf conclusion
	for(unsigned clause = 0; clause < clauseVariableList.size(); clause++)
	{
		for(unsigned variable = 0; variable < clauseVariableList.at(clause).size(); variable++)
		{
			if(localName == clauseVariableList.at(clause).at(variable).clauseName)
			{
				clauseVariableList.at(clause).at(variable).clauseValue = nonLeafConclusionValue;
			}
		}
	}
}

// --------------------------------------------------------------------------------------------------
bool BackwardChaining::DiagnosisFound(const int statementIndex)
{
	// initialize
	bool diagnosisFound = true;

	for(unsigned variable = 0; variable < clauseVariableList.at(statementIndex).size(); variable++)
	{
		//obtain the value
		int localValue = clauseVariableList.at(statementIndex).at(variable).clauseValue;
		if(localValue != 1)
		{
			diagnosisFound = false;
		}

		if(!diagnosisFound)
		{
			//we can quit checking since at least one variable has failed
			return false;
		}
	}

	return true;
}

// --------------------------------------------------------------------------------------------------
string BackwardChaining::GetSpecificPoison(const int specificPoisonIndex) const
{
	return poisonConclusionList.at(specificPoisonIndex);
}

// --------------------------------------------------------------------------------------------------
void BackwardChaining::DeepCopySymptoms(const int specificClauseIndex, vector<string>& poisonDiagnose) const
{
	for(unsigned variable = 0; variable < clauseVariableList.at(specificClauseIndex).size(); variable++)
	{
		poisonDiagnose.push_back(clauseVariableList.at(specificClauseIndex).at(variable).clauseName);
	}
}
