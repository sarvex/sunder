// Copyright 2021 The Nova Project Authors
// SPDX-License-Identifier: Apache-2.0
#ifndef NOVA_H_INCLUDED
#define NOVA_H_INCLUDED
#include <stdbool.h>
#include <stdint.h>
#include <autil/autil.h>

////////////////////////////////////////////////////////////////////////////////
//////// nova.c ////////////////////////////////////////////////////////////////
// Global compiler state and miscellaneous utilities.

#if __STDC_VERSION__ >= 201112L /* C11+ */
#    define NORETURN _Noreturn
#elif defined(__GNUC__) /* GCC and Clang */
#    define NORETURN __attribute__((noreturn))
#else
#    define NORETURN /* nothing */
#endif

// Returns a pointer to the first character of the line containing ptr in the
// NUL-terminated source.
char const*
source_line_start(char const* source, char const* ptr);
// Returns a pointer to the end-of-line newline or NUL of the line containing
// ptr in the NUL-terminated source.
char const*
source_line_end(char const* source, char const* ptr);

#ifndef ENABLE_DEBUG
#    define ENABLE_DEBUG 0 /* set to a non-zero value to enable debug output */
#endif
#define NO_PATH ((char const*)NULL)
#define NO_LINE ((size_t)0u)
#define NO_PSRC ((char const*)NULL)
#define NO_LOCATION ((struct source_location const*)NULL)
struct source_location {
    // Optional (NULL indicates no value).
    char const* path;
    // Optional (zero indicates no value).
    size_t line;
    // Optional (NULL indicates no value) pointer to the source character within
    // the module specified by path. If non-NULL then a log-messages will
    // display the line in question with a caret pointing to this character as
    // such:
    // ```
    // [file.nova:3] error: foo is not properly frobnicated
    // var foo: usize = 123u;
    //     ^
    // ```
    char const* psrc;
};

void
debug(struct source_location const* location, char const* fmt, ...);
void
error(struct source_location const* location, char const* fmt, ...);
NORETURN void
fatal(struct source_location const* location, char const* fmt, ...);

NORETURN void
todo(char const* file, int line, char const* fmt, ...);
#define TODO(...) todo(__FILE__, __LINE__, __VA_ARGS__)

NORETURN void
unreachable(char const* file, int line);
#define UNREACHABLE() unreachable(__FILE__, __LINE__)

// Round up to the nearest multiple of 8.
int
ceil8i(int x);
size_t
ceil8zu(size_t x);

// Convert a bigint to a uint8_t.
// Returns zero on success.
// Returns non-zero if the provided bigint is out-of-range, in which case *res
// is left unmodified.
int
bigint_to_u8(uint8_t* res, struct autil_bigint const* bigint);
// Convert a bigint to a size_t.
// Returns zero on success.
// Returns non-zero if the provided bigint is out-of-range, in which case *res
// is left unmodified.
int
bigint_to_uz(size_t* res, struct autil_bigint const* bigint);
// Convert a bigint to a uintmax_t.
// Returns zero on success.
// Returns non-zero if the provided bigint is out-of-range, in which case *res
// is left unmodified.
int
bigint_to_umax(uintmax_t* res, struct autil_bigint const* bigint);
// Convert a bigint into a two's complement bit array.
// Returns zero on success.
// Returns non-zero if the provided bigint is out-of-range would require more
// than autil_bitarr_count(res) bits to express, in which case *res is left
// unmodified.
int
bigint_to_bitarr(struct autil_bitarr* res, struct autil_bigint const* bigint);

// Convert a size_t to a bigint.
// The result bigint must be pre-initialized.
void
uz_to_bigint(struct autil_bigint* res, size_t uz);
// Convert a two's complement bit array into a bigint.
// The result bigint must be pre-initialized.
void
bitarr_to_bigint(
    struct autil_bigint* res,
    struct autil_bitarr const* bitarr,
    bool is_signed);

// Spawn a subprocess and wait for it to complete.
// Returns the exit status of the spawned process.
int
spawnvpw(char const* path, char const* const* argv);
// Spawn a subprocess and wait for it to complete.
// Fatally exits if the exit status of the spawned process is non-zero.
void
xspawnvpw(char const* path, char const* const* argv);

char const* // interned
canonical_path(char const* path);
char const* // interned
directory_path(char const* path);

struct module {
    bool loaded; // True if the module has been fully loaded / resolved.
    char const* path; // interned
    char const* source;

    // Global symbols.
    struct symbol_table* symbols;
    // Exported symbols declared in this module.
    struct symbol_table* exports;

    // Abstract syntax tree for the module. Initialized to NULL and populated
    // during the parse phase.
    struct ast_module const* ast;
    // List of top level declarations topologically ordered such that
    // declaration with index k does not depend on any declaration with index
    // k+n for all n. Initialized to NULL and populated during the order phase.
    autil_sbuf(struct ast_decl const*) ordered;
};
struct module*
module_new(char const* path);
void
module_del(struct module* self);

struct context {
    // Context-owned automatically managed objects.
    struct autil_freezer* freezer;

    // Interned strings.
    struct autil_sipool* sipool;
    struct {
        // clang-format off
        char const* empty;   // ""
        char const* builtin; // "builtin"
        char const* return_; // "return"
        char const* void_;   // "void"
        char const* bool_;   // "bool"
        char const* u8;      // "u8"
        char const* s8;      // "s8"
        char const* u16;     // "u16"
        char const* s16;     // "s16"
        char const* u32;     // "u32"
        char const* s32;     // "s32"
        char const* u64;     // "u64"
        char const* s64;     // "s64"
        char const* byte;    // "byte"
        char const* usize;   // "usize"
        char const* ssize;   // "ssize"
        char const* y;       // "y"
        char const* u;       // "u"
        char const* s;       // "s"
        // clang-format on
    } interned;

    // Integer (bigint) constants.
    struct autil_bigint const* u8_min;
    struct autil_bigint const* u8_max;
    struct autil_bigint const* s8_min;
    struct autil_bigint const* s8_max;
    struct autil_bigint const* u16_min;
    struct autil_bigint const* u16_max;
    struct autil_bigint const* s16_min;
    struct autil_bigint const* s16_max;
    struct autil_bigint const* u32_min;
    struct autil_bigint const* u32_max;
    struct autil_bigint const* s32_min;
    struct autil_bigint const* s32_max;
    struct autil_bigint const* u64_min;
    struct autil_bigint const* u64_max;
    struct autil_bigint const* s64_min;
    struct autil_bigint const* s64_max;
    struct autil_bigint const* usize_min;
    struct autil_bigint const* usize_max;
    struct autil_bigint const* ssize_min;
    struct autil_bigint const* ssize_max;

    // Language builtins.
    struct {
        struct source_location location;
        struct type const* void_;
        struct type const* bool_;
        struct type const* byte;
        struct type const* u8;
        struct type const* s8;
        struct type const* u16;
        struct type const* s16;
        struct type const* u32;
        struct type const* s32;
        struct type const* u64;
        struct type const* s64;
        struct type const* usize;
        struct type const* ssize;
    } builtin;

    // Map containing all symbols with static storage duration.
#define CONTEXT_STATIC_SYMBOLS_MAP_KEY_TYPE char const*
#define CONTEXT_STATIC_SYMBOLS_MAP_VAL_TYPE struct symbol const*
#define CONTEXT_STATIC_SYMBOLS_MAP_CMP_FUNC autil_cstr_vpcmp
    struct autil_map* static_symbols;

    // Global symbol table.
    struct symbol_table* global_symbol_table;

    // Currently loaded/loading modules.
    // TODO: Maybe make this a map from realpath to module?
    autil_sbuf(struct module*) modules;
};
void
context_init(void);
void
context_fini(void);
struct context const*
context(void);

struct module const*
load_module(char const* path);
struct module const*
lookup_module(char const* path);

////////////////////////////////////////////////////////////////////////////////
//////// lex.c /////////////////////////////////////////////////////////////////

enum token_kind {
    // Keywords
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NOT,
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_NAMESPACE,
    TOKEN_IMPORT,
    TOKEN_VAR,
    TOKEN_CONST,
    TOKEN_FUNC,
    TOKEN_DUMP,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_SYSCALL,
    TOKEN_COUNTOF,
    TOKEN_SIZEOF,
    TOKEN_TYPEOF,
    // Sigils
    TOKEN_EQ, // ==
    TOKEN_NE, // !=
    TOKEN_LE, // <=
    TOKEN_LT, // <
    TOKEN_GE, // >=
    TOKEN_GT, // >
    TOKEN_ASSIGN, // =
    TOKEN_PLUS, // +
    TOKEN_DASH, // -
    TOKEN_STAR, // *
    TOKEN_FSLASH, // /
    TOKEN_TILDE, // ~
    TOKEN_PIPE, // |
    TOKEN_CARET, // ^
    TOKEN_AMPERSAND, // &
    TOKEN_LPAREN, // (
    TOKEN_RPAREN, // )
    TOKEN_LBRACE, // {
    TOKEN_RBRACE, // }
    TOKEN_LBRACKET, // [
    TOKEN_RBRACKET, // ]
    TOKEN_COMMA, // ,
    TOKEN_DOT, // .
    TOKEN_COLON_COLON, // :
    TOKEN_COLON, // :
    TOKEN_SEMICOLON, // ;
    // Identifiers and Non-Keyword Literals
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_BYTES,
    // Meta
    TOKEN_EOF,
};
char const*
token_kind_to_cstr(enum token_kind kind);

struct token {
    char const* start;
    size_t count;
    struct source_location location;

    enum token_kind kind;
    union {
        // TOKEN_INTEGER
        struct {
            struct autil_vstr number;
            struct autil_vstr suffix;
        } integer;
        // TOKEN_BYTES
        // Contains the unescaped bytes of literal.
        struct autil_string const* bytes;
    } data;
};
char*
token_to_new_cstr(struct token const* token);

struct lexer*
lexer_new(struct module* module);
void
lexer_del(struct lexer* self);
struct token const*
lexer_next_token(struct lexer* self);

////////////////////////////////////////////////////////////////////////////////
//////// ast.c /////////////////////////////////////////////////////////////////
// Abstract syntax tree.

struct ast_module {
    struct ast_namespace const* namespace; // optional
    autil_sbuf(struct ast_import const* const) imports;
    autil_sbuf(struct ast_decl const* const) decls;
};
struct ast_module*
ast_module_new(
    struct ast_namespace const* namespace,
    struct ast_import const* const* imports,
    struct ast_decl const* const* decls);

struct ast_namespace {
    struct source_location const* location;
    autil_sbuf(struct ast_identifier const* const) identifiers;
};
struct ast_namespace*
ast_namespace_new(
    struct source_location const* location,
    struct ast_identifier const* const* identifiers);

struct ast_import {
    struct source_location const* location;
    char const* path; // interned
};
struct ast_import*
ast_import_new(struct source_location const* location, char const* path);

struct ast_decl {
    struct source_location const* location;
    char const* name; // interned (from the identifier)
    enum {
        AST_DECL_VARIABLE,
        AST_DECL_CONSTANT,
        AST_DECL_FUNCTION,
    } kind;
    union {
        struct {
            struct ast_identifier const* identifier;
            struct ast_typespec const* typespec;
            struct ast_expr const* expr;
        } variable;
        struct {
            struct ast_identifier const* identifier;
            struct ast_typespec const* typespec;
            struct ast_expr const* expr;
        } constant;
        struct {
            struct ast_identifier const* identifier;
            autil_sbuf(struct ast_parameter const* const) parameters;
            struct ast_typespec const* return_typespec;
            struct ast_block const* body;
        } function;
    } data;
};
struct ast_decl*
ast_decl_new_variable(
    struct source_location const* location,
    struct ast_identifier const* identifier,
    struct ast_typespec const* typespec,
    struct ast_expr const* expr);
struct ast_decl*
ast_decl_new_constant(
    struct source_location const* location,
    struct ast_identifier const* identifier,
    struct ast_typespec const* typespec,
    struct ast_expr const* expr);
struct ast_decl*
ast_decl_new_func(
    struct source_location const* location,
    struct ast_identifier const* identifier,
    struct ast_parameter const* const* paramseters,
    struct ast_typespec const* return_typespec,
    struct ast_block const* body);

struct ast_stmt {
    struct source_location const* location;
    enum ast_stmt_kind {
        AST_STMT_DECL,
        AST_STMT_IF,
        AST_STMT_FOR_RANGE,
        AST_STMT_FOR_EXPR,
        AST_STMT_DUMP,
        AST_STMT_RETURN,
        AST_STMT_ASSIGN,
        AST_STMT_EXPR,
    } kind;
    union {
        struct ast_decl const* decl;
        struct {
            autil_sbuf(struct ast_conditional const* const) conditionals;
        } if_;
        struct {
            struct ast_identifier const* identifier;
            struct ast_expr const* begin;
            struct ast_expr const* end;
            struct ast_block const* body;
        } for_range;
        struct {
            struct ast_expr const* expr;
            struct ast_block const* body;
        } for_expr;
        struct {
            struct ast_expr const* expr;
        } dump;
        struct {
            struct ast_expr const* expr; // optional
        } return_;
        struct {
            struct ast_expr const* lhs;
            struct ast_expr const* rhs;
        } assign;
        struct ast_expr const* expr;
    } data;
};
struct ast_stmt*
ast_stmt_new_decl(struct ast_decl const* decl);
struct ast_stmt*
ast_stmt_new_if(struct ast_conditional const* const* conditionals);
struct ast_stmt*
ast_stmt_new_for_range(
    struct source_location const* location,
    struct ast_identifier const* identifier,
    struct ast_expr const* begin,
    struct ast_expr const* end,
    struct ast_block const* body);
struct ast_stmt*
ast_stmt_new_for_expr(
    struct source_location const* location,
    struct ast_expr const* expr,
    struct ast_block const* body);
struct ast_stmt*
ast_stmt_new_dump(
    struct source_location const* location, struct ast_expr const* expr);
struct ast_stmt*
ast_stmt_new_return(
    struct source_location const* location, struct ast_expr const* expr);
struct ast_stmt*
ast_stmt_new_assign(
    struct source_location const* location,
    struct ast_expr const* lhs,
    struct ast_expr const* rhs);
struct ast_stmt*
ast_stmt_new_expr(struct ast_expr const* expr);

struct ast_expr {
    struct source_location const* location;
    enum ast_expr_kind {
        // Primary Expressions
        AST_EXPR_IDENTIFIER,
        AST_EXPR_QUALIFIED_IDENTIFIER,
        AST_EXPR_BOOLEAN,
        AST_EXPR_INTEGER,
        AST_EXPR_BYTES,
        AST_EXPR_LITERAL_ARRAY,
        AST_EXPR_LITERAL_SLICE,
        AST_EXPR_CAST,
        AST_EXPR_GROUPED,
        // Postfix Expressions
        AST_EXPR_SYSCALL,
        AST_EXPR_CALL,
        AST_EXPR_INDEX,
        AST_EXPR_SLICE,
        // Prefix Unary Operator Expressions
        AST_EXPR_SIZEOF,
        AST_EXPR_UNARY,
        // Infix Binary Operator Expressions
        AST_EXPR_BINARY,
    } kind;
    union ast_expr_data {
        struct ast_identifier const* identifier;
        struct {
            autil_sbuf(struct ast_identifier const* const) identifiers;
        } qualified_identifier;
        struct ast_boolean const* boolean;
        struct ast_integer const* integer;
        struct autil_string const* bytes;
        struct {
            struct ast_typespec const* typespec;
            autil_sbuf(struct ast_expr const* const) elements;
        } literal_array;
        struct {
            struct ast_typespec const* typespec;
            struct ast_expr const* pointer;
            struct ast_expr const* count;
        } literal_slice;
        struct {
            struct ast_typespec const* typespec;
            struct ast_expr const* expr;
        } cast;
        struct {
            struct ast_expr const* expr;
        } grouped;
        struct ast_stmt_syscall {
            autil_sbuf(struct ast_expr const* const) arguments;
        } syscall;
        struct {
            struct ast_expr const* func;
            autil_sbuf(struct ast_expr const* const) arguments;
        } call;
        struct {
            struct ast_expr const* lhs;
            struct ast_expr const* idx;
        } index;
        struct {
            struct ast_expr const* lhs;
            struct ast_expr const* begin;
            struct ast_expr const* end;
        } slice;
        struct {
            struct ast_typespec const* rhs;
        } sizeof_;
        struct {
            struct token const* op;
            struct ast_expr const* rhs;
        } unary;
        struct {
            struct token const* op;
            struct ast_expr const* lhs;
            struct ast_expr const* rhs;
        } binary;
    } data;
};
struct ast_expr*
ast_expr_new_identifier(struct ast_identifier const* identifier);
struct ast_expr*
ast_expr_new_qualified_identifier(
    struct ast_identifier const* const* identifiers);
struct ast_expr*
ast_expr_new_boolean(struct ast_boolean const* boolean);
struct ast_expr*
ast_expr_new_integer(struct ast_integer const* integer);
struct ast_expr*
ast_expr_new_bytes(
    struct source_location const* location, struct autil_string const* bytes);
struct ast_expr*
ast_expr_new_literal_array(
    struct source_location const* location,
    struct ast_typespec const* typespec,
    struct ast_expr const* const* elements);
struct ast_expr*
ast_expr_new_literal_slice(
    struct source_location const* location,
    struct ast_typespec const* typespec,
    struct ast_expr const* pointer,
    struct ast_expr const* count);
struct ast_expr*
ast_expr_new_cast(
    struct source_location const* location,
    struct ast_typespec const* typespec,
    struct ast_expr const* expr);
struct ast_expr*
ast_expr_new_grouped(
    struct source_location const* location, struct ast_expr const* expr);
struct ast_expr*
ast_expr_new_syscall(
    struct source_location const* location,
    struct ast_expr const* const* arguments);
struct ast_expr*
ast_expr_new_call(
    struct ast_expr const* func, struct ast_expr const* const* arguments);
struct ast_expr*
ast_expr_new_index(
    struct source_location const* location,
    struct ast_expr const* lhs,
    struct ast_expr const* idx);
struct ast_expr*
ast_expr_new_slice(
    struct source_location const* location,
    struct ast_expr const* lhs,
    struct ast_expr const* begin,
    struct ast_expr const* end);
struct ast_expr*
ast_expr_new_sizeof(
    struct source_location const* location, struct ast_typespec const* rhs);
struct ast_expr*
ast_expr_new_unary(struct token const* op, struct ast_expr const* rhs);
struct ast_expr*
ast_expr_new_binary(
    struct token const* op,
    struct ast_expr const* lhs,
    struct ast_expr const* rhs);

// Helper ast node that denotes a conditional expression (if, elif, etc.)
// consisting of a conditional expression and body.
struct ast_conditional {
    struct source_location const* location;
    struct ast_expr const* condition; // optional (NULL => else)
    struct ast_block const* body;
};
struct ast_conditional*
ast_conditional_new(
    struct source_location const* location,
    struct ast_expr const* condition,
    struct ast_block const* body);

struct ast_block {
    struct source_location const* location;
    autil_sbuf(struct ast_stmt const* const) stmts;
};
struct ast_block*
ast_block_new(
    struct source_location const* location,
    struct ast_stmt const* const* stmts);

struct ast_parameter {
    struct source_location const* location;
    struct ast_identifier const* identifier;
    struct ast_typespec const* typespec;
};
struct ast_parameter*
ast_parameter_new(
    struct ast_identifier const* identifier,
    struct ast_typespec const* typespec);

// ISO/IEC 9899:1999 Section 6.7.2 - Type Specifiers
struct ast_typespec {
    struct source_location const* location;
    enum typespec_kind {
        TYPESPEC_IDENTIFIER,
        TYPESPEC_FUNCTION,
        TYPESPEC_POINTER,
        TYPESPEC_ARRAY,
        TYPESPEC_SLICE,
        TYPESPEC_TYPEOF
    } kind;
    union {
        struct ast_identifier const* identifier;
        struct {
            autil_sbuf(struct ast_typespec const* const) parameter_typespecs;
            struct ast_typespec const* return_typespec;
        } function;
        struct {
            struct ast_typespec const* base;
        } pointer;
        struct {
            struct ast_expr const* count;
            struct ast_typespec const* base;
        } array;
        struct {
            struct ast_typespec const* base;
        } slice;
        struct {
            struct ast_expr const* expr;
        } typeof;
    } data;
};
struct ast_typespec*
ast_typespec_new_identifier(struct ast_identifier const* identifier);
struct ast_typespec*
ast_typespec_new_function(
    struct source_location const* location,
    struct ast_typespec const* const* parameter_typespecs,
    struct ast_typespec const* return_typespec);
struct ast_typespec*
ast_typespec_new_pointer(
    struct source_location const* location, struct ast_typespec const* base);
struct ast_typespec*
ast_typespec_new_array(
    struct source_location const* location,
    struct ast_expr const* count,
    struct ast_typespec const* base);
struct ast_typespec*
ast_typespec_new_slice(
    struct source_location const* location, struct ast_typespec const* base);
struct ast_typespec*
ast_typespec_new_typeof(
    struct source_location const* location, struct ast_expr const* expr);

struct ast_identifier {
    struct source_location const* location;
    char const* name; // interned
};
struct ast_identifier*
ast_identifier_new(struct source_location const* location, char const* name);

struct ast_boolean {
    struct source_location const* location;
    bool value;
};
struct ast_boolean*
ast_boolean_new(struct source_location const* location, bool value);

struct ast_integer {
    struct source_location const* location;
    struct autil_bigint const* value;
    char const* suffix; // interned
};
struct ast_integer*
ast_integer_new(
    struct source_location const* location,
    struct autil_bigint const* value,
    char const* suffix);

////////////////////////////////////////////////////////////////////////////////
//////// parse.c ///////////////////////////////////////////////////////////////

void
parse(struct module* module);

////////////////////////////////////////////////////////////////////////////////
//////// order.c ///////////////////////////////////////////////////////////////

void
order(struct module* module);

////////////////////////////////////////////////////////////////////////////////
//////// tir.c /////////////////////////////////////////////////////////////////
// Tree-based intermediate representation.

struct type {
    char const* name; // Canonical human-readable type-name (interned)
    size_t size; // sizeof

    enum type_kind {
        TYPE_VOID,
        TYPE_BOOL,
        TYPE_BYTE,
        TYPE_U8, /* integer */
        TYPE_S8, /* integer */
        TYPE_U16, /* integer */
        TYPE_S16, /* integer */
        TYPE_U32, /* integer */
        TYPE_S32, /* integer */
        TYPE_U64, /* integer */
        TYPE_S64, /* integer */
        TYPE_USIZE, /* integer */
        TYPE_SSIZE, /* integer */
        TYPE_FUNCTION,
        TYPE_POINTER,
        TYPE_ARRAY,
        TYPE_SLICE,
    } kind;
    union {
        struct {
            struct autil_bigint const* min;
            struct autil_bigint const* max;
        } integer;
        struct {
            autil_sbuf(struct type const* const) parameter_types;
            struct type const* return_type;
        } function;
        struct {
            struct type const* base;
        } pointer;
        struct {
            size_t count;
            struct type const* base;
        } array;
        struct {
            struct type const* base;
        } slice;
    } data;
};
struct type*
type_new_void(void);
struct type*
type_new_bool(void);
struct type*
type_new_byte(void);
struct type*
type_new_u8(void);
struct type*
type_new_s8(void);
struct type*
type_new_u16(void);
struct type*
type_new_s16(void);
struct type*
type_new_u32(void);
struct type*
type_new_s32(void);
struct type*
type_new_u64(void);
struct type*
type_new_s64(void);
struct type*
type_new_usize(void);
struct type*
type_new_ssize(void);
struct type*
type_new_function(
    struct type const* const* parameter_types, struct type const* return_type);
struct type*
type_new_pointer(struct type const* base);
struct type*
type_new_array(size_t count, struct type const* base);
struct type*
type_new_slice(struct type const* base);

struct type const*
type_unique_function(
    struct type const* const* parameter_types, struct type const* return_type);
struct type const*
type_unique_pointer(struct type const* base);
struct type const*
type_unique_array(size_t count, struct type const* base);
struct type const*
type_unique_slice(struct type const* base);

bool
type_is_integer(struct type const* self);
bool
type_is_uinteger(struct type const* self);
bool
type_is_sinteger(struct type const* self);
// Returns true if the type may be compared with the == or != operators.
bool
type_can_compare_equality(struct type const* self);
// Returns true if the type may be compared with the ==, !=, <, <=, >, and >=
// operators.
bool
type_can_compare_order(struct type const* self);

struct address {
    enum address_kind {
        ADDRESS_STATIC,
        ADDRESS_LOCAL,
    } kind;
    union {
        struct {
            // Full normalized name, including nested namespace information,
            // uniquely identifying the base region of the static storage
            // location in which this address resides.
            char const* name; // interned
            // Offset (in bytes) from the base region.
            size_t offset;
        } static_;
        struct {
            int rbp_offset;
        } local;
    } data;
};
struct address
address_init_static(char const* name, size_t offset);
struct address
address_init_local(int rbp_offset);
struct address*
address_new(struct address from);

struct symbol {
    enum symbol_kind {
        SYMBOL_TYPE,
        SYMBOL_VARIABLE,
        SYMBOL_CONSTANT,
        SYMBOL_FUNCTION,
        SYMBOL_NAMESPACE,
    } kind;
    struct source_location const* location;
    char const* name; // interned
    // SYMBOL_TYPE      => The type itself.
    // SYMBOL_VARIABLE  => The type of the variable.
    // SYMBOL_CONSTANT  => The type of the constant.
    // SYMBOL_FUNCTION  => The type of the function (always TYPE_FUNCTION).
    // SYMBOL_NAMESPACE => NULL.
    struct type const* type;
    // SYMBOL_TYPE      => NULL.
    // SYMBOL_VARIABLE  => ADDRESS_STATIC or ADDRESS_LOCAL.
    // SYMBOL_CONSTANT  => ADDRESS_STATIC or ADDRESS_LOCAL.
    // SYMBOL_FUNCTION  => ADDRESS_STATIC.
    // SYMBOL_NAMESPACE => NULL.
    struct address const* address;
    // SYMBOL_TYPE      => NULL.
    // SYMBOL_VARIABLE  => Compile-type-value of the variable (globals only).
    // SYMBOL_CONSTANT  => Compile-time value of the constant.
    // SYMBOL_FUNCTION  => Compile-time value of the function.
    // SYMBOL_NAMESPACE => NULL.
    struct value const* value;
    // SYMBOL_TYPE      => NULL.
    // SYMBOL_VARIABLE  => NULL.
    // SYMBOL_CONSTANT  => NULL.
    // SYMBOL_FUNCTION  => NULL.
    // SYMBOL_NAMESPACE => Symbols under the namespace.
    struct symbol_table* symbols;
};
struct symbol*
symbol_new_type(
    struct source_location const* location, struct type const* type);
struct symbol*
symbol_new_variable(
    struct source_location const* location,
    char const* name,
    struct type const* type,
    struct address const* address,
    struct value const* value);
struct symbol*
symbol_new_constant(
    struct source_location const* location,
    char const* name,
    struct type const* type,
    struct address const* address,
    struct value const* value);
struct symbol*
symbol_new_function(
    struct source_location const* location,
    char const* name,
    struct type const* type,
    struct address const* address,
    struct value const* value);
struct symbol*
symbol_new_namespace(
    struct source_location const* location,
    char const* name,
    struct symbol_table* symbols);

struct symbol_table {
    struct symbol_table const* parent; // optional (NULL => global scope)
    // Mapping from cstring to symbol. The cstring key corresponding to the
    // key-value pair (cstring, symbol) is not necessarily equal to the `name`
    // member of the symbol, such as in the case of the symbol with name
    // "foo.bar" with the mapping ("foo", symbol "foo.bar") in the namespace
    // symbol table of `foo`.
#define SYMBOL_MAP_KEY_TYPE char const*
#define SYMBOL_MAP_VAL_TYPE struct symbol const*
#define SYMBOL_MAP_CMP_FUNC autil_cstr_vpcmp
    struct autil_map* symbols;
};
struct symbol_table*
symbol_table_new(struct symbol_table const* parent);
void
symbol_table_freeze(struct symbol_table* self, struct autil_freezer* freezer);
void
symbol_table_insert(
    struct symbol_table* self, char const* name, struct symbol const* symbol);
// Lookup in this or any parent symbol table.
struct symbol const*
symbol_table_lookup(struct symbol_table const* self, char const* name);
// Lookup in this symbol table only.
struct symbol const*
symbol_table_lookup_local(struct symbol_table const* self, char const* name);

struct tir_stmt {
    struct source_location const* location;
    enum tir_stmt_kind {
        TIR_STMT_IF,
        TIR_STMT_FOR_RANGE,
        TIR_STMT_FOR_EXPR,
        TIR_STMT_DUMP,
        TIR_STMT_RETURN,
        TIR_STMT_ASSIGN,
        TIR_STMT_EXPR,
    } kind;
    union {
        struct {
            autil_sbuf(struct tir_conditional const* const) conditionals;
        } if_;
        struct {
            struct symbol const* loop_variable;
            struct tir_expr const* begin;
            struct tir_expr const* end;
            struct tir_block const* body;
        } for_range;
        struct {
            struct tir_expr const* expr;
            struct tir_block const* body;
        } for_expr;
        struct {
            struct tir_expr const* expr;
        } dump;
        struct {
            struct tir_expr const* expr; // optional
        } return_;
        struct {
            struct tir_expr const* lhs;
            struct tir_expr const* rhs;
        } assign;
        struct tir_expr const* expr;
    } data;
};
struct tir_stmt*
tir_stmt_new_if(struct tir_conditional const* const* conditionals);
struct tir_stmt*
tir_stmt_new_for_range(
    struct source_location const* location,
    struct symbol const* loop_variable,
    struct tir_expr const* begin,
    struct tir_expr const* end,
    struct tir_block const* body);
struct tir_stmt*
tir_stmt_new_for_expr(
    struct source_location const* location,
    struct tir_expr const* expr,
    struct tir_block const* body);
struct tir_stmt*
tir_stmt_new_dump(
    struct source_location const* location, struct tir_expr const* expr);
struct tir_stmt*
tir_stmt_new_return(
    struct source_location const* location, struct tir_expr const* expr);
struct tir_stmt*
tir_stmt_new_assign(
    struct source_location const* location,
    struct tir_expr const* lhs,
    struct tir_expr const* rhs);
struct tir_stmt*
tir_stmt_new_expr(
    struct source_location const* location, struct tir_expr const* expr);

// Minimum and maximum number of syscall arguments (including the syscall
// number) passed to a syscall expression. This is based on the Linux syscall
// convention which allows for a maximum of six parameters plus the syscall
// number to be passed via registers.
#define SYSCALL_ARGUMENTS_MIN ((size_t)1)
#define SYSCALL_ARGUMENTS_MAX ((size_t)7)
struct tir_expr {
    struct source_location const* location;
    struct type const* type;
    enum tir_expr_kind {
        TIR_EXPR_IDENTIFIER,
        TIR_EXPR_BOOLEAN,
        TIR_EXPR_INTEGER,
        TIR_EXPR_BYTES,
        TIR_EXPR_LITERAL_ARRAY,
        TIR_EXPR_LITERAL_SLICE,
        TIR_EXPR_CAST,
        TIR_EXPR_SYSCALL,
        TIR_EXPR_CALL,
        TIR_EXPR_INDEX,
        TIR_EXPR_SLICE,
        TIR_EXPR_SIZEOF,
        TIR_EXPR_UNARY,
        TIR_EXPR_BINARY,
    } kind;
    union {
        struct symbol const* identifier;
        bool boolean;
        struct autil_bigint const* integer;
        struct {
            struct address const* address;
            size_t count;
        } bytes;
        struct {
            autil_sbuf(struct tir_expr const* const) elements;
        } literal_array;
        struct {
            struct tir_expr const* pointer;
            struct tir_expr const* count;
        } literal_slice;
        struct {
            struct tir_expr const* expr;
        } cast;
        struct {
            autil_sbuf(struct tir_expr const* const) arguments;
        } syscall;
        struct {
            // Expression resulting in a callable function.
            struct tir_expr const* function;
            // Arguments to the callable function.
            autil_sbuf(struct tir_expr const* const) arguments;
        } call;
        struct {
            struct tir_expr const* lhs;
            struct tir_expr const* idx;
        } index;
        struct {
            struct tir_expr const* lhs;
            struct tir_expr const* begin;
            struct tir_expr const* end;
        } slice;
        struct {
            struct type const* rhs;
        } sizeof_;
        struct {
            enum uop_kind {
                UOP_NOT,
                UOP_POS,
                UOP_NEG,
                UOP_BITNOT,
                UOP_DEREFERENCE,
                UOP_ADDRESSOF,
                UOP_COUNTOF,
            } op;
            struct tir_expr const* rhs;
        } unary;
        struct {
            enum bop_kind {
                BOP_OR,
                BOP_AND,
                BOP_EQ,
                BOP_NE,
                BOP_LE,
                BOP_LT,
                BOP_GE,
                BOP_GT,
                BOP_ADD,
                BOP_SUB,
                BOP_MUL,
                BOP_DIV,
                BOP_BITOR,
                BOP_BITXOR,
                BOP_BITAND,
            } op;
            struct tir_expr const* lhs;
            struct tir_expr const* rhs;
        } binary;
    } data;
};
struct tir_expr*
tir_expr_new_identifier(
    struct source_location const* location, struct symbol const* identifier);
struct tir_expr*
tir_expr_new_boolean(struct source_location const* location, bool value);
struct tir_expr*
tir_expr_new_integer(
    struct source_location const* location,
    struct type const* type,
    struct autil_bigint const* value);
struct tir_expr*
tir_expr_new_bytes(
    struct source_location const* location,
    struct address const* address,
    size_t count);
struct tir_expr*
tir_expr_new_literal_array(
    struct source_location const* location,
    struct type const* type,
    struct tir_expr const* const* elements);
struct tir_expr*
tir_expr_new_literal_slice(
    struct source_location const* location,
    struct type const* type,
    struct tir_expr const* pointer,
    struct tir_expr const* count);
struct tir_expr*
tir_expr_new_cast(
    struct source_location const* location,
    struct type const* type,
    struct tir_expr const* expr);
struct tir_expr*
tir_expr_new_syscall(
    struct source_location const* location,
    struct tir_expr const* const* arguments);
struct tir_expr*
tir_expr_new_call(
    struct source_location const* location,
    struct tir_expr const* function,
    struct tir_expr const* const* arguments);
struct tir_expr*
tir_expr_new_index(
    struct source_location const* location,
    struct tir_expr const* lhs,
    struct tir_expr const* idx);
struct tir_expr*
tir_expr_new_slice(
    struct source_location const* location,
    struct tir_expr const* lhs,
    struct tir_expr const* begin,
    struct tir_expr const* end);
struct tir_expr*
tir_expr_new_sizeof(
    struct source_location const* location, struct type const* rhs);
struct tir_expr*
tir_expr_new_unary(
    struct source_location const* location,
    struct type const* type,
    enum uop_kind op,
    struct tir_expr const* rhs);
struct tir_expr*
tir_expr_new_binary(
    struct source_location const* location,
    struct type const* type,
    enum bop_kind op,
    struct tir_expr const* lhs,
    struct tir_expr const* rhs);
// ISO/IEC 9899:1999 Section 6.3.2.1
// https://en.cppreference.com/w/c/language/value_category
bool
tir_expr_is_lvalue(struct tir_expr const* self);

struct tir_function {
    char const* name; // interned
    struct type const* type; // TYPE_FUNCTION

    // Outermost symbol table containing symbols for function parameters, local
    // variables, and local constants in the outermost scope (i.e. body) of the
    // function. Initialized to NULL on struct creation.
    struct symbol_table const* symbol_table;
    // Initialized to NULL on struct creation.
    autil_sbuf(struct symbol const* const) symbol_parameters;
    // Initialized to NULL on struct creation.
    struct symbol const* symbol_return;
    // Initialized to NULL on struct creation.
    struct tir_block const* body;

    // Offset required to store all local variables in this function.
    // When the function is entered the stack pointer will be offset by this
    // amount before any expressions are pushed/popped to/from the stack during
    // intermediate calculations.
    int local_stack_offset;
};
// Creates a new incomplete (empty) function.
struct tir_function*
tir_function_new(char const* name, struct type const* type);
void
tir_function_del(struct tir_function* self);

struct tir_conditional {
    struct source_location const* location;
    struct tir_expr const* condition; // optional (NULL => else)
    struct tir_block const* body;
};
struct tir_conditional*
tir_conditional_new(
    struct source_location const* location,
    struct tir_expr const* condition,
    struct tir_block const* body);

struct tir_block {
    struct source_location const* location;
    struct symbol_table* symbol_table; // not owned
    autil_sbuf(struct tir_stmt const* const) stmts;
};
struct tir_block*
tir_block_new(
    struct source_location const* location,
    struct symbol_table* symbol_table,
    struct tir_stmt const* const* stmts);

struct value {
    struct type const* type;
    struct {
        bool boolean;
        uint8_t byte;
        struct autil_bigint* integer;
        struct tir_function const* function;
        struct address pointer;
        struct {
            autil_sbuf(struct value*) elements;
        } array;
        struct {
            struct value* pointer; // TYPE_POINTER
            struct value* count; // TYPE_USIZE
        } slice;
    } data;
};
struct value*
value_new_boolean(bool boolean);
struct value*
value_new_byte(uint8_t byte);
struct value*
value_new_integer(struct type const* type, struct autil_bigint* integer);
struct value*
value_new_function(struct tir_function const* function);
struct value*
value_new_pointer(struct type const* type, struct address address);
struct value*
value_new_array(struct type const* type, struct value** elements);
struct value*
value_new_slice(
    struct type const* type, struct value* pointer, struct value* count);
void
value_del(struct value* self);
void
value_freeze(struct value* self, struct autil_freezer* freezer);
struct value*
value_clone(struct value const* self);

bool
value_eq(struct value const* lhs, struct value const* rhs);
bool
value_lt(struct value const* lhs, struct value const* rhs);
bool
value_gt(struct value const* lhs, struct value const* rhs);

uint8_t* // sbuf
value_to_new_bytes(struct value const* value);

////////////////////////////////////////////////////////////////////////////////
//////// resolve.c /////////////////////////////////////////////////////////////

void
resolve(struct module* module);

////////////////////////////////////////////////////////////////////////////////
//////// eval.c ////////////////////////////////////////////////////////////////

struct evaluator*
evaluator_new(struct symbol_table const* symbol_table);
void
evaluator_del(struct evaluator* self);

struct value*
eval_rvalue(struct evaluator* evaluator, struct tir_expr const* expr);
struct value*
eval_lvalue(struct evaluator* evaluator, struct tir_expr const* expr);

////////////////////////////////////////////////////////////////////////////////
//////// codegen.c /////////////////////////////////////////////////////////////

void
codegen(void);

#endif // NOVA_H_INCLUDED
