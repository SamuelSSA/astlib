///
/// \package astlib
/// \file RepetitiveItemDescription.h
///
/// \author Marian Krivos <nezmar@tutok.sk>
/// \date 3Feb.,2017 
///
/// (C) Copyright 2017 R-SYS s.r.o
/// All rights reserved.
///

#pragma once

#include "Fixed.h"
#include "ItemDescription.h"

namespace astlib
{

class ASTLIB_API RepetitiveItemDescription:
    public ItemDescription
{
public:
    RepetitiveItemDescription(int id, const std::string& description, const FixedVector& fixeds);
    virtual ~RepetitiveItemDescription();

    const FixedVector& getFixedVector() const { return _fixedArray; }

private:
    virtual ItemFormat getType() const { return ItemFormat::Repetitive; }

    FixedVector _fixedArray;
};

} /* namespace astlib */

