
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <ctype.h>
#include "XMLFix.h"

using std::cout;
using std::string;
using namespace std; //I'll fix this later

int main(int argc, char* argv[]){
	if(argc != 2)
	{
		cout<<"Please specify an xml file to parse.\n"
		"a file name <fileName>Fixed will be created.\n";
		return -1;
	}	
	//First get our file names	
	std::string oldFileName = argv[1];
	std::string newFileName = oldFileName;
	std::string line; //Each line of XML we parse
	int perLoc = oldFileName.find("."); //Location of period in the file name.
	if(perLoc != std::string::npos) //There is a period
	{//Add Fixed/NoCom before the extension
		newFileName.insert(perLoc, "Fixed");
	}
	else
	{//Add Fixed/NoCom at the new of the file
		newFileName = newFileName + "Fixed";	
	}

	//Then open the actual files. (converted to CStrings so fstream won't complain).
	ifstream readFile(oldFileName.c_str(), ios::in);
	if(!readFile.is_open())
	{//We check twice so that we don't create files if garbage is passed in.
		cout<<"Error opening files."<<endl;
		return -1;
	}
	ofstream writeFile(newFileName.c_str(), ios::out | ios::trunc);//Create or overwrite
	if(!writeFile.is_open())
	{
		cout<<"Error opening files."<<endl;
		return -1;
	}

	//Check for XML style
	if(getline(readFile, line))
	{
		if(trim(line).find("<?xml") == 0)
		{//If we start with an XML formatter
			//Write this line first.
			line = fixXMLTag(line);
			writeFile << line<<"\n";
		}
		else
		{
			cout<<"Improper XML file.\n";
			return -1;
		}
	}
	else
	{
		cout << "Error reading file\n";
		return -1;
	}
	//Then put the root element.
	writeFile << "<Root>\n";
	while(getline (readFile, line))
	{
		
		writeFile << parseLine(line)<<"\n";
	}
	writeFile << "</Root>\n";		
	readFile.close();
	writeFile.close();
	return 0;
}

//Read and fix any line of XML that is passed in,
//Returns that fixed line.
std::string parseLine(std::string line){
	line = fixOpenQuotes(line);//It's imperative that this run before
	//the copyright line, or not on it at all. Could update to ignore
	//Comments.
	line = fixInnerQuotes(line);
	line = fixCopyright(line); //Does this need to run EVERY line?
	return line;
}


//Removes copyright sign, as it is invalid
//Unicode that xmllint doesn't like.
//This probably doesn't need to run on every
//Line.
std::string fixCopyright(std::string line){
	//If this is the copyright line, remove the unicode.
	if(line.find("Copyright") != std::string::npos)
	{
		return "<!-- Copyright 2006 Obsidian Entertainment, Inc. -->";
	}
return line;
}

//Corrects improper opening XML tags
std::string fixXMLTag(std::string line){
	//Let's ensure we close this properly.
	if(line.find("<?xml") != string::npos)
	{
		line = trim(line);
		if(line.at(line.length()-2) != '?'){
			line.insert(line.length()-1,"?");
		}
	}
return line;
}

//Finds and escapes quotes in an element,
//Returns a fixed line.
//The only time we're seeing "false" quotes is
//In the context open("FooBar"), so that's the only
//Case we look for right now.
std::string fixInnerQuotes(std::string line){
	if(/*count(line.begin(), line.end(), '"')>2)//*/countOccurances(line, '"') > 2)
	{//We have more than 2 quotes in one line
		int firstQuotPos =line.find("\""); //The first quotation mark
		int lastQuotPos = line.find_last_of("\""); //The last quotation mark
		bool inPar = false;
		for(int i = firstQuotPos + 1; i < lastQuotPos-1; i++)
		{
			//We're in a parenthetical, all quotes need to be replaced
			//This is not covered by our previous cases if there are 
			//Multiple quoted entries in one set of parens.
			if(line.at(i) == '(')
			{
				inPar = true;
			}
			if(line.at(i) == ')')
			{
				inPar = false;
			}
			if(inPar && line.at(i) == '"')
			{			
				line.replace(i,1,"&quot;");
				lastQuotPos = line.find_last_of("\""); //Update string length
			}
			if(line.at(i) == '(' && line.at(i+1) == '"')
			{//Opening paren, encode the quote
				line.replace(i+1,1,"&quot;");	
				lastQuotPos = line.find_last_of("\""); //Our string changed, last quote should too.
			}
			//If we have a close paren or a comma [as in foo=("elem1",bar)]
			else if((line.at(i) == '"' && line.at(i+1) == ')') || (line.at(i) == '"' && line.at(i+1) == ','))
			{//Closed paren, encode the quote
				line.replace(i,1,"&quot;");
				lastQuotPos = line.find_last_of("\""); //Update string length
			}

		}
	}	
	return line;
}

//counts the number of times a character, find,
//Appears in a string, line, and returns that 
//Number.
int countOccurances(string line, char find){
	int count = 0;
	for (int i = 0; i < line.length(); i++)
	{
				
		if (line[i] == find)
		{
			count++;
		}
	}
	return count;
}

//Replace a substring with another substring.
//This isn't included in the STDLib for some reason.
//Returns origStr with every instance of oldText replaced with newText
std::string replaceString(std::string& origStr, std::string& oldText, std::string newText) {
    size_t pos = 0;
    while ((pos = origStr.find(oldText, pos)) != std::string::npos) {
         origStr.replace(pos, oldText.length(), newText);
         pos += newText.length();
    }
    return origStr;
}
//As above. Not sure which is in use.
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    std::string::size_type start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

//Find any element that has an equal sign not followed
//By a quotation mark. Insert that quotation mark,
//and return the fixed line.
std::string fixOpenQuotes(std::string line){
	//We have an equal with no open quote
	int end = line.length() -1;
	for(int i = 0; i < end; i++)
	{
		if(line.at(i) == '='&& line.at(i+1) != '"')
		{//Equal sign should be followed by a quote
			line.insert(i+1,"\"");
			i++;//Our string got longer.
			end++;
		}

		if(line.at(i) == '(' && line.at(i+1) != '"' && line.at(i+1) != ')')
		{//Open paren should be followed by a &quot; (or an immediate close paren)
		//But if we replace it directly here, it will be doubly escaped
		//Because we run fixInnerQuotes() next. The exception to this is if the
		//next string is Local, which we can check for with line.find(i+1, "Local", 5),
		//But let's try surrounding every comma in quotes first.
		//---------------------------
		//So advantage to local is that we don't add quotes around places they don't exist. (Local)
		//Advantage to comma is that we add it every time for consistency.


			line.insert(i+1, "\"");
		}

		if(i > 0 && line.at(i) == ')' && line.at(i-1) != '"' && line.at(i-1) != '(')// && isalpha(line.at(i-1))) //Only if "Local" method (see above)
		{//A closed quote should be preceeded by &quot; See above.
		//There are some exceptions to this, like when we have one quoted element
		//In a 2 element parenthesis set. This is always a number.
		//Or when we have () empty.

			line.insert(i,"\"");
		}
		if(i > 0 && line.at(i) == ',' && line.at(i-1) != '"')
		{//No quote before , add it in.
			line.insert(i, "\"");//I swear this is the most frequently typed line of code in this documents.
		}
		if(line.at(i) == ',' && line.at(i+1) != '"')
		{//No quote after a comma
			line.insert(i+1, "\""); //Followed by this one.
		}

		if(i < end -1 && line.at(i) == ')'&& line.at(i+2) != '\\')
		{//A close paren should be followed by a " or a space and a \>
			line.insert(i+1,"\"");
			i++;//Our string got longer.
			end++;
		}

	}
	//After all of this, if we can iterate through a string
	//And find a quote followed by a whitespace character, insert a quote.
	bool inQuote = false;
	end = line.length();
	for(int i = 0; i < end; i++)
	{
		if(!inQuote)
		{
			if(line[i] == '"')
			{
				inQuote = true;
			}
		}
		else
		{
			if(line[i] == '"')
			{//Inquote is true, we're in a quoted part.
				inQuote = false;
			}
			else if(isspace(line[i])) 
			{//We can't check for just a space, because someone went and
			 //Put newlines in these files.
				line.insert(i,"\"");
				i++;
				end++;
				inQuote = false;
			}	
		}
	}
	int closeBrace = line.find("/>");
	//If a close brace exists (not a comment), there isn't a close quote, AND we have an odd number of quotes.
	if(closeBrace != string::npos && 
					(line.at(closeBrace-1) != '\"' || line.at(closeBrace-2) != '\"') &&
					countOccurances(line, '"') % 2)//Sometimes there is a space after a quote
	{//We don't have a close quote before our close brace
		line.insert(closeBrace-1,"\"");
	}
	return line;
}
    
//Remove leading and trailing whitespace
std::string trim(std::string line){
	string whitespace = " \t\n\v\f\r";
	int lineBegin = line.find_first_not_of(whitespace);
	if (lineBegin == std::string::npos)
	        return ""; //empty string

	int lineEnd = line.find_last_not_of(whitespace);
	int lineRange = lineEnd - lineBegin + 1;
	return line.substr(lineBegin, lineRange);
}



