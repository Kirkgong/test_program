#ifndef TSFORMAT_H
#define TSFORMAT_H
#include <vector>
#include "string.h"
#include "stdint.h"
#include "stdio.h"

#define NOPTS_VALUE 0
#define TS_STREAM_PACKET_LEN 188

typedef void (*FRAME_READY_CALLBACK)(void *arg);

typedef struct TS_header
{
    unsigned sync_byte : 8;                    // 同步字节，固定为0x47 ，表示后面的是一个TS分组，当然，后面包中的数据是不会出现0x47的
    unsigned transport_error_indicator : 1;    // 传输错误标志位，一般传输错误的话就不会处理这个包了
    unsigned payload_unit_start_indicator : 1; // 有效负载的开始标志，根据后面有效负载的内容不同功能也不同
    // payload_unit_start_indicator为1时，在前4个字节之后会有一个调整字节，它的数值决定了负载内容的具体开始位置。
    unsigned transport_priority : 1;           // 传输优先级位，1表示高优先级
    unsigned PID : 13;                         // 有效负载数据的类型
    unsigned transport_scrambling_control : 2; // 加密标志位,00表示未加密
    unsigned adaption_field_control : 2;       // 调整字段控制,。01仅含有效负载，10仅含调整字段，11含有调整字段和有效负载。为00的话解码器不进行处理。
    unsigned continuity_counter : 4;           // 一个4bit的计数器，范围0-15
} TS_header;

typedef struct TS_PAT_Program
{
    unsigned program_number : 16; // 节目号
    unsigned reserved_3 : 3;
    unsigned program_map_PID : 13; // 节目映射表的PID，节目号大于0时对应的PID，每个节目对应一个
} TS_PAT_Program;

typedef struct PAT_Packet_tag
{
    unsigned table_id : 8;                 // 固定为0x00 ，标志是该表是PAT
    unsigned section_syntax_indicator : 1; // 段语法标志位，固定为1
    unsigned zero : 1;                     // 0
    unsigned reserved_1 : 2;               // 保留位
    unsigned section_length : 12;          // 表示有用的字节数，包括CRC32
    unsigned transport_stream_id : 16;     // 该传输流的ID，区别于一个网络中其它多路复用的流
    unsigned reserved_2 : 2;               // 保留位
    unsigned version_number : 5;           // 范围0-31，表示PAT的版本号
    unsigned current_next_indicator : 1;   // 发送的PAT是当前有效还是下一个PAT有效
    unsigned section_number : 8;           // 分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
    unsigned last_section_number : 8;      // 最后一个分段的号码
    std::vector<TS_PAT_Program> program;
    unsigned CRC_32 : 32;
} PAT_Packet;

typedef struct TS_PMT_Stream
{
    unsigned stream_type : 8; // 指示特定PID的节目元素包的类型。该处PID由elementary PID指定
    unsigned reserved_5 : 3;
    unsigned elementary_PID : 13; // 该域指示TS包的PID值。这些TS包含有相关的节目元素
    unsigned reserved_6 : 4;
    unsigned ES_info_length : 12; // 前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
    unsigned descriptor;
} TS_PMT_Stream;

// Program Map Table
typedef struct PMT_Packet_tag
{
    unsigned table_id : 8;
    unsigned section_syntax_indicator : 1;
    unsigned zero : 1;
    unsigned reserved_1 : 2;
    unsigned section_length : 12;
    unsigned program_number : 16;
    unsigned reserved_2 : 2;
    unsigned version_number : 5;
    unsigned current_next_indicator : 1;
    unsigned section_number : 8;
    unsigned last_section_number : 8;
    unsigned reserved_3 : 3;
    unsigned PCR_PID : 13;
    unsigned reserved_4 : 4;
    unsigned program_info_length : 12;
    std::vector<TS_PMT_Stream> PMT_Stream;
    unsigned CRC_32 : 32;
} PMT_Packet;

typedef struct PES_Packet
{
    uint32_t packet_start_code_prefix;
    uint8_t stream_id;
    uint32_t packet_length;
    uint8_t reserved_0;
    uint8_t scrambling_control;
    uint8_t priority;
    uint8_t data_alignment_indicator;
    uint8_t copyright;
    uint8_t original_or_copy;
    uint8_t pts_dts_flags;
    uint8_t escr_flag;
    uint8_t es_rate_flag;
    uint8_t dsm_trick_mode_flag;
    uint8_t additional_copy_info_flag;
    uint8_t crc_flag;
    uint8_t extension_flag;
    uint8_t header_data_length;
    uint8_t playload_offset;
    uint32_t pts;
    uint32_t dts;
} PES_Packet;

typedef struct Frame_Struct
{
    uint8_t *buf;
    uint8_t stream_type;
    uint32_t frame_len;
    uint32_t pts;
    uint32_t dts;
    void (*frame_ready_callback)(void *arg);
    void *Private;
} Frame_Struct;

class TSFormat
{
private:
    /* data */
    PAT_Packet pat_packet;
    PMT_Packet pmt_packet;
    PES_Packet pes_packet;
    Frame_Struct frame;

public:
    TSFormat(/* args */);
    ~TSFormat();
    int Parse_TS_header(unsigned char *pTSBuf, TS_header *pheader);
    int Parse_PAT(unsigned char *pTSBuf, TS_header *TSheader);
    int Parse_PMT(unsigned char *pTSBuf, TS_header *TSheader);
    int Parse_PES(unsigned char *pTSBuf, TS_header *TSheader);
    bool pat_element_exist(std::vector<TS_PAT_Program> &program, unsigned target);
    bool pmt_element_exist(std::vector<TS_PMT_Stream> &stream, unsigned target);
    int Parse_Packet(uint8_t *buf, void *parent);
    void set_frame_ready_callback(FRAME_READY_CALLBACK callback);
    void clear_pmt(void);
    void clear_len(void);
};

#endif
