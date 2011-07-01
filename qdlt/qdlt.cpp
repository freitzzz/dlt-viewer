#include <QTextStream>
#include <QFile>
#include <QtDebug>

#include <qextserialport.h>
#include <QTcpSocket>

#include "qdlt.h"

extern "C"
{
    #include "dlt_common.h"
    #include "dlt_user_shared.h"
}

const char *qDltMessageType[] = {"log","app_trace","nw_trace","control","","","",""};
const char *qDltLogInfo[] = {"","fatal","error","warn","info","debug","verbose","","","","","","","","",""};
const char *qDltTraceType[] = {"","variable","func_in","func_out","state","vfb","","","","","","","","","",""};
const char *qDltNwTraceType[] = {"","ipc","can","flexray","most","vfb","","","","","","","","","",""};
const char *qDltControlType[] = {"","request","response","time","","","","","","","","","","","",""};
const char *qDltMode[] = {"non-verbose","verbose"};
const char *qDltEndianness[] = {"little-endian","big-endian"};
const char *qDltTypeInfo[] = {"String","Bool","SignedInteger","UnsignedInteger","Float","RawData","TraceInfo"};
const char *qDltCtrlServiceId[] = {"","set_log_level","set_trace_status","get_log_info","get_default_log_level","store_config","reset_to_factory_default",
                             "set_com_interface_status","set_com_interface_max_bandwidth","set_verbose_mode","set_message_filtering","set_timing_packets",
                             "get_local_time","use_ecu_id","use_session_id","use_timestamp","use_extended_header","set_default_log_level","set_default_trace_status",
                             "get_software_version","message_buffer_overflow"};
const char *qDltCtrlReturnType [] = {"ok","not_supported","error","3","4","5","6","7","no_matching_context_id"};


QDlt::QDlt()
{

}

QDlt::~QDlt()
{

}

bool QDlt::swap(QByteArray &bytes,int size, int offset)
{
    char tmp;

    if( (offset < 0)  || (offset >= bytes.size()) )
        return false;

    if(size == -1)
        size = bytes.size()-offset;

    if((size+offset) > bytes.size())
        return false;

    for(int num = 0;num<(size/2);num++)
    {
        tmp = bytes[offset+num];
        bytes[offset+num] = bytes[offset+size-1-num];
        bytes[offset+size-1-num] = tmp;
    }

    return true;
}

QString QDlt::toAsciiTable(QByteArray &bytes, bool withLineNumber, bool withBinary, bool withAscii, int blocksize, int linesize, bool toHtml)
{
    QString text;

    /* create HTML text to show */
    if(toHtml)
        text+=QString("<html><body>");

    if(toHtml)
        text += QString("<pre>");

    int lines = (bytes.size()+linesize-1)/linesize;
    for(int line=0;line<lines;line++)
    {
        if(withLineNumber)
            text += QString("%1: ").arg(line*linesize,4,linesize,QLatin1Char('0'));
        if(withBinary)
        {
            for(int num=0;num<linesize;num++)
            {
                char ch = (bytes.constData())[line*linesize+num];
                if(num==blocksize)
                    text += QString("  ");
                else if(num!=0)
                    text += QString(" ");
                if((line*linesize+num) < bytes.size())
                    text += QString("%1").arg((unsigned char)ch,2,linesize,QLatin1Char('0'));
                else
                    text += QString("--");
            }
        }
        if(withAscii)
        {
            text += QString(" ");
            for(int num=0;num<linesize;num++)
            {
                char ch = (bytes.constData())[line*linesize+num];
                if( ((line*linesize+num) < bytes.size()) && (ch >= ' ') && (ch <= '~') )
                    text += QString(QChar(ch));
                else
                    text += QString("-");
            }
        }
        if(line!=lines-1)
        {
            if(toHtml)
                text += QString("<BR>");
            else
                text += QString("\n");
        }
    }
    if(toHtml)
        text += QString("</pre>");

    /* finish HTML text */
    if(toHtml)
        text+=QString("</body></html>");

    return text;
}

QString QDlt::toAscii(QByteArray &bytes, bool ascii)
{
    QString text;

    /* create text to show */
    for(int num=0;num<bytes.size();num++)
    {
        char ch = (bytes.constData())[num];
        if(ascii) {
            if( (ch >= ' ') && (ch <= '~') )
                text += QString(QChar(ch));
            else
                text += QString("-");
        }
        else {
            if(num!=0)
                text += QString(" ");
            text += QString("%1").arg((unsigned char)ch,2,16,QLatin1Char('0'));
        }
    }

    return text;
}

QDltArgument::QDltArgument()
{
    /* clear content of argument */
    clear();
}

QDltArgument::~QDltArgument()
{

}

int QDltArgument::getOffsetPayload()
{
    return offsetPayload;
}

QByteArray QDltArgument::getData()
{
   return data;
}

QString QDltArgument::getName()
{
    return name;
}

QString QDltArgument::getUnit()
{
    return unit;
}

int QDltArgument::getDataSize()
{
    return data.size();
}

QDltArgument::DltTypeInfoDef QDltArgument::getTypeInfo()
{
    return typeInfo;
}

QString QDltArgument::getTypeInfoString()
{
    if(typeInfo<0)
        return QString("");

    return QString(qDltTypeInfo[typeInfo]);
}

bool QDltArgument::setArgument(QByteArray &payload,unsigned int &offset,DltEndiannessDef _endianess)
{
    unsigned int dltType;
    unsigned short length=0,length2=0,length3=0;

    /* clear old data */
    clear();

    /* store offset */
    offsetPayload = offset;

    /* store new endianess */
    endianness = _endianess;

    /* get type info */
    if((unsigned int)payload.size()<(offset+sizeof(unsigned int)))
        return false;
    if(endianness == DltEndiannessLittleEndian)
        dltType = *((unsigned int*) (payload.data()+offset));
    else
        dltType = DLT_SWAP_32((*((unsigned int*) (payload.data()+offset))));
    offset += sizeof(unsigned int);

    if (dltType& DLT_TYPE_INFO_STRG)
    {
        typeInfo = DltTypeInfoStrg;
    }
    else if (dltType & DLT_TYPE_INFO_BOOL)
    {
        typeInfo = DltTypeInfoBool;
    }
    else if (dltType & DLT_TYPE_INFO_SINT)
    {
        typeInfo = DltTypeInfoSInt;
    }
    else if (dltType & DLT_TYPE_INFO_UINT)
    {
        typeInfo = DltTypeInfoUInt;
    }
    else if (dltType & DLT_TYPE_INFO_FLOA)
    {
        typeInfo = DltTypeInfoFloa;
    }
    else if (dltType & DLT_TYPE_INFO_RAWD)
    {
        typeInfo = DltTypeInfoRawd;
    }
    else if (dltType & DLT_TYPE_INFO_TRAI)
    {
        typeInfo = DltTypeInfoTrai;
    }
    else
    {
        typeInfo = DltTypeInfoUnknown;
        return false;
    }

    /* get length of string, raw data or trace info */
    if(typeInfo == DltTypeInfoStrg || typeInfo == DltTypeInfoRawd || typeInfo == DltTypeInfoTrai)
    {
        if((unsigned int)payload.size()<(offset+sizeof(unsigned short)))
            return false;
        if(endianness == DltEndiannessLittleEndian)
            length = *((unsigned short*) (payload.data()+offset));
        else
            length = DLT_SWAP_16((*((unsigned short*) (payload.data()+offset))));

        offset += sizeof(unsigned short);
    }

    /* get variable info */
    if(dltType & DLT_TYPE_INFO_VARI)
    {
        if((unsigned int)payload.size()<(offset+sizeof(unsigned short)))
            return false;
        if(endianness == DltEndiannessLittleEndian)
            length2 = *((unsigned short*) (payload.data()+offset));
        else
            length2 = DLT_SWAP_16((*((unsigned short*) (payload.data()+offset))));
        offset += sizeof(unsigned short);
        if(typeInfo == DltTypeInfoSInt || typeInfo == DltTypeInfoUInt || typeInfo == DltTypeInfoFloa)
        {
            if((unsigned int)payload.size()<(offset+sizeof(unsigned short)))
                return false;
            if(endianness == DltEndiannessLittleEndian)
                length3 = *((unsigned short*) (payload.data()+offset));
            else
                length3 = DLT_SWAP_16((*((unsigned short*) (payload.data()+offset))));
            offset += sizeof(unsigned short);
        }
        name = QString(payload.mid(offset,length2));
        offset += length2;
        if(typeInfo == DltTypeInfoSInt || typeInfo == DltTypeInfoUInt || typeInfo == DltTypeInfoFloa)
        {
            unit = QString(payload.mid(offset,length3));
            offset += length3;
        }
    }

    /* get fix point quantisation and offset */
    if(dltType & DLT_TYPE_INFO_FIXP)
    {
        /* not supported yet */
        return false;
    }

    /* get data */
    if(typeInfo == DltTypeInfoStrg || typeInfo == DltTypeInfoRawd || typeInfo == DltTypeInfoTrai)
    {
        if((unsigned int)payload.size()<(offset+length))
            return false;
        data = payload.mid(offset,length);
        offset += length;
    }
    else if(typeInfo == DltTypeInfoBool)
    {
        data = payload.mid(offset,1);
        offset += 1;
    }
    else if(typeInfo == DltTypeInfoSInt || typeInfo == DltTypeInfoUInt)
    {
        switch (dltType & DLT_TYPE_INFO_TYLE)
        {
            case DLT_TYLE_8BIT:
            {
                data = payload.mid(offset,1);
                offset += 1;
                break;
            }
            case DLT_TYLE_16BIT:
            {
                data = payload.mid(offset,2);
                offset += 2;
                break;
            }
            case DLT_TYLE_32BIT:
            {
                data = payload.mid(offset,4);
                offset += 4;
                break;
            }
            case DLT_TYLE_64BIT:
            {
                data = payload.mid(offset,8);
                offset += 8;
                break;
            }
            case DLT_TYLE_128BIT:
            {
                data = payload.mid(offset,16);
                offset += 16;
                break;
            }
            default:
            {
                return false;
            }
        }

    }
    else if(typeInfo == DltTypeInfoFloa)
    {
        switch(dltType & DLT_TYPE_INFO_TYLE)
        {
            case DLT_TYLE_8BIT:
            {
                data = payload.mid(offset,1);
                offset += 1;
                break;
            }
            case DLT_TYLE_16BIT:
             {
                data = payload.mid(offset,2);
                offset += 2;
                break;
            }
            case DLT_TYLE_32BIT:
            {
                data = payload.mid(offset,4);
                offset += 4;
                break;
            }
            case DLT_TYLE_64BIT:
            {
                data = payload.mid(offset,8);
                offset += 8;
                break;
            }
            case DLT_TYLE_128BIT:
            {
                data = payload.mid(offset,16);
                offset += 16;
                break;
            }
            default:
            {
                return false;
            }
        }

    }

    return true;
}

bool QDltArgument::getArgument(QByteArray &payload, bool verboseMode)
{
    unsigned int dltType = 0;
    unsigned short length;

    /* add the type info in verbose mode */
    if(verboseMode) {
        switch(typeInfo) {
        case DltTypeInfoUnknown:
            return false;
        case DltTypeInfoStrg:
            dltType |= DLT_TYPE_INFO_STRG;
            break;
        case DltTypeInfoBool:
            dltType |= DLT_TYPE_INFO_BOOL;
            break;
        case DltTypeInfoSInt:
            dltType |= DLT_TYPE_INFO_SINT;
            break;
        case DltTypeInfoUInt:
            dltType |= DLT_TYPE_INFO_UINT;
            break;
        case DltTypeInfoFloa:
            dltType |= DLT_TYPE_INFO_FLOA;
            break;
        case DltTypeInfoRawd:
            dltType |= DLT_TYPE_INFO_RAWD;
            break;
        case DltTypeInfoTrai:
            // dltType |= DLT_TYPE_INFO_TRAI;
            return false;
        default:
            return false;
        }
        if((typeInfo == DLT_TYPE_INFO_SINT) || (typeInfo == DLT_TYPE_INFO_UINT) || (typeInfo == DLT_TYPE_INFO_FLOA)) {
            switch(data.size())
            {
            case 1:
                dltType |= DLT_TYLE_8BIT;
                break;
            case 2:
                dltType |= DLT_TYLE_16BIT;
                break;
            case 4:
                dltType |= DLT_TYLE_32BIT;
                break;
            case 8:
                dltType |= DLT_TYLE_64BIT;
                break;
            case 16:
                dltType |= DLT_TYLE_128BIT;
                break;
            default:
                return false;
            }
        }
        payload += QByteArray((const char*)&dltType,sizeof(unsigned int));
    }

    /* add the string or raw data size to the payload */
    if((typeInfo == DLT_TYPE_INFO_RAWD) || (typeInfo == DLT_TYPE_INFO_STRG)) {
        length = data.size();
        payload += QByteArray((const char*)&dltType,sizeof(unsigned int));
    }

    /* add the value to the payload */
    payload += data;

    return true;
}

void QDltArgument::clear()
{
    typeInfo = QDltArgument::DltTypeInfoUnknown;
    offsetPayload = 0;
    data.clear();
    name.clear();
    unit.clear();
    endianness = QDltArgument::DltEndiannessUnknown;

}

QString QDltArgument::toString(bool binary)
{
    QString text;

    if(binary) {
        return toAscii(data);
    }

    switch(getTypeInfo()) {
    case DltTypeInfoUnknown:
        text += QString("?");
        break;
    case DltTypeInfoStrg:
        if(data.size()) {
            text += QString("%1").arg(QString(getData()));
        }
        break;
    case DltTypeInfoBool:
        if(data.size()) {
            if(data.constData()[0])
                text += QString("true");
            else
                text += QString("false");
        }
        else
            text += QString("?");
        break;
    case DltTypeInfoSInt:
        switch(data.size())
        {
        case 1:
            text += QString("%1").arg((short)(*(char*)(data.constData())));
            break;
        case 2:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((short)(*(short*)(data.constData())));
            else
                text += QString("%1").arg(DLT_SWAP_16((short)(*(short*)(data.constData()))));
            break;
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((int)(*(int*)(data.constData())));
            else
                text += QString("%1").arg(DLT_SWAP_32((int)(*(int*)(data.constData()))));
            break;
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((long long)(*(long long*)(data.constData())));
            else
                text += QString("%1").arg(DLT_SWAP_64((long long)(*(long long*)(data.constData()))));
            break;
        default:
            text += QString("?");
        }

        break;
    case DltTypeInfoUInt:
        switch(data.size())
        {
        case 1:
            text += QString("%1").arg((unsigned short)(*(unsigned char*)(data.constData())));
            break;
        case 2:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((unsigned short)(*(unsigned short*)(data.constData())));
            else
                text += QString("%1").arg(DLT_SWAP_16((unsigned short)(*(unsigned short*)(data.constData()))));
            break;
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((unsigned int)(*(unsigned int*)(data.constData())));
            else
                text += QString("%1").arg(DLT_SWAP_32((unsigned int)(*(unsigned int*)(data.constData()))));
            break;
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((unsigned long long)(*(unsigned long long*)(data.constData())));
            else
                text += QString("%1").arg(DLT_SWAP_64((unsigned long long)(*(unsigned long long*)(data.constData()))));
            break;
        default:
            text += QString("?");
        }

        break;
    case DltTypeInfoFloa:
        switch(data.size())
        {
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((double)(*(float*)(data.constData())));
            else
            {
                unsigned int tmp;
                tmp = DLT_SWAP_32((unsigned int)(*(unsigned int*)(data.constData())));
                text += QString("%1").arg((double)(*(float*)((void*)&tmp)));
            }
            break;
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((double)(*(double*)(data.constData())));
            else {
                unsigned int tmp;
                tmp = DLT_SWAP_64((unsigned long long)(*(unsigned long long*)(data.constData())));
                text += QString("%1").arg((double)(*(double*)((void*)&tmp)));
            }
            break;
        default:
            text += QString("?");
        }
        break;
    case DltTypeInfoRawd:
        text += toAscii(data);
        break;
    case DltTypeInfoTrai:
        text += QString("?");
        break;
    default:
        text += QString("?");
    }

    return text;
}

QVariant QDltArgument::getValue()
{
    switch(typeInfo) {
    case DltTypeInfoUnknown:
        break;
    case DltTypeInfoStrg:
        if(data.size()) {
            return QVariant(QString(getData()));
        }
        break;
    case DltTypeInfoBool:
        if(data.size()) {
            return QVariant((bool)(data.constData()[0]));
        }
        break;
    case DltTypeInfoSInt:
        switch(data.size())
        {
        case 1:
            return QVariant((short)(*(char*)(data.constData())));
        case 2:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((short)(*(short*)(data.constData())));
            else
                return QVariant(DLT_SWAP_16((short)(*(short*)(data.constData()))));
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((int)(*(int*)(data.constData())));
            else
                return QVariant(DLT_SWAP_32((int)(*(int*)(data.constData()))));
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((long long)(*(long long*)(data.constData())));
            else
                return QVariant(DLT_SWAP_64((long long)(*(long long*)(data.constData()))));
        default:
            break;
        }
        break;
    case DltTypeInfoUInt:
        switch(data.size())
        {
        case 1:
            return QVariant((unsigned short)(*(unsigned char*)(data.constData())));
        case 2:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((unsigned short)(*(unsigned short*)(data.constData())));
            else
                return QVariant(DLT_SWAP_16((unsigned short)(*(unsigned short*)(data.constData()))));
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((unsigned int)(*(unsigned int*)(data.constData())));
            else
                return QVariant(DLT_SWAP_32((unsigned int)(*(unsigned int*)(data.constData()))));
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((unsigned long long)(*(unsigned long long*)(data.constData())));
            else
                return QVariant(DLT_SWAP_64((unsigned long long)(*(unsigned long long*)(data.constData()))));
        default:
            break;
        }

        break;
    case DltTypeInfoFloa:
        switch(data.size())
        {
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((double)(*(float*)(data.constData())));
            else
            {
                unsigned int tmp;
                tmp = DLT_SWAP_32((unsigned int)(*(unsigned int*)(data.constData())));
                return QVariant((double)(*(float*)((void*)&tmp)));
            }
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((double)(*(double*)(data.constData())));
            else {
                unsigned int tmp;
                tmp = DLT_SWAP_64((unsigned long long)(*(unsigned long long*)(data.constData())));
                return QVariant((double)(*(double*)((void*)&tmp)));
            }
        default:
            break;
        }
        break;
    case DltTypeInfoRawd:
        return QVariant(data);
        break;
    case DltTypeInfoTrai:
        break;
    default:
        break;
    }

    return QVariant();
}

bool QDltArgument::setValue(QVariant value, bool verboseMode)
{
    endianness = QDltArgument::DltEndiannessLittleEndian;

    switch(value.type())
    {
    case QVariant::ByteArray:
        data = value.toByteArray();
        typeInfo = QDltArgument::DltTypeInfoRawd;
        return true;
    case QVariant::String:
        data = value.toByteArray();
        typeInfo = QDltArgument::DltTypeInfoStrg;
        return true;
    case QVariant::Bool:
        {
        bool bvalue = value.toBool();
        unsigned char cvalue = bvalue;
        data = QByteArray((const char*)&cvalue,sizeof(unsigned char));
        typeInfo = QDltArgument::DltTypeInfoSInt;
        return true;
        }
        break;
    case QVariant::Int:
        {
        int bvalue = value.toInt();
        data = QByteArray((const char*)&bvalue,sizeof(int));
        typeInfo = QDltArgument::DltTypeInfoSInt;
        return true;
        }
    case QVariant::LongLong:
        {
        long long bvalue = value.toLongLong();
        data = QByteArray((const char*)&bvalue,sizeof(long long));
        typeInfo = QDltArgument::DltTypeInfoSInt;
        return true;
        }
    case QVariant::UInt:
        {
        unsigned int bvalue = value.toUInt();
        data = QByteArray((const char*)&bvalue,sizeof(int));
        typeInfo = QDltArgument::DltTypeInfoUInt;
        return true;
        }
    case QVariant::ULongLong:
        {
        unsigned long long bvalue = value.toULongLong();
        data = QByteArray((const char*)&bvalue,sizeof(unsigned long long));
        typeInfo = QDltArgument::DltTypeInfoUInt;
        return true;
        }
    case QVariant::Double:
        {
        double bvalue = value.toInt();
        data = QByteArray((const char*)&bvalue,sizeof(double));
        typeInfo = QDltArgument::DltTypeInfoFloa;
        return true;
        }
        break;
    default:
        break;
    }

    return false;
}

QDltMsg::QDltMsg()
{
    clear();
}

QDltMsg::~QDltMsg()
{

}

QString QDltMsg::getTypeString()
{
    return QString((type>=0 && type<=7)?qDltMessageType[type]:"");
}

QString QDltMsg::getSubtypeString()
{
    switch(type)
    {
    case DltTypeLog:
        return QString((subtype>=0 && subtype<=7)?qDltLogInfo[subtype]:"");
        break;
    case DltTypeAppTrace:
        return QString((subtype>=0 && subtype<=7)?qDltTraceType[subtype]:"");
        break;
    case DltTypeNwTrace:
        return QString((subtype>=0 && subtype<=7)?qDltNwTraceType[subtype]:"");
        break;
    case DltTypeControl:
        return QString((subtype>=0 && subtype<=7)?qDltControlType[subtype]:"");
        break;
    default:
        return QString("");
    }
}

QString QDltMsg::getModeString()
{
    return QString((mode>=0 && mode<=1)?qDltMode[mode]:"");
}

QString QDltMsg::getEndiannessString()
{
    return QString((endianness>=0 && endianness<=1)?qDltEndianness[endianness]:"");
}

unsigned int QDltMsg::getCtrlServiceId()
{
    return ctrlServiceId;
}

QString QDltMsg::getCtrlServiceIdString()
{
    return QString(( ctrlServiceId<=20 )?qDltCtrlServiceId[ctrlServiceId]:"");
}

unsigned char QDltMsg::getCtrlReturnType()
{
    return ctrlReturnType;
}

QString QDltMsg::getCtrlReturnTypeString()
{
    return QString(( ctrlReturnType<=8 )?qDltCtrlReturnType[ctrlReturnType]:"");
}

bool QDltMsg::setMsg(QByteArray buf, bool withStorageHeader)
{
    unsigned int offset;
    QDltArgument argument;
    DltStorageHeader *storageheader = 0;
    DltStandardHeader *standardheader = 0;
    DltExtendedHeader *extendedheader = 0;
    DltStandardHeaderExtra headerextra;
    unsigned int extra_size,headersize,datasize;
    int sizeStorageHeader = 0;

    /* set offset of storage header */
    if(withStorageHeader) {
        sizeStorageHeader = sizeof(DltStorageHeader);
    }

    /* empty message */
    clear();

    if(buf.size() < (int)(sizeStorageHeader+sizeof(DltStandardHeader))) {
        return false;
    }

    if(withStorageHeader) {
        storageheader = (DltStorageHeader*) buf.data();
    }
    standardheader = (DltStandardHeader*) (buf.data() + sizeStorageHeader);

    /* calculate complete size of headers */
    extra_size = DLT_STANDARD_HEADER_EXTRA_SIZE(standardheader->htyp)+(DLT_IS_HTYP_UEH(standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
    headersize = sizeStorageHeader + sizeof(DltStandardHeader) + extra_size;
    datasize =  DLT_SWAP_16(standardheader->len) - (headersize - sizeStorageHeader);

    /* load standard header extra parameters and Extended header if used */
    if (extra_size>0)
    {
        if (buf.size()  < (int)(headersize - sizeStorageHeader)) {
            return false;
        }

        /* set extended header ptr and get standard header extra parameters */
        if (DLT_IS_HTYP_UEH(standardheader->htyp)) {
            extendedheader = (DltExtendedHeader*) (buf.data() + sizeStorageHeader + sizeof(DltStandardHeader) +
                                  DLT_STANDARD_HEADER_EXTRA_SIZE(standardheader->htyp));
        }
        else {
            extendedheader = 0;
        }

        if (DLT_IS_HTYP_WEID(standardheader->htyp))
        {
            memcpy(headerextra.ecu,buf.data() + sizeStorageHeader + sizeof(DltStandardHeader),DLT_ID_SIZE);
        }

        if (DLT_IS_HTYP_WSID(standardheader->htyp))
        {
            memcpy(&(headerextra.seid),buf.data() + sizeStorageHeader + sizeof(DltStandardHeader)
                   + (DLT_IS_HTYP_WEID(standardheader->htyp) ? DLT_SIZE_WEID : 0), DLT_SIZE_WSID);
            headerextra.seid = DLT_BETOH_32(headerextra.seid);
        }

        if (DLT_IS_HTYP_WTMS(standardheader->htyp))
        {
            memcpy(&(headerextra.tmsp),buf.data() + sizeStorageHeader + sizeof(DltStandardHeader)
                   + (DLT_IS_HTYP_WEID(standardheader->htyp) ? DLT_SIZE_WEID : 0)
                   + (DLT_IS_HTYP_WSID(standardheader->htyp) ? DLT_SIZE_WSID : 0),DLT_SIZE_WTMS);
            headerextra.tmsp = DLT_BETOH_32(headerextra.tmsp);
        }
    }

    /* extract ecu id */
    if ( DLT_IS_HTYP_WEID(standardheader->htyp) )
    {
        ecuid = QString(QByteArray(headerextra.ecu,4));
    }
    else
    {
        if(storageheader)
            ecuid = QString(QByteArray(storageheader->ecu,4));
    }

    /* extract application id */
    if ((DLT_IS_HTYP_UEH(standardheader->htyp)) && (extendedheader->apid[0]!=0))
    {
        apid = QString(QByteArray(extendedheader->apid,4));
    }

    /* extract context id */
    if ((DLT_IS_HTYP_UEH(standardheader->htyp)) && (extendedheader->ctid[0]!=0))
    {
        ctid = QString(QByteArray(extendedheader->ctid,4));
    }

    /* extract type */
    if (DLT_IS_HTYP_UEH(standardheader->htyp))
    {
        type = (DltTypeDef) DLT_GET_MSIN_MSTP(extendedheader->msin);
    }

    /* extract subtype */
    if (DLT_IS_HTYP_UEH(standardheader->htyp))
    {
        subtype = DLT_GET_MSIN_MTIN(extendedheader->msin);
    }

    /* extract mode */
    if (DLT_IS_HTYP_UEH(standardheader->htyp))
    {
        if(DLT_IS_MSIN_VERB(extendedheader->msin))
        {
            mode = DltModeVerbose;
        }
        else
        {
            mode = DltModeNonVerbose;
        }
    }
    else
    {
        mode = DltModeNonVerbose;
    }

    /* extract endianness */
    if(DLT_IS_HTYP_MSBF(standardheader->htyp)) {
        endianness = DltEndiannessBigEndian;
    }
    else {
        endianness = DltEndiannessLittleEndian;
    }

    /* extract time */
    if(storageheader) {
        time.setTime_t(storageheader->seconds);
        microseconds = storageheader->microseconds;
    }

    /* extract timestamp */
    if ( DLT_IS_HTYP_WTMS(standardheader->htyp) ) {
        timestamp = headerextra.tmsp; /* big endian to host little endian conversion already done */
    }

    /* extract session id */
    if (DLT_IS_HTYP_WSID(standardheader->htyp)) {
        sessionid = headerextra.tmsp;
    }

    /* extract message counter */
    messageCounter = standardheader->mcnt;

    /* extract number of arguments */
    if (DLT_IS_HTYP_UEH(standardheader->htyp)) {
        numberOfArguments = extendedheader->noar;
    }

    /* copy payload */
    payload = buf.mid(headersize);

    /* set messageid if non verbose and no extended header */
    if(!DLT_IS_HTYP_UEH(standardheader->htyp) && payload.size()>=4) {
        /* message id is always in big endian format */
        if(endianness == DltEndiannessLittleEndian) {
            messageId = (*((unsigned int*) payload.data()));
        }
        else {
            messageId = DLT_SWAP_32((*((unsigned int*) payload.data())));
        }
    }

    /* set service id if message of type control */
    if((type == DltTypeControl) && payload.size()>=4) {
        if(endianness == DltEndiannessLittleEndian)
            ctrlServiceId = *((unsigned int*) payload.data());
        else
            ctrlServiceId = DLT_SWAP_32(*((unsigned int*) payload.data()));
    }

    /* set return type if message of type control response */
    if((type == QDltMsg::DltTypeControl) && (subtype == QDltMsg::DltControlResponse) && payload.size()>=6) {
        ctrlReturnType = *((unsigned char*) &(payload.data()[4]));
    }

    /* get the arguments of the payload */
    if(mode==DltModeVerbose) {
        offset = 0;
        arguments.clear();
        for(int num=0;num<numberOfArguments;num++) {
            if(argument.setArgument(payload,offset,endianness)==false) {
                /* There was an error parsing the arguments */
                return false;
            }
            arguments.append(argument);
        }
    }

    return true;
}

bool QDltMsg::getMsg(QByteArray &buf,bool withStorageHeader) {
    DltStorageHeader storageheader;
    DltStandardHeader standardheader;
    DltStandardHeaderExtra headerextra;
    DltExtendedHeader extendedheader;

    /* empty return buffer */
    buf.clear();

    /* prepare payload */
    payload.clear();
    for (int num = 0;num<arguments.size();num++)
    {
        if(!(arguments[num].getArgument(payload,mode==DltModeVerbose)))
            return false;
    }

    /* write storageheader */
    if(withStorageHeader)
    {
        storageheader.pattern[0] = 'D';
        storageheader.pattern[1] = 'L';
        storageheader.pattern[2] = 'T';
        storageheader.pattern[3] = 0x01;
        strncpy(storageheader.ecu,ecuid.toAscii().data(),ecuid.size()>3?4:ecuid.size()+1);
        storageheader.microseconds = microseconds;
        storageheader.seconds = time.toTime_t();
        buf += QByteArray((const char *)&storageheader,sizeof(DltStorageHeader));
    }

    /* write standardheader */
    standardheader.htyp = 0x01 << 5; /* intialise with version number 0x1 */
    if(endianness == DltEndiannessBigEndian) {
        standardheader.htyp |= DLT_HTYP_MSBF;
    }
    if(mode == DltModeVerbose) {
        standardheader.htyp |= DLT_HTYP_UEH;
        standardheader.htyp |= DLT_HTYP_WEID;
        standardheader.htyp |= DLT_HTYP_WSID;
        standardheader.htyp |= DLT_HTYP_WTMS;
        standardheader.len = DLT_SWAP_16(sizeof(DltStandardHeader) + sizeof(headerextra.ecu) + sizeof(headerextra.seid) +
                                     sizeof(headerextra.tmsp) + sizeof(DltExtendedHeader) + payload.size());
    }
    else {
        standardheader.len = DLT_SWAP_16(sizeof(DltStandardHeader) + payload.size());
    }
    standardheader.mcnt = messageCounter;
    buf += QByteArray((const char *)&standardheader,sizeof(DltStandardHeader));

    /* write standard header extra */
    if(mode == DltModeVerbose) {
        strncpy(headerextra.ecu,ecuid.toAscii().data(),ecuid.size()>3?4:ecuid.size()+1);
        buf += QByteArray((const char *)&(headerextra.ecu),sizeof(headerextra.ecu));
        headerextra.seid = DLT_SWAP_32(sessionid);
        buf += QByteArray((const char *)&(headerextra.seid),sizeof(headerextra.seid));
        headerextra.tmsp = DLT_SWAP_32(timestamp);
        buf += QByteArray((const char *)&(headerextra.tmsp),sizeof(headerextra.tmsp));
    }

    /* write extendedheader */
    if(mode == DltModeVerbose) {
        strncpy(extendedheader.apid,apid.toAscii().data(),apid.size()>3?4:apid.size()+1);
        strncpy(extendedheader.ctid,ctid.toAscii().data(),ctid.size()>3?4:ctid.size()+1);
        extendedheader.msin = 0;
        if(mode == DltModeVerbose) {
            extendedheader.msin |= DLT_MSIN_VERB;
        }
        extendedheader.msin |= (((unsigned char)type) << 1) & DLT_MSIN_MSTP;
        extendedheader.msin |= (((unsigned char)subtype) << 4) & DLT_MSIN_MTIN;
        extendedheader.noar = numberOfArguments;
        buf += QByteArray((const char *)&extendedheader,sizeof(DltExtendedHeader));
    }

    /* write payload */
    buf += payload;

    return true;
}


void QDltMsg::clear()
{
    ecuid.clear();
    apid.clear();
    ctid.clear();
    type = DltTypeUnknown;
    subtype = DltLogUnknown;
    mode = DltModeUnknown;
    endianness = DltEndiannessUnknown;
    time = QDateTime();
    microseconds = 0;
    timestamp = 0;
    sessionid = 0;
    numberOfArguments = 0;
    messageId = 0;
    ctrlServiceId = 0;
    ctrlReturnType = 0;
    arguments.clear();
}

void QDltMsg::clearArguments()
{
    arguments.clear();
}

int QDltMsg::sizeArguments()
{
    return arguments.size();
}

bool QDltMsg::getArgument(int index,QDltArgument &argument)
{
      if(index<0 || index>=arguments.size())
          return false;

      argument = arguments.at(index);

      return true;
}

void QDltMsg::addArgument(QDltArgument argument, int index)
{
    if(index == -1)
        arguments.append(argument);
    else
        arguments.insert(index,argument);
}

void QDltMsg::removeArgument(int index)
{
    arguments.removeAt(index);
}

QString QDltMsg::toStringHeader()
{
    QString text;

    text += QString("%1.%2").arg(getTime().toString("yyyy/MM/dd hh:mm:ss")).arg(getMicroseconds(),6,10,QLatin1Char('0'));
    text += QString(" %1.%2").arg(getTimestamp()/10000).arg(getTimestamp()%10000,5,10,QLatin1Char('0'));
    text += QString(" %1").arg(getMessageCounter());
    text += QString(" %1").arg(getEcuid());
    text += QString(" %1").arg(getApid());
    text += QString(" %1").arg(getCtid());
    text += QString(" %2").arg(getTypeString());
    text += QString(" %2").arg(getSubtypeString());
    text += QString(" %2").arg(getModeString());
    text += QString(" %1").arg(getNumberOfArguments());

    return text;
}

QString QDltMsg::toStringPayload()
{
    QString text;
    QDltArgument argument;
    QByteArray data;

    if((getMode()==QDltMsg::DltModeNonVerbose) && (getType()!=QDltMsg::DltTypeControl) && (getNumberOfArguments() == 0)) {
        text += QString("[%1] ").arg(getMessageId());
        data = payload.mid(4,(payload.size()>260)?256:(payload.size()-4));
        //text += toAscii(data);
        //text += toAsciiTable(data,false,false,true,8,64,false);
        if(!data.isEmpty())
        {
            text += toAscii(data, true);
            text += "|";
            text += toAscii(data, false);
        }
        return text;
    }

    if( getType()==QDltMsg::DltTypeControl && getSubtype()==QDltMsg::DltControlResponse) {
        text += QString("[%1 %2] ").arg(getCtrlServiceIdString()).arg(getCtrlReturnTypeString());
        data = payload.mid(6,(payload.size()>262)?256:(payload.size()-6));
        text += toAscii(data);

        return text;
    }

    if( getType()==QDltMsg::DltTypeControl) {
        text += QString("[%1] ").arg(getCtrlServiceIdString());
        data = payload.mid(4,(payload.size()>260)?256:(payload.size()-4));
        text += toAscii(data);

        return text;
    }

    for(int num=0;num<arguments.size();num++) {
        if(getArgument(num,argument)) {
            if(num!=0) {
                text += " ";
            }
            text += argument.toString();
        }

    }

    return text;
}

QDltFile::QDltFile()
{
    filterFlag = false;
}

QDltFile::~QDltFile()
{
    if(infile.isOpen()) {
        infile.close();
    }
}

int QDltFile::size()
{
    //return file->counter;
    return indexAll.size();
}

int QDltFile::sizeFilter()
{
    if(filterFlag)
        return indexFilter.size();
    else
        return indexAll.size();
}

bool QDltFile::open(QString _filename) {

    /* check if file is already opened */
    if(infile.isOpen()) {
        qWarning() << "open: file is already open";
        infile.close();
    }

    /* set new filename */
    infile.setFileName(_filename);

    /* open the log file read only */
    if(infile.open(QIODevice::ReadOnly)==false) {
        /* open file failed */
        qWarning() << "open of file" << _filename << "failed";
        return false;
    }

    qDebug() << "Open file" << _filename << "started";

    /* create the index for the log file */
    if(!createIndex()) {
        /* index creation failed */
        infile.close();
        return false;
    }

    qDebug() << "Open file" << _filename << "finished";
    qDebug() << indexAll.size() << "messages found";

    /* Success */
    return true;
}

bool QDltFile::createIndex()
{

    /* check if file is already opened */
    if(!infile.isOpen()) {
        /* return empty buffer */
        qDebug() << "createIndex: Infile is not open";
        return false;
    }

    /* clear old index */
    indexAll.clear();

    return updateIndex();
}

bool QDltFile::updateIndex()
{
    QByteArray buf;
    int found = 0;
    unsigned long pos = 0;
    int lastSize = 0;

    /* check if file is already opened */
    if(!infile.isOpen()) {
        qDebug() << "updateIndex: Infile is not open";
        return false;
    }

    /* start at last found position */
    if(indexAll.size()) {
        /* move behind last found position */
        pos = indexAll[indexAll.size()-1] + 4;
        infile.seek(pos);
        lastSize = indexAll.size();
    }
    else {
        /* the file was empty the last call */
        infile.seek(0);
    }

    /* walk through the whole file and find all DLT0x01 markers */
    /* store the found positions in the indexAll */
    while(true) {

        /* read buffer from file */
        buf = infile.read(1000);

        /* check if data was read */
        if(buf.isEmpty())
            break;

        /* find marker in buffer */
        for(int num=0;num<buf.size();num++) {
            if((found == 0) && (buf[num]=='D'))
                found = 1;
            else if((found == 1) && (buf[num]=='L'))
                found = 2;
            else if((found == 2) && (buf[num]=='T'))
                found = 3;
            else if((found == 3) && (buf[num]==(char)0x01)) {
                indexAll.append(pos+num-3);
                found = 0;
            }
            else
                found = 0;

        }
        pos += 1000;
    }

    /* success */
    return true;
}

bool QDltFile::createIndexFilter()
{
    /* clear old index */
    indexFilter.clear();

    return updateIndexFilter();
}

bool QDltFile::updateIndexFilter()
{
    QDltMsg msg;
    QByteArray buf;
    int index;

    /* update index filter by starting from last found index in list */

    /* get lattest found index in filter list */
    if(indexFilter.size()>0) {
        index = indexFilter[indexFilter.size()-1] + 1;
    }
    else {
        index = 0;
    }

    for(int num=index;num<indexAll.size();num++) {
        buf = getMsg(num);
        if(!buf.isEmpty()) {
            msg.setMsg(buf);
            if(checkFilter(msg)) {
                indexFilter.append(num);
            }
        }

    }

    return true;
}

bool QDltFile::checkFilter(QDltMsg &msg)
{
    QDltFilter filter;
    bool found = false, foundFilter;

    if(pfilter.isEmpty())
        found = true;
    else
        found = false;

    for(int numfilter=0;numfilter<pfilter.size();numfilter++)
    {
        filter = pfilter[numfilter];

        foundFilter = true;
        if(filter.enableEcuid && ( msg.getEcuid() != filter.ecuid)) {
            foundFilter = false;
        }
        if(filter.enableApid && (msg.getApid() != filter.apid)) {
            foundFilter = false;
        }
        if(filter.enableCtid && (msg.getCtid() != filter.ctid)) {
            foundFilter = false;
        }
        if(filter.enableHeader && !(msg.toStringHeader().contains(filter.header))) {
            foundFilter = false;
        }
        if(filter.enablePayload && !(msg.toStringPayload().contains(filter.payload))) {
            foundFilter = false;
        }
        if(filter.enableCtrlMsgs && !((msg.getType() == QDltMsg::DltTypeControl))) {
            foundFilter = false;
        }
        if(filter.enableLogLevelMax && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() <= filter.logLevelMax))) {
            foundFilter = false;
        }
        if(filter.enableLogLevelMin && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() >= filter.logLevelMin))) {
            foundFilter = false;
        }

        if(foundFilter)
            found = true;

    }
    for(int numfilter=0;numfilter<nfilter.size();numfilter++)
    {
        filter = nfilter[numfilter];

        foundFilter = true;
        if(filter.enableEcuid && ( msg.getEcuid() != filter.ecuid)) {
            foundFilter = false;
        }
        if(filter.enableApid && (msg.getApid() != filter.apid)) {
            foundFilter = false;
        }
        if(filter.enableCtid && (msg.getCtid() != filter.ctid)) {
            foundFilter = false;
        }
        if(filter.enableHeader && !(msg.toStringHeader().contains(filter.header))) {
            foundFilter = false;
        }
        if(filter.enablePayload && !(msg.toStringPayload().contains(filter.payload))) {
            foundFilter = false;
        }
        if(filter.enableCtrlMsgs && !((msg.getType() == QDltMsg::DltTypeControl))) {
            foundFilter = false;
        }
        if(filter.enableLogLevelMax && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() <= filter.logLevelMax))) {
            foundFilter = false;
        }
        if(filter.enableLogLevelMin && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() >= filter.logLevelMin))) {
            foundFilter = false;
        }

        if(foundFilter)
            found = false;
    }

    return found;
}

void QDltFile::clearFilterIndex()
{
    /* clear old index */
    indexFilter.clear();

}

void QDltFile::addFilterIndex (int index)
{
    indexFilter.append(index);

}

int QDltFile::checkMarker(QDltMsg &msg)
{
    QDltFilter filter;
    bool found = false, foundFilter;
    int colour = 0;

    for(int numfilter=0;numfilter<marker.size();numfilter++)
    {
        filter = marker[numfilter];

        foundFilter = true;
        if(filter.enableEcuid && ( msg.getEcuid() != filter.ecuid)) {
            foundFilter = false;
        }
        if(filter.enableApid && (msg.getApid() != filter.apid)) {
            foundFilter = false;
        }
        if(filter.enableCtid && (msg.getCtid() != filter.ctid)) {
            foundFilter = false;
        }
        if(filter.enableHeader && !(msg.toStringHeader().contains(filter.header))) {
            foundFilter = false;
        }
        if(filter.enablePayload && !(msg.toStringPayload().contains(filter.payload))) {
            foundFilter = false;
        }
        if(filter.enableCtrlMsgs && !((msg.getType() == QDltMsg::DltTypeControl))) {
            foundFilter = false;
        }
        if(filter.enableLogLevelMax && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() <= filter.logLevelMax))) {
            foundFilter = false;
        }
        if(filter.enableLogLevelMin && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() >= filter.logLevelMin))) {
            foundFilter = false;
        }

        if(foundFilter)
        {
            found = true;
            colour = filter.filterColour;
        }
    }

    /* marker was found; return colour of marker */
    if(found)
        return (colour + 1);

    return found;
}

void QDltFile::close()
{
    /* close file */
    infile.close();
}

QByteArray QDltFile::getMsg(int index)
{
    QByteArray buf;

    /* check if file is already opened */
    if(!infile.isOpen()) {
        /* return empty buffer */
        qDebug() << "getMsg: Infile is not open";

        /* return empty data buffer */
        return QByteArray();
    }

    /* check if index is in range */
    if(index<0 || index>=indexAll.size()) {
        qDebug() << "getMsg: Index is out of range";

        /* return empty data buffer */
        return QByteArray();
    }

    /* move to file position selected by index */
    infile.seek(indexAll[index]);

    /* read DLT message from file */
    if(index == (indexAll.size()-1))
        /* last message in file */
        buf = infile.read(infile.size()-indexAll[index]);
    else
        /* any other file position */
        buf = infile.read(indexAll[index+1]-indexAll[index]);

    /* return DLT message buffer */
    return buf;
}

bool QDltFile::getMsg(int index,QDltMsg &msg)
{
    QByteArray data;

    data = getMsg(index);

    if(data.isEmpty())
        return false;

    return msg.setMsg(data);
}

QByteArray QDltFile::getMsgFilter(int index)
{
    if(filterFlag) {
        /* check if index is in range */
        if(index<0 || index>=indexFilter.size()) {
            qDebug() << "getMsgFilter: Index is out of range";

            /* return empty data buffer */
            return QByteArray();
        }
        return getMsg(indexFilter[index]);
    }
    else {
        /* check if index is in range */
        if(index<0 || index>=indexAll.size()) {
            qDebug() << "getMsgFilter: Index is out of range";

            /* return empty data buffer */
            return QByteArray();
        }
        return getMsg(index);
    }
}

void QDltFile::clearFilter()
{
    pfilter.clear();
    nfilter.clear();
    marker.clear();
    qDebug() << "clearFilter: Clear filter";
}

void QDltFile::addPFilter(QDltFilter &_filter)
{
    pfilter.append(_filter);
    qDebug() << "addPFilter: Add Filter" << _filter.apid << _filter.ctid;
}

void QDltFile::addNFilter(QDltFilter &_filter)
{
    nfilter.append(_filter);
    qDebug() << "addNFilter: Add Filter" << _filter.apid << _filter.ctid;
}

void QDltFile::addMarker(QDltFilter &_filter)
{
    marker.append(_filter);
    qDebug() << "addMarker: Add Filter" << _filter.apid << _filter.ctid;
}

bool QDltFile::isFilter()
{
    return filterFlag;
}

void QDltFile::enableFilter(bool state)
{
    filterFlag = state;
}

QDltConnection::QDltConnection()
{
    sendSerialHeader = false;
    syncSerialHeader = false;
}

QDltConnection::~QDltConnection()
{

}

void QDltConnection::setSendSerialHeader(bool _sendSerialHeader)
{
    sendSerialHeader = _sendSerialHeader;
}

bool QDltConnection::getSendSerialHeader()
{
    return sendSerialHeader;
}

void QDltConnection::setSyncSerialHeader(bool _syncSerialHeader)
{
    syncSerialHeader = _syncSerialHeader;
}

bool QDltConnection::getSyncSerialHeader()
{
    return syncSerialHeader;
}

QDltTCPConnection::QDltTCPConnection()
    : QDltConnection()
{

}

QDltTCPConnection::~QDltTCPConnection()
{
    hostname = "localhost";
    tcpport = DLT_DAEMON_TCP_PORT;
}

void QDltTCPConnection::setHostname(QString _hostname)
{
    hostname = _hostname;
}

QString QDltTCPConnection::getHostname()
{
    return hostname;
}

void QDltTCPConnection::setTcpPort(unsigned int _tcpport)
{
    tcpport = _tcpport;
}

void QDltTCPConnection::setDefaultTcpPort()
{
    tcpport = DLT_DAEMON_TCP_PORT;
}

unsigned int QDltTCPConnection::getTcpPort()
{
    return tcpport;
}

QDltSerialConnection::QDltSerialConnection()
    : QDltConnection()
{
    port = "";
    baudrate = 0;

    serialport = 0;
}

QDltSerialConnection::~QDltSerialConnection()
{

}

void QDltSerialConnection::setPort(QString _port)
{
    port = _port;
}

QString QDltSerialConnection::getPort()
{
    return port;
}

void QDltSerialConnection::setBaudrate(int _baudrate)
{
    baudrate = _baudrate;
}

unsigned int QDltSerialConnection::getBaudrate()
{
    return baudrate;
}
