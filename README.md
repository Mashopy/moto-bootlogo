# Motorola / Lenovo boot logo unpack

>[!CAUTION]
> **The logo encoding and decoding are based on the tool by [@eriktim](https://github.com/eriktim/moto-bootlogo) and script by [__carock__](http://forum.xda-developers.com/showpost.php?p=48891456&postcount=140).**
> **This tool is only for unpacking the .png inside `logo.bin` image !**

## Dependencies

* A `logo.bin` image from Motorola / Lenovo Hello UI.
* `libpng` package installed.

## Build

```console
Mashopy@HollowBastion:~/moto-bootlogo$ make
g++ -Wall -c -g -I/usr/include/libpng16  -o build/main.o src/main.cpp
g++ -Wall -c -g -I/usr/include/libpng16  -o build/BinFile.o src/BinFile.cpp
g++ -Wall -c -g -I/usr/include/libpng16  -o build/BinHeader.o src/BinHeader.cpp
g++ -Wall -c -g -I/usr/include/libpng16  -o build/BinImage.o src/BinImage.cpp
g++ -Wall -g build/main.o build/BinFile.o build/BinHeader.o build/BinImage.o -lpng16  -o build/moto-bootlogo
```

## Usage example
> **This tool has been tested with the `logo.bin` image from the Motorola Edge 70 Max / Lenovo Legion Y70 (New generation).**

```console
Mashopy@HollowBastion:~/moto-bootlogo$ build/moto-bootlogo logo.bin
Parsing header of 'logo.bin'.
Header size: 173 bytes.
Header logo: 'logo_boot', starts at byte 1024 and occupies 9951372 bytes.
Header logo: 'logo_battery', starts at byte 9952768 and occupies 61675 bytes.
Header logo: 'logo_lowpower', starts at byte 10014720 and occupies 1853800 bytes.
Header logo: 'logo_charge', starts at byte 11868672 and occupies 2164023 bytes.
Header logo: 'switchconsole', starts at byte 14032896 and occupies 44588 bytes.
Found 5 images.
Image size: 626 x 626
Image size: 1440 x 3168
Image size: 1440 x 3168
Image size: 1440 x 3168
Image size: 610 x 135
```

## License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**.

Key points to be aware of:

* You are free to use, modify, and distribute the software.
* If you modify and use the software publicly, you must release your source code.
* You must retain the same license (`AGPL-3.0`) when redistributing modified versions.
* You cannot keep modifications private if the software is used to provide a networked service.

For full details, please refer to the [LICENSE](https://github.com/R0rt1z2/fenrir/tree/master/LICENSE) file.
