/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2015-2016 Erik Timmers <e.timmers@gmail.com>.
 * SPDX-FileCopyrightText: 2026 Elias Gheeraert <eliasgheeraert@gmail.com>.
 */

#include "BinFile.h"

#include <stdint.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <vector>

BinFile::BinFile(string filename) {
    _filename = filename;
    _count = -1;
}

BinFile::~BinFile() {
    if (_file.is_open()) {
        _file.close();
    }

    for (map<string, BinHeader*>::const_iterator it = _headers.begin(), end = _headers.end();
         it != end; it++) {
        BinHeader* header = it->second;
        delete header;
    }
}

bool BinFile::copy_image_header(string fromTag, string toTag) {
    if (_count < 0) {
        _parse_header();
    }

    BinHeader* from = _headers[fromTag];
    if (!from) {
        cerr << "Image header not found: " << fromTag << endl;
        return false;
    }

    BinHeader* to = _headers[toTag];
    if (!to) {
        cerr << "Image header not found: " << toTag << endl;
        return false;
    }

    size_t offset = from->get_offset();
    size_t length = from->get_size();
    size_t g = to->get_g();

    _file.close();
    _file.open(_filename.c_str(), ios::in | ios::out | ios::binary);

    _file.seekg(g + 24, ios::beg);

    char data[8];

    for (int i = 0; i < 4; ++i) {
        data[i] = (offset >> (8 * i)) & 0xff;
    }

    for (int i = 0; i < 4; ++i) {
        data[4 + i] = (length >> (8 * i)) & 0xff;
    }

    _file.write(data, 8);
    _file.close();

    _count = -1;

    return true;
}

map<string, BinHeader*> BinFile::get_headers(void) {
    if (_count < 0) {
        _parse_header();
    }

    return _headers;
}

bool BinFile::replace_image(string tag, string filename) {
    if (_count < 0) {
        _parse_header();
    }

    BinHeader* header = _headers[tag];

    if (!header) {
        cerr << "Image header not found: " << tag << endl;
        return false;
    }

    BinImage* image = header->get_image();

    if (!image->set_data_from_png(filename)) {
        cerr << "Failed reading image data from '" << filename << "'" << endl;
        return false;
    }

    if (_file.is_open()) {
        _file.close();
    }

    _file.open(_filename.c_str(), ios::in | ios::out | ios::binary);

    if (_file.fail()) {
        cerr << "Failed opening '" << _filename << "' for writing" << endl;
        return false;
    }

    size_t length = image->get_size();

    if (length > header->get_size()) {
        cerr << "Resulting image is too large (" << length << " of " << header->get_size()
             << " bytes)" << endl;
        return false;
    }

    header->update_size();

    char size[4];

    size[0] = length & 0xff;
    size[1] = (length >> 8) & 0xff;
    size[2] = (length >> 16) & 0xff;
    size[3] = (length >> 24) & 0xff;

    _file.seekg(header->get_g() + 28, ios::beg);
    _file.write(size, 4);

    uint8_t* data = image->get_data();

    _file.seekg(header->get_offset(), ios::beg);
    _file.write((char*)data, length);

    _count = -1;

    return true;
}

size_t BinFile::_read_value(void) {
    char bytes[4];

    _file.read(bytes, 4);

    size_t value = 0;

    for (int i = 3; i >= 0; i--) {
        value += (uint8_t)bytes[i] << (8 * i);
    }

    return value;
}

void BinFile::_parse_header(void) {
    _headers.clear();

    if (_file.is_open()) {
        _file.close();
    }

    _file.open(_filename.c_str(), ios::in | ios::binary);

    if (_file.fail()) {
        cerr << "Failed opening file '" << _filename << "'." << endl;
        return;
    }

    char bytes[24];

    _file.seekg(0, ios::beg);

    cout << "Parsing header of '" << _filename << "'." << endl;

    _file.read(bytes, 9);

    if (_file.fail() || memcmp(bytes, "MotoLogo", 8)) {
        cerr << "Header is NOT valid" << endl;
        return;
    }

    _file.read(bytes, 2);

    uint8_t size = (uint8_t)bytes[0];

    if (_file.fail() || !size) {
        cerr << "Header is empty" << endl;
        return;
    }

    cout << "Header size: " << (int)size << " bytes." << endl;

    _file.seekg(2, ios::cur);

    while (_file.tellg() < size) {
        _file.read(bytes, 24);

        string tag(bytes, 24);
        tag.erase(tag.find('\0'));

        size_t offset = _read_value();
        size_t length = _read_value();

        cout << "Header logo: '" << tag << "', starts at byte " << offset << " and occupies "
             << length << " bytes." << endl;

        if (tag.empty() || !offset || !length) {
            cerr << "Found an invalid image header" << endl;
            continue;
        }

        vector<char> data(length);

        streampos g = _file.tellg();

        _file.seekg(offset, ios::beg);

        _file.read(data.data(), length);

        _file.seekg(g, ios::beg);

        BinImage* image = new BinImage(data.data(), length);

        BinHeader* header = new BinHeader(offset, length, (size_t)g - 32, image);

        _headers.insert(pair<string, BinHeader*>(tag, header));
    }

    _count = _headers.size();
}
