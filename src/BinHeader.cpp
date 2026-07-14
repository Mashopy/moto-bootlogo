/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2015-2016 Erik Timmers <e.timmers@gmail.com>.
 * SPDX-FileCopyrightText: 2026 Elias Gheeraert <eliasgheeraert@gmail.com>.
 */

#include "BinHeader.h"

BinHeader::BinHeader(size_t offset, size_t size, size_t g, BinImage* image) {
    _offset = offset;
    _size = size;
    _g = g;
    _image = image;
}

BinHeader::~BinHeader() {
    if (_image) {
        delete _image;
        _image = NULL;
    }
}

BinImage* BinHeader::get_image(void) {
    return _image;
}

size_t BinHeader::get_g(void) {
    return _g;
}

size_t BinHeader::get_offset(void) {
    return _offset;
}

size_t BinHeader::get_size(void) {
    return _size;
}

void BinHeader::update_size(void) {
    _size = _image->get_size();
}
