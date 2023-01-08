#include <xbee_api.hpp>

static uint8_t GLOBAL_FRAME_ID = 0x01;

void frameIDIncrement(){
    // avoid ID == 0 to always have a response
    if(GLOBAL_FRAME_ID == 0xFF)
        GLOBAL_FRAME_ID = 0x01;
    else
        GLOBAL_FRAME_ID += 1;
}

uint8_t* parseOptions(std::string options){
    // nullptr if no options
    if(options.empty())
        return nullptr;
    // ptr to parsed value
    uint8_t* parsedOpt = nullptr;
    uint16_t lengthOpt = options.length();
    // get first 2 chars of option
    std::string isHex = options.substr(0, 2);
    // hex if isHex == '0x' || isHex == '0X'
    if((isHex.compare("0x") == 0 || isHex.compare("0X") == 0) && lengthOpt > 2){
        // add leading 0 if odd number of chars
        std::string optVal = "";
        if(lengthOpt % 2 == 1){
            optVal = "0" + options.substr(2, lengthOpt);
        }
        else{
            optVal = options.substr(2, lengthOpt);
            // length is 1 shorter
            lengthOpt -=1;
        }
        //preserve chars in groups of 2
        parsedOpt = new uint8_t[(lengthOpt/2)]();
        // store length in 1st pos
        parsedOpt[0] = lengthOpt/2;
        std::string tmp = "";

        for(int i = 0; i < (lengthOpt/2); i++){
            tmp = optVal[2*i];
            tmp += optVal[(2*i)+1];
            parsedOpt[i+1] = std::stoi(tmp, nullptr, 16);
        }
    }
    // if not hex, convert 1:1
    else{
        parsedOpt = new uint8_t[lengthOpt+1]();
        //store length in first pos
        parsedOpt[0] = lengthOpt;
        for(int i = 0; i < lengthOpt; ++i)
            parsedOpt[i+1] = uint8_t(options[i]);
    }
    return parsedOpt;
}

// calculate xbee frame checksum per xbee documentation
uint8_t calculateCHKSM(uint8_t* framePtr, int frameLength){
    uint8_t sum = 0x00;
    for(int i = 3; i < frameLength -1; ++i){
        sum += framePtr[i];
    }
    return 0xFF - sum;
}

uint8_t* formATFrame(std::string ATCommand, std::string newvalue){
    // TO DO- Check that ATCommand is valid
    //if()
    // frame has 8 bytes without optional value
    // optional value can be a string or a hex value, denoted with "0x"
    uint16_t frameSize = 8;
    uint8_t *newValCpy = parseOptions(newvalue);
    uint16_t valLen = 0;
    if(newValCpy != nullptr){
        valLen = newValCpy[0];
        // update frame size
        frameSize += valLen;
        }
    // isolate CMD MSB and LSB
    std::string cmdBytes = ATCommand.substr(0, 2);
    // start forming frame
    uint8_t* framePtr = new uint8_t[frameSize]();
    framePtr[0] = 0x7E;
    // length field wants bytes from its end through checksum, so subtract overhead from frame
    int totalLength = frameSize - 4;
    // get MSB of length
    framePtr[1] = (totalLength && 0xFF00) >> 8;
    // get LXB of length
    framePtr[2] = totalLength;
    // assign AT Command identifier
    framePtr[3] = 0x08;
    // assign + increment running frame ID
    framePtr[4] = GLOBAL_FRAME_ID;
    frameIDIncrement();
    // assign command MSB and LSB
    framePtr[5] = uint8_t(cmdBytes[0]);
    framePtr[6] = uint8_t(cmdBytes[1]);
    // assign opt value
    if(int(valLen) > 0){
        for(int i = 0; i < valLen; ++i){
            framePtr[i+7] = newValCpy[i+1];
        }
    }
    // assign CHKSUM
    framePtr[frameSize-1] = calculateCHKSM(framePtr, frameSize);
    return framePtr;
}

uint8_t* formTXFrame(std::string RFData, uint64_t dst_64, uint16_t dst_16, uint8_t bcr, uint8_t opt){
    // 18 bytes of overhead in TX frame
    uint16_t frameSize = 18;
    uint16_t RFMax = 0x100;
    // check for encryption enabled
    if((opt & 2) == 0x02)
        RFMax -=4;
    // get length of RFData
    uint16_t RFSize = RFData.length();
    // only capture up to 256 bytes // throw error if too big
    if(RFSize > RFMax)
        return nullptr;
    // total frame size
    frameSize += RFSize;

    // begin forming frame
    uint8_t* framePtr = new uint8_t[frameSize]();
    framePtr[0] = 0x7E;
    // assign sizes
    framePtr[1] = (frameSize-4) >> 8;
    framePtr[2] = (frameSize-4);
    // assign frame type and ID
    framePtr[3] = 0x10;
    // assign + increment running frame ID
    framePtr[4] = GLOBAL_FRAME_ID;
    frameIDIncrement();
    // assign 8 byte address
    for(int i = 0; i < 8; ++i){
        framePtr[i+5] = dst_64 >> (56-(8*i));
    }
    //assign 2 byte address
    framePtr[13] = dst_16 >> 8;
    framePtr[14] = dst_16;
    // assign BCR and OPT
    framePtr[15] = bcr;
    framePtr[16] = opt;
    // assign RF data
    for(int i = 0; i < RFSize; ++i){
        framePtr[17+i] = uint8_t(RFData[i]);
    }
    // assign checksum
    framePtr[frameSize-1] = calculateCHKSM(framePtr, frameSize);
    return framePtr;
}

uint8_t* formATFrame_Remote(std::string ATCommand, uint64_t dst_64, std::string newvalue, uint8_t opt, uint16_t dst_16){
    // 19 bytes without optional value
    uint16_t frameSize = 19;
    // get optional value
    uint8_t *newValCpy = parseOptions(newvalue);
    uint16_t valLen = 0;
    if(newValCpy != nullptr){
        valLen = newValCpy[0];
        // update frame size
        frameSize += valLen;
        }
    // isolate CMD MSB and LSB
    std::string cmdBytes = ATCommand.substr(0, 2);
    // start forming frame
    uint8_t* framePtr = new uint8_t[frameSize]();
    framePtr[0] = 0x7E;
    // length field wants bytes from its end through checksum, so subtract overhead from frame
    int totalLength = frameSize - 4;
    framePtr[1] = (totalLength && 0xFF00) >> 8;
    // get LXB of length
    framePtr[2] = totalLength;
    // assign AT Command identifier
    framePtr[3] = 0x17;
    // assign + increment running frame ID
    framePtr[4] = GLOBAL_FRAME_ID;
    frameIDIncrement();
    // assign 8 byte address
    for(int i = 0; i < 8; ++i){
        framePtr[i+5] = dst_64 >> (56-(8*i));
    }
    //assign 2 byte address
    framePtr[13] = dst_16 >> 8;
    framePtr[14] = dst_16;
    // assign opt
    framePtr[15] = opt;
    // assign command MSB and LSB
    framePtr[16] = uint8_t(cmdBytes[0]);
    framePtr[17] = uint8_t(cmdBytes[1]);
    if(int(valLen) > 0){
        for(int i = 0; i < valLen; ++i){
            framePtr[i+18] = newValCpy[i+1];
        }
    }
    // assign CHKSUM
    framePtr[frameSize-1] = calculateCHKSM(framePtr, frameSize);
    return framePtr;
}

json* parseATCR(uint8_t* frame, uint16_t frameLength){
    std::string cmd = "";
    cmd += char(frame[5]);
    cmd += char(frame[6]);
    std::string data = "";
    // length > 5 means there is data
    if(frameLength > 5){
      for(int i=0; i < frameLength-5; ++i)
        data += char(frame[i+8]);
    }
    json *tmp = new json{
        {"FRAME TYPE", "Local AT Command Response"},
        {"FRAME ID", frame[4]},
        {"COMMAND", cmd},
        {"STATUS", frame[7]},
        {"DATA",data}
    };
    return tmp;
}

json* parseMS(uint8_t* frame, uint16_t frameLength){
    json *tmp = new json{
        {"FRAME TYPE", "MODEM STATUS"},
        {"FRAME ID", frame[3]},
        {"STATUS", frame[4]},
    };
    return tmp;
}

json* parseTS(uint8_t* frame, uint16_t frameLength){
    std::string dst = "0x";
    dst += char(frame[5]);
    dst += char(frame[6]);
    json *tmp = new json{
        {"FRAME TYPE", "Transmission Status"},
        {"FRAME ID", frame[4]},
        {"DESTINATION", dst},
        {"TRANSMIT RETRY", frame[7]},
        {"DELIVERY STATUS", frame[8]},
        {"DISCOVERY STATUS", frame[9]}
    };
    return tmp;
}

json* receivePacket(uint8_t* frame, uint16_t frameLength){
    // get addresses
    std::string dst64 = "0x";
    for(int i = 0; i <8; ++i)
        dst64 += char(frame[i+4]);
    std::string dst16 = "0x";
    dst16 += char(frame[12]);
    dst16 += char(frame[13]);
    std::string data = "";
    // length > 12 means there is data
    if(frameLength > 12){
      for(int i=0; i < frameLength-12; ++i)
        data += char(frame[i+15]);
    }
    json *tmp = new json{
        {"FRAME TYPE", "Receive Packet"},
        {"FRAME ID", frame[4]},
        {"DST64", dst64},
        {"DST16", dst16},
        {"OPT", frame[14]},
        {"DATA",data}
    };
    return tmp;
}

json* explicitRX(uint8_t* frame, uint16_t frameLength){
    // get addresses
    std::string dst64 = "0x";
    for(int i = 0; i <8; ++i)
        dst64 += char(frame[i+4]);
    std::string dst16 = "0x";
    dst16 += char(frame[12]);
    dst16 += char(frame[13]);
    std::string cluster = "0x";
    cluster+= char(frame[16]);
    cluster+= char(frame[17]);
    std::string profile = "0x";
    cluster+= char(frame[18]);
    cluster+= char(frame[19]);
    std::string data = "";
    // length > 18 means there is data
    if(frameLength > 18){
      for(int i=0; i < frameLength-18; ++i)
        data += char(frame[i+21]);
    }
    json *tmp = new json{
        {"FRAME TYPE", "Receive Packet"},
        {"FRAME ID", frame[4]},
        {"DST64", dst64},
        {"DST16", dst16},
        {"SRC", frame[14]},
        {"DST", frame[15]},
        {"CLUSTER", cluster},
        {"PROFILE", profile},
        {"OPT", frame[20]},
        {"DATA",data}
    };
    return tmp;
}

json* remoteAT(uint8_t* frame, uint16_t frameLength){
    // get addresses
    std::string dst64 = "0x";
    for(int i = 0; i <8; ++i)
        dst64 += char(frame[i+5]);
    std::string dst16 = "0x";
    dst16 += char(frame[13]);
    dst16 += char(frame[14]);
    std::string cmd = "0x";
    cmd += char(frame[15]);
    cmd += char(frame[16]);
    std::string data = "";
    // length > 12 means there is data
    if(frameLength > 15){
      for(int i=0; i < frameLength-15; ++i)
        data += char(frame[i+18]);
    }
    json *tmp = new json{
        {"FRAME TYPE", "Receive Packet"},
        {"FRAME ID", frame[4]},
        {"DST64", dst64},
        {"DST16", dst16},
        {"AT CMD", cmd},
        {"STATUS", frame[17]},
        {"DATA",data}
    };
    return tmp;
}

json* readFrame(uint8_t* frame){
    // return nullptr if not valid
    if(frame[0] != 0x7E)
        return nullptr;
    json *data = nullptr;
    // find data size
    uint16_t frameLength = frame[1] + frame[2];
    // read in appropriate data based on frame type
    switch(frame[3]){
        // AT command response
        case 0x88:
            data = parseATCR(frame, frameLength);
            break;
        // modem status
        case 0x8A:
            data = parseMS(frame, frameLength);
            break;
        // transmit status
        case 0x8B:
            data = parseTS(frame, frameLength);
            break;
        // receive packet
        case 0x90:
            data = receivePacket(frame, frameLength);
            break;
        // explicit rx
        case 0x91:
            data = explicitRX(frame, frameLength);
            break;
        // remote AT resposne
        case 0x97:
            data = remoteAT(frame, frameLength);
            break;
        default:
            break;
    }
    return data;
}
