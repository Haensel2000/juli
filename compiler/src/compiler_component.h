//
//  compiler_component.h
//  juli
//
//  Created by Hannes Widmoser on 1/1/13.
//
//

#ifndef __juli__compiler_component__
#define __juli__compiler_component__

#include <vector>
#include <analysis/error.h>

namespace juli {
    
    class CompilerComponent {
    protected:
        std::vector<Error>& errors;
    public:
        
        CompilerComponent(std::vector<Error>& errors);
        
        std::vector<Error>& getErrors() const;
    };
    
}

#endif /* defined(__juli__compiler_component__) */
