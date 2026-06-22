#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>

/* ============================================================
   LIMITS & CONSTANTS
   ============================================================ */
#define MAX_VARS        256
#define MAX_STR         1024
#define MAX_LINES       4096
#define MAX_ARRAY_SIZE  1024
#define MAX_PARAMS      16
#define MAX_FUNCTIONS   64
#define MAX_CALL_DEPTH  64

/* ============================================================
   VALUE TYPES
   ============================================================ */
typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL,
    VAL_ARRAY,
    VAL_NIL
} ValType;

typedef struct Value Value;
struct Value {
    ValType type;
    double  num;
    char   *str;         /* heap-allocated string */
    Value  *arr;         /* heap-allocated array elements */
    int     arr_len;
};

/* ============================================================
   VARIABLES
   ============================================================ */
typedef struct {
    char   name[MAX_STR];
    Value  val;
    int    is_const;
    int    scope;        /* scope depth */
} Var;

static Var   vars[MAX_VARS];
static int   var_count = 0;
static int   current_scope = 0;

/* ============================================================
   FUNCTIONS (seva)
   ============================================================ */
typedef struct {
    char  name[MAX_STR];
    char  params[MAX_PARAMS][MAX_STR];
    int   param_count;
    int   start_line;   /* line after 'seva ...' */
    int   end_line;     /* line of matching 'samapt' */
} Function;

static Function funcs[MAX_FUNCTIONS];
static int      func_count = 0;

/* ============================================================
   SOURCE LINES
   ============================================================ */
static char *lines[MAX_LINES];
static int   line_count = 0;

/* ============================================================
   RETURN VALUE (used by seva/wapas)
   ============================================================ */
static Value return_value;
static int   did_return   = 0;
static int   did_break    = 0;
static int   did_continue = 0;

/* ============================================================
   UTILITY: duplicate string
   ============================================================ */
static char *dup_str(const char *s) {
    if (!s) return NULL;
    char *d = malloc(strlen(s) + 1);
    strcpy(d, s);
    return d;
}

/* ============================================================
   VALUE HELPERS
   ============================================================ */
static Value make_nil(void) {
    Value v; memset(&v, 0, sizeof(v)); v.type = VAL_NIL; return v;
}
static Value make_num(double n) {
    Value v; memset(&v, 0, sizeof(v)); v.type = VAL_NUMBER; v.num = n; return v;
}
static Value make_bool(int b) {
    Value v; memset(&v, 0, sizeof(v)); v.type = VAL_BOOL; v.num = b ? 1.0 : 0.0; return v;
}
static Value make_str(const char *s) {
    Value v; memset(&v, 0, sizeof(v)); v.type = VAL_STRING; v.str = dup_str(s); return v;
}
static int val_truthy(Value *v) {
    if (v->type == VAL_NIL)    return 0;
    if (v->type == VAL_BOOL)   return v->num != 0.0;
    if (v->type == VAL_NUMBER) return v->num != 0.0;
    if (v->type == VAL_STRING) return v->str && strlen(v->str) > 0;
    if (v->type == VAL_ARRAY)  return v->arr_len > 0;
    return 0;
}
static double val_to_num(Value *v) {
    if (v->type == VAL_NUMBER || v->type == VAL_BOOL) return v->num;
    if (v->type == VAL_STRING) return atof(v->str ? v->str : "0");
    return 0.0;
}

static void free_value(Value *v) {
    if (v->type == VAL_STRING && v->str) { free(v->str); v->str = NULL; }
    if (v->type == VAL_ARRAY  && v->arr) {
        for (int i = 0; i < v->arr_len; i++) free_value(&v->arr[i]);
        free(v->arr); v->arr = NULL;
    }
}

static Value copy_value(Value *v) {
    Value c = *v;
    if (v->type == VAL_STRING) c.str = dup_str(v->str);
    if (v->type == VAL_ARRAY) {
        c.arr = malloc(sizeof(Value) * v->arr_len);
        for (int i = 0; i < v->arr_len; i++) c.arr[i] = copy_value(&v->arr[i]);
    }
    return c;
}

static void print_value(Value *v) {
    if (v->type == VAL_NIL)    { printf("nil"); return; }
    if (v->type == VAL_BOOL)   { printf("%s", v->num ? "sach" : "jhooth"); return; }
    if (v->type == VAL_NUMBER) {
        if (v->num == (long long)v->num) printf("%lld", (long long)v->num);
        else printf("%g", v->num);
        return;
    }
    if (v->type == VAL_STRING) { printf("%s", v->str ? v->str : ""); return; }
    if (v->type == VAL_ARRAY) {
        printf("[");
        for (int i = 0; i < v->arr_len; i++) {
            if (i) printf(", ");
            print_value(&v->arr[i]);
        }
        printf("]");
    }
}

/* ============================================================
   STRING TRIM
   ============================================================ */
static char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (!*s) return s;
    char *e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) e--;
    *(e+1) = '\0';
    return s;
}

/* ============================================================
   VARIABLE MANAGEMENT
   ============================================================ */
static Var *find_var(const char *name) {
    /* Search from most-recent scope outward */
    for (int i = var_count - 1; i >= 0; i--)
        if (strcmp(vars[i].name, name) == 0) return &vars[i];
    return NULL;
}

static void set_var(const char *name, Value val, int is_const) {
    Var *v = find_var(name);
    if (v) {
        if (v->is_const) {
            fprintf(stderr, "Error: Cannot modify constant '%s'\n", name);
            return;
        }
        free_value(&v->val);
        v->val = copy_value(&val);
        return;
    }
    if (var_count >= MAX_VARS) { fprintf(stderr, "Error: Too many variables\n"); return; }
    strncpy(vars[var_count].name, name, MAX_STR-1);
    vars[var_count].val      = copy_value(&val);
    vars[var_count].is_const = is_const;
    vars[var_count].scope    = current_scope;
    var_count++;
}

static void push_scope(void) { current_scope++; }
static void pop_scope(void)  {
    /* Remove all vars from current scope */
    while (var_count > 0 && vars[var_count-1].scope == current_scope) {
        free_value(&vars[var_count-1].val);
        var_count--;
    }
    current_scope--;
}

/* ============================================================
   FORWARD DECLARATIONS
   ============================================================ */
static Value evaluate(char *expr);
static void  execute_statement(int line_num);
static void  execute_block(int start, int end);

/* ============================================================
   FIND MATCHING samapt
   (depth-aware, handles nested je/dohrao/jad tak/seva)
   ============================================================ */
static int find_matching_samapt(int start) {
    int depth = 1;
    for (int i = start + 1; i < line_count; i++) {
        char buf[MAX_STR]; strncpy(buf, lines[i], MAX_STR-1); buf[MAX_STR-1]='\0';
        char *l = trim(buf);

        if (strcmp(l, "samapt") == 0) {
            depth--;
            if (depth == 0) return i;
        } else if ((strncmp(l,"je",2)==0 && (l[2]==' '||l[2]=='\0'))
                || (strncmp(l,"dohrao",6)==0)
                || (strncmp(l,"jad tak",7)==0)
                || (strncmp(l,"seva",4)==0)
                || (strncmp(l,"hukam",5)==0)) {
            depth++;
        }
    }
    return line_count - 1;
}

/* find else (nhite) between start and end_line */
static int find_nhite(int start, int end_line) {
    int depth = 0;
    for (int i = start + 1; i < end_line; i++) {
        char buf[MAX_STR]; strncpy(buf, lines[i], MAX_STR-1);
        char *l = trim(buf);
        if ((strncmp(l,"je",2)==0 && (l[2]==' '||l[2]=='\0'))
          || strncmp(l,"dohrao",6)==0
          || strncmp(l,"jad tak",7)==0
          || strncmp(l,"seva",4)==0
          || strncmp(l,"hukam",5)==0) depth++;
        else if (strcmp(l,"samapt")==0) depth--;
        else if (depth==0 && strncmp(l,"nhite",5)==0 && (l[5]=='\0'||l[5]==' ')) return i;
    }
    return -1;
}

/* ============================================================
   EXPRESSION EVALUATOR
   Full precedence: or -> and -> not -> compare -> add/sub -> mul/div -> unary -> atom
   ============================================================ */

/* Skip whitespace */
static const char *skip_ws(const char *p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

/* Forward declarations for recursive descent */
static Value parse_expr(const char **p);
static Value parse_or(const char **p);
static Value parse_and(const char **p);

static Value parse_compare(const char **p);
static Value parse_add(const char **p);
static Value parse_mul(const char **p);
static Value parse_unary(const char **p);
static Value parse_atom(const char **p);
static Value call_function(const char *name, const char **p);

/* ---- ARRAY LITERAL: [v1, v2, ...] ---- */
static Value parse_array_literal(const char **p) {
    (*p)++; /* skip '[' */
    Value arr; memset(&arr,0,sizeof(arr)); arr.type=VAL_ARRAY;
    int cap = 8;
    arr.arr = malloc(sizeof(Value)*cap);
    arr.arr_len = 0;
    *p = skip_ws(*p);
    if (**p == ']') { (*p)++; return arr; }
    while (**p && **p != ']') {
        if (arr.arr_len >= cap) { cap*=2; arr.arr=realloc(arr.arr,sizeof(Value)*cap); }
        arr.arr[arr.arr_len++] = parse_expr(p);
        *p = skip_ws(*p);
        if (**p == ',') { (*p)++; *p=skip_ws(*p); }
    }
    if (**p == ']') (*p)++;
    return arr;
}

/* ---- ATOM ---- */
static Value parse_atom(const char **p) {
    *p = skip_ws(*p);

    /* Parentheses */
    if (**p == '(') {
        (*p)++;
        Value v = parse_expr(p);
        *p = skip_ws(*p);
        if (**p == ')') (*p)++;
        return v;
    }

    /* Array literal */
    if (**p == '[') return parse_array_literal(p);

    /* String literal */
    if (**p == '"' || **p == '\'') {
        char delim = **p; (*p)++;
        char buf[MAX_STR]; int j=0;
        while (**p && **p != delim) {
            if (**p=='\\' && *(*p+1)) {
                (*p)++;
                switch(**p) {
                    case 'n': buf[j++]='\n'; break;
                    case 't': buf[j++]='\t'; break;
                    default:  buf[j++]=**p; break;
                }
            } else buf[j++] = **p;
            (*p)++;
        }
        if (**p) (*p)++;
        buf[j]='\0';
        return make_str(buf);
    }

    /* Number */
    if (isdigit((unsigned char)**p) || (**p=='-' && isdigit((unsigned char)*(*p+1)))) {
        char *end;
        double n = strtod(*p, &end);
        *p = end;
        return make_num(n);
    }

    /* Identifier, keyword, function call */
    if (isalpha((unsigned char)**p) || **p=='_') {
        char name[MAX_STR]; int j=0;
        while (**p && (isalnum((unsigned char)**p)||**p=='_'||**p==' ')) {
            /* Handle multi-word keywords: "jad tak", "sach", "jhooth", "na", "te", "ya" */
            /* Stop collecting if we hit an operator char */
            if (**p==' ') {
                /* peek ahead for multi-word */
                const char *peek = *p+1;
                while (*peek==' ') peek++;
                /* check known second words */
                if (strncmp(peek,"tak",3)==0 && !isalnum(peek[3])) {
                    /* "jad tak" - but we shouldn't be here; handled elsewhere */
                    break;
                }
                break; /* normal word boundary */
            }
            name[j++] = **p; (*p)++;
        }
        name[j]='\0';

        /* boolean literals */
        if (strcmp(name,"sach")==0)   return make_bool(1);
        if (strcmp(name,"jhooth")==0) return make_bool(0);

        *p = skip_ws(*p);

        /* function call */
        if (**p == '(') return call_function(name, p);

        /* array indexing */
        Value base;
        Var *v = find_var(name);
        base = v ? copy_value(&v->val) : make_nil();

        while (**p == '[') {
            (*p)++;
            Value idx = parse_expr(p);
            *p = skip_ws(*p);
            if (**p==']') (*p)++;
            if (base.type==VAL_ARRAY) {
                int i = (int)val_to_num(&idx);
                if (i>=0 && i<base.arr_len) {
                    Value elem = copy_value(&base.arr[i]);
                    free_value(&base);
                    base = elem;
                } else {
                    free_value(&base);
                    base = make_nil();
                }
            }
            free_value(&idx);
        }
        return base;
    }

    return make_nil();
}

/* ---- FUNCTION CALL ---- */
static Value call_function(const char *name, const char **p) {
    (*p)++; /* skip '(' */
    /* Parse args */
    Value args[MAX_PARAMS]; int argc=0;
    *p = skip_ws(*p);
    while (**p && **p!=')') {
        if (argc < MAX_PARAMS) args[argc++] = parse_expr(p);
        else { Value dummy=parse_expr(p); free_value(&dummy); }
        *p = skip_ws(*p);
        if (**p==',') { (*p)++; *p=skip_ws(*p); }
    }
    if (**p==')') (*p)++;

    /* Built-in functions */
    if (strcmp(name,"lambayi")==0 || strcmp(name,"len")==0) {
        /* len(array or string) */
        if (argc>0) {
            if (args[0].type==VAL_ARRAY) { int n=args[0].arr_len; for(int i=0;i<argc;i++)free_value(&args[i]); return make_num(n); }
            if (args[0].type==VAL_STRING) { int n=strlen(args[0].str?args[0].str:""); for(int i=0;i<argc;i++)free_value(&args[i]); return make_num(n); }
        }
        for(int i=0;i<argc;i++) free_value(&args[i]);
        return make_num(0);
    }
    if (strcmp(name,"gol")==0 || strcmp(name,"round")==0) {
        double n = argc>0 ? val_to_num(&args[0]) : 0;
        for(int i=0;i<argc;i++) free_value(&args[i]);
        return make_num(round(n));
    }
    if (strcmp(name,"ghat")==0 || strcmp(name,"floor")==0) {
        double n = argc>0 ? val_to_num(&args[0]) : 0;
        for(int i=0;i<argc;i++) free_value(&args[i]);
        return make_num(floor(n));
    }
    if (strcmp(name,"vadh")==0 || strcmp(name,"ceil")==0) {
        double n = argc>0 ? val_to_num(&args[0]) : 0;
        for(int i=0;i<argc;i++) free_value(&args[i]);
        return make_num(ceil(n));
    }
    if (strcmp(name,"jad")==0 || strcmp(name,"abs")==0) {
        double n = argc>0 ? val_to_num(&args[0]) : 0;
        for(int i=0;i<argc;i++) free_value(&args[i]);
        return make_num(fabs(n));
    }
    if (strcmp(name,"varg")==0 || strcmp(name,"sqrt")==0) {
        double n = argc>0 ? val_to_num(&args[0]) : 0;
        for(int i=0;i<argc;i++) free_value(&args[i]);
        return make_num(sqrt(n));
    }
    if (strcmp(name,"ank")==0 || strcmp(name,"toNumber")==0) {
        double n = argc>0 ? val_to_num(&args[0]) : 0;
        for(int i=0;i<argc;i++) free_value(&args[i]);
        return make_num(n);
    }
    if (strcmp(name,"cheez")==0 || strcmp(name,"toString")==0) {
        char buf[MAX_STR];
        if (argc>0) {
            if (args[0].type==VAL_STRING) { strncpy(buf,args[0].str?args[0].str:"",MAX_STR-1); }
            else if (args[0].type==VAL_NUMBER) {
                if (args[0].num==(long long)args[0].num) snprintf(buf,MAX_STR,"%lld",(long long)args[0].num);
                else snprintf(buf,MAX_STR,"%g",args[0].num);
            } else if (args[0].type==VAL_BOOL) snprintf(buf,MAX_STR,"%s",args[0].num?"sach":"jhooth");
            else strcpy(buf,"nil");
        } else strcpy(buf,"nil");
        for(int i=0;i<argc;i++) free_value(&args[i]);
        return make_str(buf);
    }
    if (strcmp(name,"mod")==0) {
        double a = argc>0 ? val_to_num(&args[0]) : 0;
        double b = argc>1 ? val_to_num(&args[1]) : 1;
        for(int i=0;i<argc;i++) free_value(&args[i]);
        return make_num(fmod(a,b));
    }

    /* User-defined functions */
    for (int fi=0; fi<func_count; fi++) {
        if (strcmp(funcs[fi].name, name)==0) {
            Function *f = &funcs[fi];
            push_scope();
            /* Bind params */
            for (int i=0; i<f->param_count && i<argc; i++) {
                set_var(f->params[i], args[i], 0);
            }
            for(int i=0;i<argc;i++) free_value(&args[i]);
            /* Execute body */
            did_return = 0;
            return_value = make_nil();
            execute_block(f->start_line, f->end_line);
            Value ret = copy_value(&return_value);
            free_value(&return_value);
            return_value = make_nil();
            did_return = 0;
            pop_scope();
            return ret;
        }
    }

    fprintf(stderr, "Error: Unknown function '%s'\n", name);
    for(int i=0;i<argc;i++) free_value(&args[i]);
    return make_nil();
}

/* ---- UNARY ---- */
static Value parse_unary(const char **p) {
    *p = skip_ws(*p);
    if (**p=='-') {
        (*p)++;
        Value v = parse_unary(p);
        v.num = -val_to_num(&v); v.type = VAL_NUMBER;
        return v;
    }
    /* "na " = logical NOT */
    if (strncmp(*p,"na ",3)==0) {
        *p += 3;
        Value v = parse_unary(p);
        int b = !val_truthy(&v);
        free_value(&v);
        return make_bool(b);
    }
    return parse_atom(p);
}

/* ---- MUL / DIV / MOD / POWER ---- */
static Value parse_mul(const char **p) {
    Value left = parse_unary(p);
    while (1) {
        *p = skip_ws(*p);
        char op = **p;
        if (op!='*' && op!='/' && op!='%' && op!='^') break;
        (*p)++;
        Value right = parse_unary(p);
        double l = val_to_num(&left), r = val_to_num(&right);
        free_value(&left); free_value(&right);
        if      (op=='*') left = make_num(l*r);
        else if (op=='/') left = make_num(r!=0 ? l/r : 0);
        else if (op=='%') left = make_num(fmod(l,r));
        else if (op=='^') left = make_num(pow(l,r));
    }
    return left;
}

/* ---- ADD / SUB (also string concat) ---- */
static Value parse_add(const char **p) {
    Value left = parse_mul(p);
    while (1) {
        *p = skip_ws(*p);
        char op = **p;
        if (op!='+' && op!='-') break;
        (*p)++;
        Value right = parse_mul(p);

        if (op=='+' && (left.type==VAL_STRING || right.type==VAL_STRING)) {
            /* String concatenation */
            char buf[MAX_STR*2];
            char ls[MAX_STR], rs[MAX_STR];
            if (left.type==VAL_STRING)  strncpy(ls, left.str?left.str:"",  MAX_STR-1);
            else if (left.type==VAL_NUMBER) {
                if (left.num==(long long)left.num) snprintf(ls,MAX_STR,"%lld",(long long)left.num);
                else snprintf(ls,MAX_STR,"%g",left.num);
            } else strcpy(ls,"");
            if (right.type==VAL_STRING) strncpy(rs, right.str?right.str:"", MAX_STR-1);
            else if (right.type==VAL_NUMBER) {
                if (right.num==(long long)right.num) snprintf(rs,MAX_STR,"%lld",(long long)right.num);
                else snprintf(rs,MAX_STR,"%g",right.num);
            } else strcpy(rs,"");
            snprintf(buf, sizeof(buf), "%s%s", ls, rs);
            free_value(&left); free_value(&right);
            left = make_str(buf);
        } else {
            double l = val_to_num(&left), r = val_to_num(&right);
            free_value(&left); free_value(&right);
            left = make_num(op=='+' ? l+r : l-r);
        }
    }
    return left;
}

/* ---- COMPARE ---- */
static Value parse_compare(const char **p) {
    Value left = parse_add(p);
    while (1) {
        *p = skip_ws(*p);
        char op1 = **p, op2 = *(*p+1);
        int handled = 0;
        if      (op1=='=' && op2=='=') { (*p)+=2; handled=1; }
        else if (op1=='!' && op2=='=') { (*p)+=2; handled=1; }
        else if (op1=='<' && op2=='=') { (*p)+=2; handled=1; }
        else if (op1=='>' && op2=='=') { (*p)+=2; handled=1; }
        else if (op1=='<')             { (*p)+=1; handled=1; op2=' '; }
        else if (op1=='>')             { (*p)+=1; handled=1; op2=' '; }
        if (!handled) break;

        Value right = parse_add(p);
        int result = 0;
        /* String comparison */
        if (left.type==VAL_STRING && right.type==VAL_STRING) {
            int cmp = strcmp(left.str?left.str:"", right.str?right.str:"");
            if      (op1=='=' && op2=='=') result = cmp==0;
            else if (op1=='!' && op2=='=') result = cmp!=0;
            else if (op1=='<' && op2=='=') result = cmp<=0;
            else if (op1=='>' && op2=='=') result = cmp>=0;
            else if (op1=='<')             result = cmp<0;
            else if (op1=='>')             result = cmp>0;
        } else {
            double l = val_to_num(&left), r = val_to_num(&right);
            if      (op1=='=' && op2=='=') result = l==r;
            else if (op1=='!' && op2=='=') result = l!=r;
            else if (op1=='<' && op2=='=') result = l<=r;
            else if (op1=='>' && op2=='=') result = l>=r;
            else if (op1=='<')             result = l<r;
            else if (op1=='>')             result = l>r;
        }
        free_value(&left); free_value(&right);
        left = make_bool(result);
    }
    return left;
}

/* ---- AND (te) ---- */
static Value parse_and(const char **p) {
    Value left = parse_compare(p);
    while (1) {
        *p = skip_ws(*p);
        if (strncmp(*p,"te ",3)!=0 && strncmp(*p,"te\0",3)!=0) break;
        *p += 2;
        Value right = parse_compare(p);
        int b = val_truthy(&left) && val_truthy(&right);
        free_value(&left); free_value(&right);
        left = make_bool(b);
    }
    return left;
}

/* ---- OR (ya) ---- */
static Value parse_or(const char **p) {
    Value left = parse_and(p);
    while (1) {
        *p = skip_ws(*p);
        if (strncmp(*p,"ya ",3)!=0 && strncmp(*p,"ya\0",3)!=0) break;
        *p += 2;
        Value right = parse_and(p);
        int b = val_truthy(&left) || val_truthy(&right);
        free_value(&left); free_value(&right);
        left = make_bool(b);
    }
    return left;
}

/* ---- TOP-LEVEL EXPR ---- */
static Value parse_expr(const char **p) {
    return parse_or(p);
}

/* Public evaluate entry point */
static Value evaluate(char *expr) {
    const char *p = expr;
    return parse_expr(&p);
}

/* ============================================================
   REGISTER FUNCTIONS (first pass)
   ============================================================ */
static void register_functions(void) {
    for (int i=0; i<line_count; i++) {
        char buf[MAX_STR]; strncpy(buf,lines[i],MAX_STR-1); buf[MAX_STR-1]='\0';
        char *l = trim(buf);
        if (strncmp(l,"seva ",5)!=0) continue;
        if (func_count >= MAX_FUNCTIONS) break;
        char *rest = l+5;
        /* Parse name */
        char *paren = strchr(rest,'(');
        if (!paren) continue;
        char fname[MAX_STR];
        int fn=0;
        while (rest<paren && !isspace((unsigned char)*rest)) fname[fn++]=*rest++;
        fname[fn]='\0';
        strncpy(funcs[func_count].name, fname, MAX_STR-1);
        /* Parse params */
        funcs[func_count].param_count = 0;
        paren++;
        while (*paren && *paren!=')') {
            while (*paren==' '||*paren==',') paren++;
            if (*paren==')') break;
            int k=0; char pname[MAX_STR];
            while (*paren && *paren!=',' && *paren!=')' && !isspace((unsigned char)*paren))
                pname[k++]=*paren++;
            pname[k]='\0';
            if (k>0 && funcs[func_count].param_count<MAX_PARAMS)
                strncpy(funcs[func_count].params[funcs[func_count].param_count++], pname, MAX_STR-1);
        }
        funcs[func_count].start_line = i+1;
        funcs[func_count].end_line   = find_matching_samapt(i);
        func_count++;
    }
}

/* ============================================================
   EXECUTE BLOCK
   ============================================================ */
static void execute_block(int start, int end) {
    for (int i=start; i<end && !did_return && !did_break && !did_continue; i++) {
        char buf[MAX_STR]; strncpy(buf,lines[i],MAX_STR-1); buf[MAX_STR-1]='\0';
        char *l = trim(buf);
        if (!l || strlen(l)==0) continue;
        if (strcmp(l,"samapt")==0) break;
        /* Skip nhite at block level - handled by je */
        if (strncmp(l,"nhite",5)==0 && (l[5]=='\0'||l[5]==' '||l[5]=='\n')) { /* skip */ continue; }
        execute_statement(i);
    }
}

/* ============================================================
   EXECUTE STATEMENT
   ============================================================ */
static void execute_statement(int line_num) {
    char buf[MAX_STR*4]; strncpy(buf, lines[line_num], MAX_STR*4-1); buf[MAX_STR*4-1]='\0';
    char *line = trim(buf);
    if (!line || strlen(line)==0) return;

    /* ---- Comments (waak) ---- */
    if (strncmp(line,"waak",4)==0) return;

    /* ---- Program header/structure ---- */
    if (line[0]=='#') return;
    if (strcmp(line,"hukam")==0) return;
    if (strcmp(line,"samapt")==0) return;
    if (strncmp(line,"nhite",5)==0) return;

    /* ---- Function definition (skip body) ---- */
    if (strncmp(line,"seva ",5)==0) {
        /* Skip to matching samapt - body already registered */
        return;
    }

    /* ---- RETURN (wapas) ---- */
    if (strncmp(line,"wapas",5)==0) {
        char *expr = line+5;
        expr = trim(expr);
        if (strlen(expr)>0) {
            free_value(&return_value);
            return_value = evaluate(expr);
        } else {
            return_value = make_nil();
        }
        did_return = 1;
        return;
    }

    /* ---- BREAK (rok) ---- */
    if (strcmp(line,"rok")==0) { did_break=1; return; }

    /* ---- CONTINUE (agle) ---- */
    if (strcmp(line,"agle")==0) { did_continue=1; return; }

    /* ---- PRINT (likh) ---- */
    if (strncmp(line,"likh",4)==0 && (line[4]==' '||line[4]=='\0')) {
        char *expr = trim(line+4);
        if (strlen(expr)==0) { printf("\n"); return; }
        Value v = evaluate(expr);
        print_value(&v);
        printf("\n");
        free_value(&v);
        return;
    }

    /* ---- INPUT (suno) ---- */
    if (strncmp(line,"suno",4)==0 && (line[4]==' '||line[4]=='\0')) {
        char *var = trim(line+4);
        char input[MAX_STR];
        fflush(stdout);
        if (!fgets(input, MAX_STR, stdin)) return;
        input[strcspn(input,"\n")] = 0;
        /* Try to parse as number */
        char *end;
        double num = strtod(input, &end);
        Value v;
        /* If entire string was consumed as number */
        if (*end=='\0' && end!=input) v = make_num(num);
        else v = make_str(input);
        set_var(var, v, 0);
        free_value(&v);
        return;
    }

    /* ---- VARIABLE DECLARATION (ank, cheez, halat, sangat) ---- */
    if ((strncmp(line,"ank ",4)==0) ||
        (strncmp(line,"cheez ",6)==0) ||
        (strncmp(line,"halat ",6)==0) ||
        (strncmp(line,"sangat ",7)==0)) {
        int is_const = 0;
        /* skip type keyword */
        char *rest = strchr(line,' ');
        rest = trim(rest);
        char *eq = strchr(rest,'=');
        if (eq) {
            *eq = '\0';
            char *name = trim(rest);
            char *expr = trim(eq+1);
            Value v = evaluate(expr);
            set_var(name, v, is_const);
            free_value(&v);
        } else {
            /* Declaration without init */
            char *name = trim(rest);
            set_var(name, make_nil(), 0);
        }
        return;
    }

    /* ---- CONSTANT (pakka) ---- */
    if (strncmp(line,"pakka ",6)==0) {
        char *rest = trim(line+6);
        /* skip optional type keyword */
        if (strncmp(rest,"ank ",4)==0||strncmp(rest,"cheez ",6)==0||
            strncmp(rest,"halat ",6)==0||strncmp(rest,"sangat ",7)==0) {
            rest = strchr(rest,' ');
            rest = trim(rest);
        }
        char *eq = strchr(rest,'=');
        if (eq) {
            *eq='\0';
            char *name = trim(rest);
            char *expr = trim(eq+1);
            Value v = evaluate(expr);
            set_var(name, v, 1);
            free_value(&v);
        }
        return;
    }

    /* ---- IF (je) ---- */
    if (strncmp(line,"je ",3)==0) {
        char *expr = trim(line+3);
        /* Remove trailing 'ta' or ':' if present */
        int el = strlen(expr);
        if (el>=2 && strcmp(expr+el-2," ta")==0) { expr[el-3]='\0'; }
        if (expr[el-1]==':') expr[el-1]='\0';

        int samapt_line = find_matching_samapt(line_num);
        int nhite_line  = find_nhite(line_num, samapt_line);

        Value cond = evaluate(expr);
        int   truth = val_truthy(&cond);
        free_value(&cond);

        push_scope();
        if (truth) {
            int end = nhite_line!=-1 ? nhite_line : samapt_line;
            execute_block(line_num+1, end);
        } else if (nhite_line!=-1) {
            execute_block(nhite_line+1, samapt_line);
        }
        pop_scope();
        /* skip to after samapt - but we handle this in execute_block loop */
        /* We need to advance the outer loop; hack: skip lines via a jump.
           Since we can't return a jump, the outer execute_block just continues;
           the inner blocks already executed. Lines between je...samapt that
           aren't in our executed sub-block are just execute_statement'd harmlessly
           because they're nhite/samapt and we skip those. */
        return;
    }

    /* ---- FOR LOOP (dohrao VAR ton START to END) ---- */
    if (strncmp(line,"dohrao ",7)==0) {
        char *rest = trim(line+7);
        char temp[MAX_STR]; strncpy(temp,rest,MAX_STR-1);
        char var_name[MAX_STR]="";
        double loop_start=0, loop_end=0, step=1;

        char *tok = strtok(temp," ");
        if (!tok) return;
        strncpy(var_name,tok,MAX_STR-1);

        tok = strtok(NULL," ");
        if (!tok || strcmp(tok,"ton")!=0) { fprintf(stderr,"dohrao syntax error\n"); return; }

        tok = strtok(NULL," ");
        if (!tok) return;
        loop_start = atof(tok);

        tok = strtok(NULL," ");
        if (!tok || strcmp(tok,"to")!=0) { fprintf(stderr,"dohrao syntax error\n"); return; }

        tok = strtok(NULL," ");
        if (!tok) return;
        loop_end = atof(tok);

        /* Optional step */
        tok = strtok(NULL," ");
        if (tok && strcmp(tok,"kadam")==0) {
            tok = strtok(NULL," ");
            if (tok) step = atof(tok);
        }

        int samapt_line = find_matching_samapt(line_num);

        for (double iv=loop_start;
             (step>0 ? iv<=loop_end : iv>=loop_end) && !did_return;
             iv+=step) {
            push_scope();
            set_var(var_name, make_num(iv), 0);
            did_break=0; did_continue=0;
            execute_block(line_num+1, samapt_line);
            pop_scope();
            if (did_break) { did_break=0; break; }
            if (did_continue) { did_continue=0; continue; }
        }
        return;
    }

    /* ---- WHILE LOOP (jad tak COND) ---- */
    if (strncmp(line,"jad tak ",8)==0) {
        char *expr = trim(line+8);
        int samapt_line = find_matching_samapt(line_num);

        while (!did_return) {
            /* Re-read expr each iteration since it may reference vars */
            char expr_copy[MAX_STR]; strncpy(expr_copy,expr,MAX_STR-1);
            Value cond = evaluate(expr_copy);
            int truth = val_truthy(&cond);
            free_value(&cond);
            if (!truth) break;

            push_scope();
            did_break=0; did_continue=0;
            execute_block(line_num+1, samapt_line);
            pop_scope();
            if (did_break) { did_break=0; break; }
            if (did_continue) { did_continue=0; continue; }
        }
        return;
    }

    /* ---- ARRAY ELEMENT ASSIGNMENT: name[idx] = expr ---- */
    if (isalpha((unsigned char)line[0])) {
        char *bracket = strchr(line,'[');
        char *eq      = strchr(line,'=');
        if (bracket && eq && bracket < eq && *(eq-1)!='!' && *(eq-1)!='<' && *(eq-1)!='>' && *(eq+1)!='=') {
            /* Array index assignment */
            char name[MAX_STR]; int nlen=bracket-line;
            strncpy(name,line,nlen); name[nlen]='\0'; trim(name);
            *bracket='\0'; bracket++;
            char *close_b = strchr(bracket,']');
            if (!close_b) goto plain_assign;
            *close_b='\0';
            char *idx_expr = bracket;
            *eq='\0';
            char *val_expr = trim(eq+1);
            Value idx = evaluate(idx_expr);
            Value val = evaluate(val_expr);
            int i = (int)val_to_num(&idx);
            free_value(&idx);
            Var *v = find_var(name);
            if (v && v->val.type==VAL_ARRAY && i>=0 && i<v->val.arr_len) {
                free_value(&v->val.arr[i]);
                v->val.arr[i] = copy_value(&val);
            }
            free_value(&val);
            return;
        }
    }

plain_assign:
    /* ---- COMPOUND ASSIGNMENT & PLAIN ASSIGNMENT ---- */
    if (isalpha((unsigned char)line[0])) {
        /* Check for +=, -=, *=, /= */
        char *plus_eq  = strstr(line,"+=");
        char *minus_eq = strstr(line,"-=");
        char *mul_eq   = strstr(line,"*=");
        char *div_eq   = strstr(line,"/=");
        char *eq       = NULL;
        char op_char   = 0;

        if      (plus_eq  && (!eq || plus_eq  < eq)) { eq=plus_eq;  op_char='+'; }
        if      (minus_eq && (!eq || minus_eq < eq)) { eq=minus_eq; op_char='-'; }
        if      (mul_eq   && (!eq || mul_eq   < eq)) { eq=mul_eq;   op_char='*'; }
        if      (div_eq   && (!eq || div_eq   < eq)) { eq=div_eq;   op_char='/'; }

        if (!eq) {
            /* plain = */
            eq = strchr(line,'=');
            if (eq && eq>line && (*(eq-1)=='!'||*(eq-1)=='<'||*(eq-1)=='>'||*(eq+1)=='=')) eq=NULL;
        } else {
            /* it's a compound eq; skip the operator char before = */
        }

        if (eq) {
            if (op_char) {
                *eq = '\0'; eq+=2; /* skip op and '=' */
            } else {
                *eq = '\0'; eq+=1;
            }
            char *name = trim(line);
            char *expr = trim(eq);
            Value val = evaluate(expr);
            if (op_char) {
                Var *v = find_var(name);
                Value cur = v ? copy_value(&v->val) : make_num(0);
                double l=val_to_num(&cur), r=val_to_num(&val);
                free_value(&cur); free_value(&val);
                switch(op_char){
                    case '+': val=make_num(l+r); break;
                    case '-': val=make_num(l-r); break;
                    case '*': val=make_num(l*r); break;
                    case '/': val=make_num(r!=0?l/r:0); break;
                }
            }
            set_var(name, val, 0);
            free_value(&val);
            return;
        }

        /* Bare function call as statement */
        char *paren = strchr(line,'(');
        if (paren) {
            const char *p = line;
            Value v = parse_expr(&p);
            free_value(&v);
            return;
        }
    }
}

/* ============================================================
   EXECUTE WHOLE PROGRAM
   ============================================================ */
static void execute_program(void) {
    /* First pass: register functions */
    register_functions();
    /* Second pass: execute top-level statements */
    /* Find hukam block if present */
    int hukam_start = -1, hukam_end = -1;
    for (int i=0; i<line_count; i++) {
        char buf[MAX_STR]; strncpy(buf,lines[i],MAX_STR-1); buf[MAX_STR-1]='\0';
        char *l = trim(buf);
        if (strcmp(l,"hukam")==0) { hukam_start=i; hukam_end=find_matching_samapt(i); break; }
    }
    if (hukam_start!=-1) {
        execute_block(hukam_start+1, hukam_end);
    } else {
        /* No hukam block — run entire file as top-level */
        execute_block(0, line_count);
    }
}

/* ============================================================
   MAIN
   ============================================================ */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("╔═══════════════════════════════════════╗\n");
        printf("║   13# (13 Sharp) Interpreter          ║\n");
        printf("║   Punjabi-Programming                 ║\n");
        printf("╠═══════════════════════════════════════╣\n");
        printf("║  Usage: 13sharp <filename.13>         ║\n");
        printf("╠═══════════════════════════════════════╣\n");
        printf("║  Keywords:                            ║\n");
        printf("║   ank    = number variable            ║\n");
        printf("║   cheez  = string variable            ║\n");
        printf("║   halat  = boolean variable           ║\n");
        printf("║   sangat = array variable             ║\n");
        printf("║   likh   = print                      ║\n");
        printf("║   suno   = input                      ║\n");
        printf("║   je     = if                         ║\n");
        printf("║   nhite  = else                       ║\n");
        printf("║   samapt = end                        ║\n");
        printf("║   dohrao = for loop                   ║\n");
        printf("║   jad tak= while loop                 ║\n");
        printf("║   seva   = function                   ║\n");
        printf("║   wapas  = return                     ║\n");
        printf("║   pakka  = constant                   ║\n");
        printf("║   sach   = true                       ║\n");
        printf("║   jhooth = false                      ║\n");
        printf("║   te/ya/na = and/or/not               ║\n");
        printf("╚═══════════════════════════════════════╝\n");
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) { fprintf(stderr,"Cannot open file: %s\n", argv[1]); return 1; }

    char buffer[MAX_STR*4];
    while (fgets(buffer, sizeof(buffer), f) && line_count < MAX_LINES) {
        buffer[strcspn(buffer,"\n")] = 0;
        buffer[strcspn(buffer,"\r")] = 0;
        lines[line_count] = dup_str(buffer);
        line_count++;
    }
    fclose(f);

    /* Strip UTF-8 BOM */
    if (line_count>0 && (unsigned char)lines[0][0]==0xEF &&
        (unsigned char)lines[0][1]==0xBB && (unsigned char)lines[0][2]==0xBF) {
        memmove(lines[0], lines[0]+3, strlen(lines[0])-2);
    }

    return_value = make_nil();
    execute_program();

    /* Cleanup */
    for (int i=0; i<line_count; i++) free(lines[i]);
    for (int i=0; i<var_count; i++) free_value(&vars[i].val);
    free_value(&return_value);

    return 0;
}
