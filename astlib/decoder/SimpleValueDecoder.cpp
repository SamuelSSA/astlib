///
/// \package astlib
/// \file SimpleValueDecoder.cpp
///
/// \author Marian Krivos <nezmar@tutok.sk>
/// \date 13. 2. 2017 - 18:47:22
///
/// (C) Copyright 2017 R-SYS,s.r.o
/// All rights reserved.
///

#include "SimpleValueDecoder.h"

namespace astlib
{

SimpleValueDecoder::SimpleValueDecoder()
{
}

SimpleValueDecoder::~SimpleValueDecoder()
{
}

void SimpleValueDecoder::begin(int cat)
{
    _msg = std::make_shared<SimpleAsterixRecord>();
    _msg->setCategory(cat);
}

void SimpleValueDecoder::beginItem(const ItemDescription& uapItem)
{
}

void SimpleValueDecoder::beginRepetitive(size_t size)
{
}

void SimpleValueDecoder::beginArray(AsterixItemCode code, size_t size)
{
    _msg->initializeArray(code, size);
}

void SimpleValueDecoder::decodeBoolean(const CodecContext& context, bool value, int index)
{
    _msg->setItem(context.bits.code, std::move(Poco::Dynamic::Var(value)), index);
}

void SimpleValueDecoder::decodeSigned(const CodecContext& context, Poco::Int64 value, int index)
{
    _msg->setItem(context.bits.code, std::move(Poco::Dynamic::Var(value)), index);
}

void SimpleValueDecoder::decodeUnsigned(const CodecContext& context, Poco::UInt64 value, int index)
{
    _msg->setItem(context.bits.code, std::move(Poco::Dynamic::Var(value)), index);
}

void SimpleValueDecoder::decodeReal(const CodecContext& context, double value, int index)
{
    _msg->setItem(context.bits.code, std::move(Poco::Dynamic::Var(value)), index);
}

void SimpleValueDecoder::decodeString(const CodecContext& context, const std::string& value, int index)
{
    _msg->setItem(context.bits.code, std::move(Poco::Dynamic::Var(value)), index);
}

void SimpleValueDecoder::end()
{
    onMessageDecoded(_msg);
    _msg = nullptr;
}

} /* namespace astlib */
