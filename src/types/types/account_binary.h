#pragma once

struct AccountBinary{
        // --- crypto parameters ---
    static constexpr std::size_t kGcmIvSizeBytes      = 12;
    static constexpr std::size_t kGcmTagSizeBytes     = 16;

    // --- logical field sizes ---
    static constexpr std::size_t kDescriptionSize     = 128;
    static constexpr std::size_t kEmailSize           = 128;
    static constexpr std::size_t kUsernameSize        = 128;
    static constexpr std::size_t kPasswordSize        = 128;
    static constexpr std::size_t kExtraSize            = 1024;

    // --- layout ---
    static constexpr std::size_t kDataSize =
          kEmailSize
        + kUsernameSize
        + kPasswordSize
        + kExtraSize;

    char description[kDescriptionSize];
    char data[kDataSize];
    char iv_description[kGcmIvSizeBytes];
    char iv_data[kGcmIvSizeBytes];
    char tag_description[kGcmTagSizeBytes];
    char tag_data[kGcmTagSizeBytes];

    void SetZero() noexcept;
    ~AccountBinary() noexcept;
};
