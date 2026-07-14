/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2015-2016 Erik Timmers <e.timmers@gmail.com>.
 * SPDX-FileCopyrightText: 2026 Elias Gheeraert <eliasgheeraert@gmail.com>.
 */

#include "BinImage.h"

#include <png.h>
#include <stdio.h>
#include <string.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#define PNG_DEBUG 3

BinImage::BinImage(char* data, size_t size)
    : _size(0), _width(0), _height(0), _data(nullptr), _g(0), _raw(nullptr) {
    // check if image data starts with "MotoRun"
    if (size < 7 || memcmp(data, "MotoRun", 7) != 0) {
        cerr << "Could not find valid image data" << endl;
        return;
    }

    _g = 8;  // skip "MotoRun "

    _data = new uint8_t[size];
    memcpy(_data, data, size);

    _size = size;

    _width = _read_dimension();
    _height = _read_dimension();

    if (!_width || !_height) {
        cerr << "Could not determine image size" << endl;
        return;
    }
}

BinImage::~BinImage() {
    delete[] _data;
    delete[] _raw;

    _data = nullptr;
    _raw = nullptr;
}

void BinImage::create_png(string filename) {
    cout << "Image size: " << _width << " x " << _height << endl;

    _decode();

    if (!_raw) {
        cerr << "Failed decoding binary image" << endl;
        return;
    }

    FILE* fp = fopen(filename.c_str(), "wb");

    if (!fp) {
        cerr << "Could not open file for writing PNG image" << endl;
        return;
    }

    png_structp pngStruct =
            png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!pngStruct) {
        fclose(fp);
        return;
    }

    png_infop pngInfo = png_create_info_struct(pngStruct);

    if (!pngInfo) {
        png_destroy_write_struct(&pngStruct, nullptr);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(pngStruct))) {
        png_destroy_write_struct(&pngStruct, &pngInfo);

        fclose(fp);
        return;
    }

    png_init_io(pngStruct, fp);

    png_set_IHDR(pngStruct, pngInfo, _width, _height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    vector<png_byte*> rows(_height);

    for (size_t i = 0; i < _height; i++) {
        rows[i] = &_raw[i * _width * 3];
    }

    png_set_rows(pngStruct, pngInfo, rows.data());

    png_write_png(pngStruct, pngInfo, PNG_TRANSFORM_BGR, nullptr);

    png_destroy_write_struct(&pngStruct, &pngInfo);

    fclose(fp);

    delete[] _raw;
    _raw = nullptr;
}

uint8_t* BinImage::get_data(void) {
    return _data;
}

size_t BinImage::get_size(void) {
    return _size;
}

bool BinImage::set_data_from_png(string filename) {
    FILE* fp = fopen(filename.c_str(), "rb");

    if (!fp) {
        cerr << "Failed opening '" << filename << "'" << endl;
        return false;
    }

    png_byte header[8];

    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8)) {
        cerr << "Not a PNG file" << endl;

        fclose(fp);
        return false;
    }

    png_structp pngStruct =
            png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!pngStruct) {
        fclose(fp);
        return false;
    }

    png_infop pngInfo = png_create_info_struct(pngStruct);

    if (!pngInfo) {
        png_destroy_read_struct(&pngStruct, nullptr, nullptr);

        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(pngStruct))) {
        png_destroy_read_struct(&pngStruct, &pngInfo, nullptr);

        fclose(fp);

        return false;
    }

    png_init_io(pngStruct, fp);

    png_set_sig_bytes(pngStruct, 8);

    png_read_info(pngStruct, pngInfo);

    unsigned int width, height;

    int depth;
    int type;
    int interlaceType;
    int compressionType;
    int filterMethod;

    png_get_IHDR(pngStruct, pngInfo, &width, &height, &depth, &type, &interlaceType,
                 &compressionType, &filterMethod);

    if (width != _width || height != _height) {
        cerr << "PNG dimensions mismatch" << endl;

        return false;
    }

    if (type == PNG_COLOR_TYPE_RGB_ALPHA) {
        png_set_strip_alpha(pngStruct);

    } else if (type == PNG_COLOR_TYPE_RGB) {
        png_set_bgr(pngStruct);

    } else {
        cerr << "Unsupported PNG type" << endl;

        return false;
    }

    png_read_update_info(pngStruct, pngInfo);

    _raw = new uint8_t[3 * width * height];

    vector<png_bytep> rows(height);

    for (size_t i = 0; i < height; i++) {
        rows[i] = &_raw[i * 3 * width];
    }

    png_read_image(pngStruct, rows.data());

    png_destroy_read_struct(&pngStruct, &pngInfo, nullptr);

    fclose(fp);

    return _encode();
}

bool BinImage::_encode(void) {
    if (!_raw) {
        return false;
    }

    string prefix("MotoRun");

    vector<uint8_t> data(prefix.begin(), prefix.end());

    data.push_back(0x00);

    data.push_back((_width >> 8) & 0xff);
    data.push_back(_width & 0xff);

    data.push_back((_height >> 8) & 0xff);
    data.push_back(_height & 0xff);

    uint8_t mode;

    for (size_t i = 0, L, length; i < _width * _height; i += length) {
        length = _block_length(true, i);

        if (length >= 3) {
            mode = 0x80;
            L = 1;

        } else {
            length = _block_length(false, i);

            mode = 0x00;
            L = length;
        }

        data.push_back(mode | ((length >> 8) & 0x0f));

        data.push_back(length & 0xff);

        for (size_t l = 1; l <= L; l++) {
            size_t j = 3 * (i + l);

            data.push_back(_raw[j]);
            data.push_back(_raw[j + 1]);
            data.push_back(_raw[j + 2]);
        }
    }

    delete[] _data;

    _size = data.size();

    _data = new uint8_t[_size];

    memcpy(_data, data.data(), _size);

    return true;
}

size_t BinImage::_block_length(bool repeat, size_t g) {
    size_t max = _width - (g % _width);
    size_t l;
    bool warning = false;

    for (l = 1; l < max; ++l) {
        size_t j = 3 * (g + l);

        bool equal = true;

        for (size_t k = 0; k < 3; ++k) {
            if (_raw[j + k] != _raw[j + 3 + k]) {
                equal = false;
                break;
            }
        }

        if (equal) {
            if (!repeat) {
                if (warning) {
                    return l - 2;
                } else {
                    warning = true;
                }
            }
        } else {
            if (repeat) {
                return l;
            } else {
                warning = false;
            }
        }
    }

    return l;
}

void BinImage::_decode(void) {
    _raw = new uint8_t[3 * _width * _height];

    size_t g = 0;

    while (_g < _size) {
        uint8_t byte = _data[_g++];

        if ((byte & 0x70) > 0) {
            cerr << "Invalid nybble (" << (int)byte << ") at position " << _g << endl;

            delete[] _raw;
            _raw = nullptr;
            return;

        } else {
            uint16_t runlength = 0;

            runlength += (byte & 0x0f) << 8;
            runlength += _data[_g++];

            bool repeat = byte >> 7;

            for (uint16_t i = 0; i < runlength; ++i) {
                memcpy(&_raw[g], &_data[_g], 3);

                g += 3;

                if (!repeat) {
                    _g += 3;
                }
            }

            if (repeat) {
                _g += 3;
            }
        }
    }

    if (_g != _size) {
        cerr << "Incorrect file size (" << _g << "/" << _size << ")" << endl;

        delete[] _raw;
        _raw = nullptr;
    }
}

size_t BinImage::_read_dimension(void) {
    uint8_t bytes[2];

    bytes[0] = _data[_g++];
    bytes[1] = _data[_g++];

    return ((size_t)bytes[0] << 8) | bytes[1];
}
