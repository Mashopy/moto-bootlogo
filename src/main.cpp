/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2015-2016 Erik Timmers <e.timmers@gmail.com>.
 * SPDX-FileCopyrightText: 2026 Elias Gheeraert <eliasgheeraert@gmail.com>.
 */

#include <unistd.h>

#include <fstream>
#include <iostream>

#include "BinFile.h"

using namespace std;

void print_usage(void) {
    cout << "Usage: moto-bootlogo <logo.bin>" << endl;
}

int main(int argc, char* argv[]) {
    // require at least 2 arguments
    if (argc < 2) {
        print_usage();
        return 1;
    }

    string filename = argv[argc - 1];

    // create BinFile
    BinFile* bin = new BinFile(filename);

    // parse (and thus verify) BinFile
    map<string, BinHeader*> headers = bin->get_headers();
    int size = headers.size();
    cout << "Found " << size << " image" << (size == 1 ? "" : "s") << "." << endl;

    // create PNG images
    for (map<string, BinHeader*>::const_iterator it = headers.begin(), end = headers.end();
         it != end; ++it) {
        string tag = it->first;
        BinHeader* header = it->second;
        BinImage* image = header->get_image();
        image->create_png(tag.append(".png"));
    }

    // exit
    delete bin;
    return 0;
}
