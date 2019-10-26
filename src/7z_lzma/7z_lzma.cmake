# Copyright (c) 2019 Kavawuvi
#
# This code is licensed under the GNU GPL v3.0 or any later version
# See LICENSE for more information.

# LZMA SDK was retrieved from https://www.7-zip.org/sdk.html

# Define the source files
add_library(7z_lzma STATIC
    src/7z_lzma/Ppmd7Enc.c
    src/7z_lzma/Ppmd7Dec.c
    src/7z_lzma/AesOpt.c
    src/7z_lzma/LzmaDec.c
    src/7z_lzma/7zAlloc.c
    src/7z_lzma/Bra86.c
    src/7z_lzma/Sort.c
    src/7z_lzma/Bcj2Enc.c
    src/7z_lzma/Lzma2Enc.c
    src/7z_lzma/7zBuf.c
    src/7z_lzma/XzCrc64Opt.c
    src/7z_lzma/Bcj2.c
    src/7z_lzma/Aes.c
    src/7z_lzma/7zArcIn.c
    src/7z_lzma/Lzma2Dec.c
    src/7z_lzma/XzCrc64.c
    src/7z_lzma/7zCrcOpt.c
    src/7z_lzma/7zCrc.c
    src/7z_lzma/Lzma86Dec.c
    src/7z_lzma/Alloc.c
    src/7z_lzma/LzFind.c
    src/7z_lzma/XzEnc.c
    src/7z_lzma/7zDec.c
    src/7z_lzma/Bra.c
    src/7z_lzma/Threads.c
    src/7z_lzma/MtDec.c
    src/7z_lzma/7zFile.c
    src/7z_lzma/BraIA64.c
    src/7z_lzma/Ppmd7.c
    src/7z_lzma/Lzma86Enc.c
    src/7z_lzma/XzIn.c
    src/7z_lzma/XzDec.c
    src/7z_lzma/LzmaLib.c
    src/7z_lzma/Xz.c
    src/7z_lzma/Sha256.c
    src/7z_lzma/Delta.c
    src/7z_lzma/MtCoder.c
    src/7z_lzma/LzFindMt.c
    src/7z_lzma/DllSecur.c
    src/7z_lzma/7zStream.c
    src/7z_lzma/Lzma2DecMt.c
    src/7z_lzma/LzmaEnc.c
    src/7z_lzma/CpuArch.c
    src/7z_lzma/7zBuf2.c
)
