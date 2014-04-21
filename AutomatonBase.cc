#include "AutomatonBase.h"

#include <stdlib.h>

AutomatonBase::AutomatonBase(AutomatonType type)
    :automataType_(type)
{
    automatonConverter_ = (automataType_ == AutomatonType_NFA)?
        &AutomatonBase::ConvertSynTreeToNFA : &AutomatonBase::ConvertSynTreeToDFA;
}

AutomatonBase::~AutomatonBase()
{
}

int AutomatonBase::ConvertSynTreeToNFA()
{
    // TODO
    return 0;
}

int AutomatonBase::ConvertSynTreeToDFA()
{
    // TODO
    return 0;
}

