/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2015-2016 Erik Timmers <e.timmers@gmail.com>.
 * SPDX-FileCopyrightText: 2026 Elias Gheeraert <eliasgheeraert@gmail.com>.
 */

#ifndef BINIMAGE_H
#define BINIMAGE_H

#include <stddef.h>
#include <stdint.h>

#include <string>

using namespace std;

class BinImage {
  public:
    BinImage(char* data, size_t size);
    ~BinImage();

    void create_png(std::string filename);
    uint8_t* get_data(void);
    size_t get_size(void);
    bool set_data_from_png(std::string filename);

  private:
    size_t _block_length(bool repeat, size_t g);
    void _decode(void);
    bool _encode(void);
    size_t _read_dimension(void);

    size_t _size = 0;
    size_t _width = 0;
    size_t _height = 0;
    uint8_t* _data = nullptr;
    size_t _g = 0;
    uint8_t* _raw = nullptr;
};

#endif
