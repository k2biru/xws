#ifndef __ERROR_CODE_H
#define __ERROR_CODE_H

#define OPRT_RET    int

#define OPRT_OK                                 (1)
#define OPRT_ERR_INVALID_ACCESS                 (-1)
#define OPRT_ERR_MALLOC_FAILED                  (-2)
#define OPRT_ERR_NVS_FAILED                     (-3)
#define OPRT_ERR_TABLE_FULL                     (-4)
#define OPRT_ERR_TABLE_EMPTY                    (-5)
#define OPRT_ERR_JSON_PARSE                     (-6)
#define OPRT_ERR_INVALID_ARG                    (-7)
#define OPRT_ERR_UNKNOW_CB                      (-8)
#define OPRT_ERR_INVALID_OPERTATION             (-9)
#define OPRT_ERR_KEY                            (-10)
#define OPRT_ERR_SIZE_MAX                       (-11)
#define OPRT_ERR_NOT_FOUND                      (-12)

#endif //__ERROR_CODE_H