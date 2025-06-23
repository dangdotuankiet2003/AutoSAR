#ifndef STD_TYPES_H
#define STD_TYPES_H
/**
 * @brief: Định nghĩa trạng thái logic cao và thấp
 * @details: Dùng cho tín hiệu đầu vào/ra
 */
#define STD_HIGH 0x01U
#define STD_LOW 0x00U

#define NULL_PTR ((void* )0)
#define null 0

// Số nguyên không dấu 8-16-32 bit
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

/**
 * @typedef: Std_VersionInfoType
 * @brief: Cấu trúc thông tin phiên bản
 * @details: Lưu trữ thông tin về phiên bản của module
 */

typedef struct{
    uint16 vendorID;        /**< @kiểu dữ liệu: uint16, ID của nhà cung cấp */
    uint16 moduleID;        /**< @kiểu dữ liệu: uint16, ID của module */
    uint8 sw_major_version; /**< @kiểu dữ liệu: uint8, Phiên bản chính của phần mềm */
    uint8 sw_minor_version; /**< @kiểu dữ liệu: uint8, Phiên bản phụ của phần mềm */
    uint8 sw_patch_version; /**< @kiểu dữ liệu: uint8, Phiên bản vá của phần mềm */
} Std_VersionInfoType;


#endif //STD_TYPES_H