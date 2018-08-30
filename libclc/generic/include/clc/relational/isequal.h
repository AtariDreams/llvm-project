#define _CLC_ISEQUAL_DECL(TYPE, RETTYPE) \
  _CLC_OVERLOAD _CLC_DECL RETTYPE isequal(TYPE x, TYPE y);

#define _CLC_VECTOR_ISEQUAL_DECL(TYPE, RETTYPE) \
  _CLC_ISEQUAL_DECL(TYPE##2, RETTYPE##2)  \
  _CLC_ISEQUAL_DECL(TYPE##3, RETTYPE##3)  \
  _CLC_ISEQUAL_DECL(TYPE##4, RETTYPE##4)  \
  _CLC_ISEQUAL_DECL(TYPE##8, RETTYPE##8)  \
  _CLC_ISEQUAL_DECL(TYPE##16, RETTYPE##16)

_CLC_ISEQUAL_DECL(float, int)
_CLC_VECTOR_ISEQUAL_DECL(float, int)

#ifdef cl_khr_fp64
_CLC_ISEQUAL_DECL(double, int)
_CLC_VECTOR_ISEQUAL_DECL(double, long)
#endif
#ifdef cl_khr_fp16
_CLC_ISEQUAL_DECL(half, int)
_CLC_VECTOR_ISEQUAL_DECL(half, short)
#endif

#undef _CLC_ISEQUAL_DECL
#undef _CLC_VECTOR_ISEQUAL_DEC
