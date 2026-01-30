#pragma once

struct CuentaArch{
    char desc[128];
    char bloque[1408]; // 128 * 3 + 1024
    char tag_desc[16];
    char tag_bloque[16];
    char iv_desc[12];
    char iv_bloque[12];

    ~CuentaArch() noexcept;
};
