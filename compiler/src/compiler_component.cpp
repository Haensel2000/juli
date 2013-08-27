//
//  compiler_component.cpp
//  juli
//
//  Created by Hannes Widmoser on 1/1/13.
//
//

#include "compiler_component.h"

juli::CompilerComponent::CompilerComponent(std::vector<Error>& errors) : errors(errors) {}

std::vector<juli::Error>& juli::CompilerComponent::getErrors() const
{
    return errors;
}