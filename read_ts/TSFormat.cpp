#include "TSFormat.h"
#include "stdlib.h"

TSFormat::TSFormat(/* args */)
{
    frame.buf = (uint8_t *)malloc(2 * 1024 * 1024);
}

TSFormat::~TSFormat()
{
    pat_packet.program.clear();
    pmt_packet.PMT_Stream.clear();
    free(frame.buf);
}

bool TSFormat::pat_element_exist(std::vector<TS_PAT_Program> &program, unsigned target)
{
    for (auto &item : program)
    {
        if (item.program_map_PID == target)
        {
            return true;
        }
    }
    return false;
}

bool TSFormat::pmt_element_exist(std::vector<TS_PMT_Stream> &stream, unsigned target)
{
    for (auto &item : stream)
    {
        if (item.elementary_PID == target)
        {
            return true;
        }
    }
    return false;
}

int TSFormat::Parse_TS_header(unsigned char *pTSBuf, TS_header *pheader)
{
    pheader->sync_byte = pTSBuf[0];
    if (pheader->sync_byte != 0x47)
        return -1;
    pheader->transport_error_indicator = pTSBuf[1] >> 7;
    pheader->payload_unit_start_indicator = pTSBuf[1] >> 6 & 0x01;
    pheader->transport_priority = pTSBuf[1] >> 5 & 0x01;
    pheader->PID = (pTSBuf[1] & 0x1F) << 8 | pTSBuf[2];
    pheader->transport_scrambling_control = pTSBuf[3] >> 6;
    pheader->adaption_field_control = pTSBuf[3] >> 4 & 0x03;
    pheader->continuity_counter = pTSBuf[3] & 0x0F;
    return 0;
}

// Parse PAT
int TSFormat::Parse_PAT(unsigned char *pTSBuf, TS_header *TSheader)
{
    if (TSheader->payload_unit_start_indicator == 0x01) // 表示含有PSI或者PES头
    {
        if (TSheader->PID == 0x0) // 表示PAT
        {
            int iBeginlen = 4;
            int adaptation_field_length = pTSBuf[4];
            switch (TSheader->adaption_field_control)
            {
            case 0x0: // reserved for future use by ISO/IEC
                return -1;
            case 0x1:                               // 无调整字段，仅含有效负载
                iBeginlen += pTSBuf[iBeginlen] + 1; // + pointer_field
                break;
            case 0x2: // 仅含调整字段，无有效负载
                return -1;
            case 0x3: // 调整字段后含有效负载
                if (adaptation_field_length > 0)
                {
                    iBeginlen += 1;                       // adaptation_field_length占8位
                    iBeginlen += adaptation_field_length; // + adaptation_field_length
                }
                else
                {
                    iBeginlen += 1; // adaptation_field_length占8位
                }
                iBeginlen += pTSBuf[iBeginlen] + 1; // + pointer_field
                break;
            default:
                break;
            }
            unsigned char *pPAT = pTSBuf + iBeginlen;
            pat_packet.table_id = pPAT[0];
            pat_packet.section_syntax_indicator = pPAT[1] >> 7;
            pat_packet.zero = pPAT[1] >> 6 & 0x1;
            pat_packet.reserved_1 = pPAT[1] >> 4 & 0x3;
            pat_packet.section_length = (pPAT[1] & 0x0F) << 8 | pPAT[2];
            pat_packet.transport_stream_id = pPAT[3] << 8 | pPAT[4];
            pat_packet.reserved_2 = pPAT[5] >> 6;
            pat_packet.version_number = pPAT[5] >> 1 & 0x1F;
            pat_packet.current_next_indicator = (pPAT[5] << 7) >> 7;
            pat_packet.section_number = pPAT[6];
            pat_packet.last_section_number = pPAT[7];
            int len = 0;
            len = 3 + pat_packet.section_length;
            pat_packet.CRC_32 = (pPAT[len - 4] & 0x000000FF) << 24 | (pPAT[len - 3] & 0x000000FF) << 16 |
                                (pPAT[len - 2] & 0x000000FF) << 8 | (pPAT[len - 1] & 0x000000FF);

            int n = 0;
            for (n = 0; n < (pat_packet.section_length - 12); n += 4)
            {
                TS_PAT_Program pat_program;
                pat_program.program_number = pPAT[8 + n] << 8 | pPAT[9 + n];
                pat_program.reserved_3 = pPAT[10 + n] >> 5;
                if (pat_program.program_number == 0x00)
                {
                    // pat_packet.network_PID = (pPAT[10 + n] & 0x1F) << 8 | pPAT[11 + n];
                    printf("pat_packet.network_PID\n");
                }
                else
                {
                    // 有效的PMT的PID,然后通过这个PID值去查找PMT包
                    pat_program.program_map_PID = (pPAT[10 + n] & 0x1F) << 8 | pPAT[11 + n];
                    // printf("program_map_PID = %x\n", program_map_PID);
                }
                if (!pat_element_exist(pat_packet.program, pat_program.program_map_PID))
                {
                    pat_packet.program.push_back(pat_program);
                    pat_element_exist(pat_packet.program, pat_program.program_map_PID);
                }
            }
            return 0;
        }
    }
    return -1;
}

// Parse PMT
int TSFormat::Parse_PMT(unsigned char *pTSBuf, TS_header *TSheader)
{
    if (TSheader->payload_unit_start_indicator == 0x01) // 表示含有PSI或者PES头
    {
        // if (TSheader.PID == 0x0) // 表示PMT
        {
            int iBeginlen = 4;
            int adaptation_field_length = pTSBuf[4];
            switch (TSheader->adaption_field_control)
            {
            case 0x0: // reserved for future use by ISO/IEC
                return -1;
            case 0x1:                               // 无调整字段，仅含有效负载
                iBeginlen += pTSBuf[iBeginlen] + 1; // + pointer_field
                break;
            case 0x2: // 仅含调整字段，无有效负载
                return -1;
            case 0x3: // 调整字段后含有效负载
                if (adaptation_field_length > 0)
                {
                    iBeginlen += 1;                       // adaptation_field_length占8位
                    iBeginlen += adaptation_field_length; // + adaptation_field_length
                }
                else
                {
                    iBeginlen += 1; // adaptation_field_length占8位
                }
                iBeginlen += pTSBuf[iBeginlen] + 1; // + pointer_field
                break;
            default:
                break;
            }
            unsigned char *pPAT = pTSBuf + iBeginlen;
            pmt_packet.table_id = pPAT[0];
            pmt_packet.section_syntax_indicator = pPAT[1] >> 7;
            pmt_packet.zero = pPAT[1] >> 6 & 0x1;
            pmt_packet.reserved_1 = pPAT[1] >> 4 & 0x3;
            pmt_packet.section_length = (pPAT[1] & 0x0F) << 8 | pPAT[2];
            pmt_packet.program_number = pPAT[3] << 8 | pPAT[4];
            pmt_packet.reserved_2 = pPAT[5] >> 6;
            pmt_packet.version_number = pPAT[5] >> 1 & 0x1F;
            pmt_packet.current_next_indicator = (pPAT[5] << 7) >> 7;
            pmt_packet.section_number = pPAT[6];
            pmt_packet.last_section_number = pPAT[7];

            pmt_packet.reserved_3 = pPAT[8] >> 5 & 0x7;
            pmt_packet.PCR_PID = (pPAT[8] & 0x1F) << 8 | pPAT[9];

            pmt_packet.reserved_4 = pPAT[10] >> 4 & 0xF;
            pmt_packet.program_info_length = (pPAT[10] & 0xF) << 8 | pPAT[11];

            int len = 0;
            len = 3 + pmt_packet.section_length;
            pmt_packet.CRC_32 = (pPAT[len - 4] & 0x000000FF) << 24 | (pPAT[len - 3] & 0x000000FF) << 16 |
                                (pPAT[len - 2] & 0x000000FF) << 8 | (pPAT[len - 1] & 0x000000FF);

            int n = 0;
            // program info descriptor
            if (pmt_packet.program_info_length != 0)
                n += pmt_packet.program_info_length;

            for (; n < (pmt_packet.section_length - 12) + 2 - 4; n += 5)
            {
                TS_PMT_Stream pmt_stream;
                pmt_stream.stream_type = pPAT[12 + n];
                pmt_stream.reserved_5 = pPAT[13 + n] >> 5;
                pmt_stream.elementary_PID = (pPAT[13 + n] & 0x1F) << 8 | pPAT[14 + n];
                pmt_stream.reserved_6 = pPAT[15 + n] >> 4;
                pmt_stream.ES_info_length = (pPAT[15 + n] & 0xF) << 8 | pPAT[16 + n];
                if (pmt_stream.ES_info_length != 0)
                {
                    for (int len = 2; len <= pmt_stream.ES_info_length; len++)
                    {
                        pmt_stream.descriptor = pmt_stream.descriptor << 8 | pPAT[16 + 4 + len];
                    }
                    n += pmt_stream.ES_info_length;
                }
                if (!pmt_element_exist(pmt_packet.PMT_Stream, pmt_stream.elementary_PID))
                {
                    pmt_packet.PMT_Stream.push_back(pmt_stream);
                }
            }
            return 0;
        }
    }
    return -1;
}

// Parse PES
int TSFormat::Parse_PES(unsigned char *pTSBuf, TS_header *TSheader)
{
    int iBeginlen = 4;
    int adaptation_field_length = pTSBuf[4];
    switch (TSheader->adaption_field_control)
    {
    case 0x0: // reserved for future use by ISO/IEC
        return -1;
    case 0x1: // 无调整字段，仅含有效负载
        // iBeginlen += pTSBuf[iBeginlen] + 1; // + pointer_field
        break;
    case 0x2: // 仅含调整字段，无有效负载
        return -1;
    case 0x3: // 调整字段后含有效负载
        if (adaptation_field_length > 0)
        {
            iBeginlen += 1;                       // adaptation_field_length占8位
            iBeginlen += adaptation_field_length; // + adaptation_field_length
        }
        else
        {
            iBeginlen += 1; // adaptation_field_length占8位
        }
        break;
    default:
        break;
    }

    if (TSheader->payload_unit_start_indicator == 0x01) // 表示含有PSI或者PES头
    {
        unsigned char *pPAT = pTSBuf + iBeginlen;

        if (frame.frame_ready_callback != nullptr && frame.frame_len != 0)
        {
            frame.frame_ready_callback((void *)&frame);
        }

        frame.frame_len = 0;

        pes_packet.packet_start_code_prefix = ((pPAT[0] << 16) | (pPAT[1] << 8) | (pPAT[2]));
        pes_packet.stream_id = ((pPAT[3]));
        frame.stream_type = pes_packet.stream_id;

        pes_packet.packet_length = ((pPAT[4] << 8) | (pPAT[5]));
        pes_packet.reserved_0 = ((pPAT[6] & 0xc0) >> 6);
        pes_packet.scrambling_control = ((pPAT[6] & 0x30) >> 4);

        pes_packet.priority = ((pPAT[6] & 0x08) >> 3);                 // 1 bslbf
        pes_packet.data_alignment_indicator = ((pPAT[6] & 0x04) >> 2); // 1 bslbf
        pes_packet.copyright = ((pPAT[6] & 0x02) >> 1);                // 1 bslbf
        pes_packet.original_or_copy = ((pPAT[6] & 0x01));              // 1 bslbf

        pes_packet.pts_dts_flags = ((pPAT[7] & 0xc0) >> 6); // 2 bslbf
        pes_packet.escr_flag = ((pPAT[7] & 0x20) >> 5);     // 1 bslbf
        pes_packet.es_rate_flag = ((pPAT[7] & 0x10) >> 4);  // 1 bslbf

        pes_packet.dsm_trick_mode_flag = ((pPAT[7] & 0x08) >> 3);       // 1 bslbf
        pes_packet.additional_copy_info_flag = ((pPAT[7] & 0x04) >> 2); // 1 bslbf
        pes_packet.crc_flag = ((pPAT[7] & 0x02) >> 1);                  // 1 bslbf
        pes_packet.extension_flag = ((pPAT[7] & 0x01));                 // 1 bslbf

        pes_packet.header_data_length = ((pPAT[8])); /* 8 uimsbf*/

        pPAT += 9;
        pes_packet.playload_offset = pes_packet.header_data_length + 9;
        switch (pes_packet.pts_dts_flags)
        {
        case 2:
            pes_packet.pts = ((uint64_t)((pPAT[0] & 0x0e) >> 1) << 30) |
                             ((((pPAT[1] << 8) | (pPAT[2] & 0xfe)) >> 1) << 15) |
                             (((pPAT[3] << 8) | (pPAT[4] & 0xfe)) >> 1);
            pes_packet.dts = NOPTS_VALUE;
            break;
        case 3:
            pes_packet.pts = ((uint64_t)((pPAT[0] & 0x0e) >> 1) << 30) |
                             ((((pPAT[1] << 8) | (pPAT[2] & 0xfe)) >> 1) << 15) |
                             (((pPAT[3] << 8) | (pPAT[4] & 0xfe)) >> 1);
            pPAT += 5;
            pes_packet.dts = ((uint64_t)((pPAT[0] & 0x0e) >> 1) << 30) |
                             ((((pPAT[1] << 8) | (pPAT[2] & 0xfe)) >> 1) << 15) |
                             (((pPAT[3] << 8) | (pPAT[4] & 0xfe)) >> 1);
            pPAT += 5;
            break;
        default:
            pes_packet.pts = NOPTS_VALUE;
            pes_packet.dts = NOPTS_VALUE;
            break;
        }
        frame.pts = pes_packet.pts;
        frame.dts = pes_packet.dts;

        memcpy(&frame.buf[frame.frame_len], &pTSBuf[pes_packet.playload_offset + iBeginlen],
               TS_STREAM_PACKET_LEN - pes_packet.playload_offset - iBeginlen);

        frame.frame_len += (TS_STREAM_PACKET_LEN - pes_packet.playload_offset - iBeginlen);

        static int count = 0;
        if (count < 100 && pTSBuf[pes_packet.playload_offset + iBeginlen] != 0xFF)
        {
            count++;

            for (uint32_t i = 0; i < 30; i++)
            {
                printf("0x%02x ", pTSBuf[pes_packet.playload_offset + iBeginlen+i]);
            }
            printf("\n");
        }

        return 0;
    }
    else
    {
        if (frame.frame_len != 0)
        {
            memcpy(&frame.buf[frame.frame_len], &pTSBuf[iBeginlen], TS_STREAM_PACKET_LEN - iBeginlen);
            frame.frame_len += (TS_STREAM_PACKET_LEN - iBeginlen);
        }
    }
    return -1;
}

int TSFormat::Parse_Packet(uint8_t *buf, void *parent)
{
    TS_header ts_header;

    frame.Private = parent;

    Parse_TS_header(buf, &ts_header);
    if (ts_header.PID == 0x00)
    {
        Parse_PAT(buf, &ts_header);
    }
    else if (pat_element_exist(pat_packet.program, ts_header.PID))
    {
        Parse_PMT(buf, &ts_header);
    }
    else if (pmt_element_exist(pmt_packet.PMT_Stream, ts_header.PID))
    {
        Parse_PES(buf, &ts_header);
    }
    else
    {
        if (ts_header.PID != 0x11)
        {
            printf("undefine packet ts_header.PID = %x\n", ts_header.PID);
        }
    }
}

void TSFormat::set_frame_ready_callback(FRAME_READY_CALLBACK callback)
{
    frame.frame_ready_callback = callback;
}

void TSFormat::clear_pmt(void)
{
    pat_packet.program.clear();
    pmt_packet.PMT_Stream.clear();
}

void TSFormat::clear_len(void)
{
    frame.frame_len = 0;
}