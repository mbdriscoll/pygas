#ifndef _PYGAS_PIPELINE_H_
#define _PYGAS_PIPELINE_H_

#include "pygas.h"

#ifdef __cplusplus
extern "C" {
#endif

int pygas_register_fragment(msg_info_t* msg_info, char* fragment,
        char** msg, int* nbytes);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* define _PYGAS_PIPELINE_H_ */
