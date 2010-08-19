/*
 * il.h - data structures for intermediate languages
 *
 * Copyright 2010 Rui Ueyama <rui314@gmail.com>.  All rights reserved.
 * This code is available under the simplified BSD license.  See LICENSE for details.
 */

#include "8cc.h"

struct Exp;

/*==============================================================================
 * C data type
 */

typedef enum {
    TVOID,
    TARRAY,
    TPTR,
    TFUNC,
    TINT,
    TFLOAT,
    TSTRUCT,
    TUNION,
    TENUM,
} TypeEnum;

typedef enum {
    ICHAR, IUCHAR,
    ISHORT, IUSHORT,
    IINT, IUINT,
    ILONG, IULONG,
    ILLONG, IULLONG,
} IntKind;

typedef enum {
    FFLOAT,
    FDOUBLE,
    FLDOUBLE,
} FloatKind;

#define TYPE_HEADER TypeEnum type;

typedef struct Type {
    TYPE_HEADER;
} Type;

typedef struct ArrayType {
    TYPE_HEADER;
    Type *ptr;
    struct Exp *size;
    // C99 6.7.5.3p12 Function declarators (including prototypes)
    bool is_star;
} ArrayType;

typedef struct PtrType {
    TYPE_HEADER;
    Type *ptr;
} PtrType;

typedef struct FuncType {
    TYPE_HEADER;
    Type *ret;
    // List of Types
    List *param;
    // List of NVars.  Used only during reading a declarator.
    List *vars;
    // True iff the parameter list of the function ends with "...".
    bool ellipsis;
} FuncType;

typedef struct IntType {
    TYPE_HEADER;
    IntKind kind;
} IntType;

typedef struct FloatType {
    TYPE_HEADER;
    FloatKind kind;
} FloatType;

typedef struct VoidType {
    TYPE_HEADER;
} VoidType;

// Structure fields.
typedef struct Field {
    Type *ctype;
    String *name;
    // If a bitfield, "bitfield" >= 0.  -1 indicate that this is not a bitfield.
    int bitfield;
} Field;

typedef struct StructType {
    TYPE_HEADER;
    String *name;
    // List of Fields
    List *field;
} StructType;

typedef StructType UnionType;

typedef struct EnumType {
    TYPE_HEADER;
    String *name;
    // Map from String to Exp
    Dict *item;
} EnumType;

#define ARRAY_TYPE(obj)  ((ArrayType *)(obj))
#define PTR_TYPE(obj)    ((PtrType *)(obj))
#define FUNC_TYPE(obj)   ((FuncType *)(obj))
#define INT_TYPE(obj)    ((IntType *)(obj))
#define FLOAT_TYPE(obj)  ((FloatType *)(obj))
#define VOID_TYPE(obj)   ((VoidType *)(obj))
#define STRUCT_TYPE(obj) ((StructType *)(obj))
#define UNION_TYPE(obj)  ((UnionType *)(obj))
#define ENUM_TYPE(obj)   ((EnumType *)(obj))

extern Type *make_array_type(Type *ptr, struct Exp *size);
extern Type *make_ptr_type(Type *ptr);
extern Type *make_func_type(Type *ret, List *param);
extern Type *make_int_type(IntKind kind);
extern Type *make_float_type(FloatKind kind);
extern Type *make_void_type(void);
extern Type *make_struct_or_union_type(TypeEnum type, String *name, List *field);
extern Type *make_enum_type(String *name, Dict *item);

/*==============================================================================
 * Data type for Node
 */

typedef enum {
    EADDROF,
    EBINOP,
    ECAST,
    ECONST,
    ELVAL,
    ESTARTOF,
    ESIZEOFTYPE,
    EUNOP,
    ICALL,
    ISET,
    SGOTO,
    SIF,
    SINSTR,
    SLOOP,
    SRETURN,
    FUNCTION,
} NodeType;

#define NODE_HEADER NodeType type
#define NODE_TYPE(obj) ((obj)->type)
#define NODE(obj) ((Node *)(obj))

typedef struct Node {
    NODE_HEADER;
} Node;


/*==============================================================================
 * Variables
 */

typedef enum {
    NO_STORAGE,
    STATIC,
    REGISTER,
    EXTERN
} Storage;

typedef enum {
    GLOBAL,
    LOCAL,
} VarType;

typedef struct NVar {
    VarType type;
    Storage storage;
    Type *ctype;
    String *name;
} NVar;

extern NVar *make_nvar(VarType vartype, Storage st, Type *ctype, String *name);


/*==============================================================================
 * Expressions
 */

#define EXP_HEADER NodeType type; Type *ctype

typedef struct Exp {
    EXP_HEADER;
} Exp;

#define EXP_CTYPE(e) ((e)->ctype)

/*
 * Lvalue
 */

typedef struct LvalBase {
    enum { LVAL_VAR, LVAL_MEM } type;
    void *p; // Var or Exp
} LvalBase;

typedef struct LvalOff {
    enum { FIELD, INDEX } type;
    Exp *exp;
    struct LvalOff *off;
} LvalOff;

typedef struct LvalExp {
    EXP_HEADER;
    LvalBase base;
    LvalOff *off;
} LvalExp;

#define LVAL_BASE_TYPE(e) ((e)->base.type)
#define LVAL_BASE_VAR(e) ((NVar *)((e)->base.p))
#define LVAL_BASE_EXP(e) ((Exp *)((e)->base.p))

#define LVAL_OFF_TYPE(e) ((e)->off.type)
#define LVAL_OFF_INDEX(e) ((e)->off.exp);
#define LVAL_OFF_OFF(e) ((e)->off.off);

extern LvalOff *make_lval_off(int type, Exp *exp);
extern Exp *make_lval_exp(Type *ctype, LvalBase base);

/*
 * Other expressions
 */
typedef struct ConstExp {
    EXP_HEADER;
    union {
        double f;
        long i;
    } v;
} ConstExp;

typedef struct UnopExp {
    EXP_HEADER;
    int op;
    Exp *exp;
} UnopExp;

typedef struct AddrOfExp {
    EXP_HEADER;
    LvalExp *lval;
} AddrOfExp;

typedef struct BinopExp {
    EXP_HEADER;
    int op;
    Exp *exp0;
    Exp *exp1;
} BinopExp;

typedef struct StartOfExp {
    EXP_HEADER;
    LvalExp *lval;
} StartOfExp;

typedef struct SizeOfTypeExp {
    EXP_HEADER;
    Type *argtype;
} SizeOfTypeExp;

typedef struct CastExp {
    EXP_HEADER;
    Exp *exp;
} CastExp;

#define EXP(obj)              ((Exp *)(obj))
#define LVAL_EXP(obj)         ((LvalExp *)(obj))
#define CONST_EXP(obj)        ((ConstExp *)(obj))
#define UNOP_EXP(obj)         ((UnopExp *)(obj))
#define ADDR_OF_EXP(obj)      ((AddrOfExp *)(obj))
#define BINOP_EXP(obj)        ((BinopExp *)(obj))
#define START_OF_EXP(obj)     ((StartOfExp *)(obj))
#define SIZE_OF_TYPE_EXP(obj) ((SizeOfTypeExp *)(obj))
#define CAST_EXP(obj)         ((CastExp *)(obj))

extern Exp *make_const_int(Type *ctype, long i);
extern Exp *make_const_float(Type *ctype, double f);
extern Exp *make_unop_exp(Type *ctype, int op, Exp *exp);
extern Exp *make_addrof_exp(Type *ctype, LvalExp *lval);
extern Exp *make_binop_exp(Type *ctype, int op, Exp *exp0, Exp *exp1);
extern Exp *make_startof_exp(Type *ctype, LvalExp *lval);
extern Exp *make_sizeoftype_exp(Type *ctype, Type *argtype);
extern Exp *make_cast_exp(Type *ctype, Exp *exp);

/*==============================================================================
 * Instructions
 */

typedef struct SetInstr {
    NODE_HEADER;
    LvalExp *lval;
    Exp *exp;
} SetInstr;

typedef struct CallInstr {
    NODE_HEADER;
    LvalExp *retval;
    LvalExp *fn;
    List *param; // List of Exps
} CallInstr;

#define SET_INSTR(obj)  ((SetInstr *)(obj))
#define CALL_INSTR(obj) ((CallInstr *)(obj))

extern Node *make_set_instr(LvalExp *lval, Exp *exp);
extern Node *make_call_instr(LvalExp *retval, LvalExp *fn, List *param);


/*==============================================================================
 * Statement
 */

#define STMT_HEADER NODE_HEADER; String *label

typedef struct Stmt {
    STMT_HEADER;
} Stmt;

typedef struct InstrStmt {
    STMT_HEADER;
    List *instr; // List of Instrs
} InstrStmt;

typedef struct GotoStmt {
    STMT_HEADER;
    Stmt *stmt;
} GotoStmt;

typedef struct ReturnStmt {
    STMT_HEADER;
    Exp *exp;
} ReturnStmt;

typedef struct IfStmt {
    STMT_HEADER;
    Exp *exp;
    List *then;
    List *els;
} IfStmt;

typedef struct LoopStmt {
    STMT_HEADER;
    List *stmt;
} LoopStmt;

#define STMT(obj)        ((Stmt *)(obj))
#define INSTR_STMT(obj)  ((InstrStmt *)(obj))
#define GOTO_STMT(obj)   ((GotoStmt *)(obj))
#define RETURN_STMT(obj) ((ReturnStmt *)(obj))
#define IF_STMT(obj)     ((IfStmt *)(obj))
#define LOOP_STMT(obj)   ((LoopStmt *)(obj))

extern Node *make_instr_stmt(List *instr);
extern Node *make_goto_stmt(Stmt *stmt);
extern Node *make_return_stmt(Exp *exp);
extern Node *make_if_stmt(Exp *exp, List *then, List *els);
extern Node *make_loop_stmt(List *stmt);

/*==============================================================================
 * Function
 */

typedef struct NFunction {
    NODE_HEADER;
    Type *ctype;
    String *name;
    List *param;
    // List of local variables
    List *var;
    // Entry block containing list of instr
    List *stmt;
} NFunction;

extern NFunction *make_nfunction(String *name, Type *ctype, List *param, List *var, List *stmt);

/*==============================================================================
 * Global definitions
 */

typedef struct Global {
    // List of NVars
    List *var;
    // List of NFunctions
    List *func;
} Global;

/*==============================================================================
 * Pretty printer
 */

extern String *pp_type(Type *ctype);
extern String *pp_var(NVar *var);
extern String *pp_node(Node *node);
extern String *pp_nfunction(NFunction *fn);