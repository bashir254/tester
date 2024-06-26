/**
 * SPDX-FileCopyrightText: (C) 2009 Dominik Seichter <domseichter@web.de>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <podofo/podofo.h>
#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

#include "DeleteOperation.h"
#include "MoveOperation.h"

using namespace std;
using namespace PoDoFo;

class BadConversion : public runtime_error {
public:
    BadConversion(const string& s)
        : runtime_error(s)
    { }
};

void print_help()
{
    printf("Usage: podofopages [inputfile] [outputfile]\n");
    printf("Options:\n");
    printf("\t--delete NUMBER\n");
    printf("\tDeletes the page NUMBER (number is 0-based)\n");
    printf("\tThe page will not really be deleted from the PDF.\n");
    printf("\tIt is only removed from the so called pagestree and\n");
    printf("\ttherefore invisible. The content of the page can still\n");
    printf("\tbe retrieved from the document though.\n\n");
    printf("\t--move FROM TO\n");
    printf("\tMoves a page FROM TO in the document (FROM and TO are 0-based)\n\n");
    printf("\nPoDoFo Version: %s\n\n", PODOFO_VERSION_STRING);
}

void work(const string_view& inputPath, const string_view& outputPath, const vector<Operation*>& operations)
{
    cout << "Input file: " << inputPath << endl;
    cout << "Output file: " << outputPath << endl;

    PdfMemDocument doc;
    doc.Load(inputPath);

    unsigned total = (unsigned)operations.size();
    unsigned i = 1;
    for (auto operation : operations)
    {
        string msg = operation->ToString();
        cout << "Operation " << i << " of " << total << ": " << msg;

        operation->Perform(doc);

        i++;
    }

    cout << "Operations done. Writing PDF to disk." << endl;

    doc.Save(outputPath);

    cout << "Done." << endl;
}

double convertToInt(const string& s)
{
    istringstream i(s);
    int x;
    if (!(i >> x))
        throw BadConversion("convertToInt(\"" + s + "\")");
    return x;
}

int main(int argc, char* argv[])
{
    char* inputPath = NULL;
    char* outputPath = NULL;

    if (argc < 3)
    {
        print_help();
        exit(-1);
    }

    // Fill operations vector
    vector<Operation*> operations;
    for (int i = 1; i < argc; i++)
    {
        string argument = argv[i];
        if (argument == "--delete" || argument == "-delete")
        {
            int page = static_cast<int>(convertToInt(string(argv[i + 1])));
            operations.push_back(new DeleteOperation(page));
            i++;
        }
        else if (argument == "--move" || argument == "-move")
        {
            int from = static_cast<int>(convertToInt(string(argv[i + 1])));
            int to = static_cast<int>(convertToInt(string(argv[i + 2])));
            operations.push_back(new MoveOperation(from, to));
            i++;
            i++;
        }
        else
        {
            if (inputPath == NULL)
            {
                inputPath = argv[i];
            }
            else if (outputPath == NULL)
            {
                outputPath = argv[i];
            }
            else
            {
                cerr << "Ignoring unknown argument: " << argument << endl;
            }
        }
    }

    if (!inputPath)
    {
        cerr << "Please specify an input file." << endl;
        exit(-2);
    }

    if (!outputPath)
    {
        cerr << "Please specify an output file." << endl;
        exit(-3);
    }

    if (string(inputPath) == string(outputPath))
    {
        cerr << "Input and outpuf file must point to different files." << endl;
        exit(-4);
    }

    try
    {
        work(inputPath, outputPath, operations);
    }
    catch (PdfError& e)
    {
        cerr << "Error: An error " << (int)e.GetCode() << " ocurred." << endl;
        e.PrintErrorMsg();
        return (int)e.GetCode();
    }
    catch (runtime_error& re)
    {
        cerr << "Error: An error " << re.what() << " ocurred." << endl;
        return -1;
    }

    // Delete operations vectore
    vector<Operation*>::iterator it = operations.begin();
    while (it != operations.end())
    {
        delete (*it);
        it++;
    }

    return 0;
}

