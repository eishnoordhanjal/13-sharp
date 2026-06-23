/*
 * 13# (13 Sharp) Interpreter v3.0
 * Punjabi-Inspired Programming Language
 *
 * Usage:
 *   13sharp              → interactive shell (like python)
 *   13sharp file.13      → run a file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* ============================================================
   LIMITS
   ============================================================ */
#define MAX_VARS       512
#define MAX_STR        2048
#define MAX_LINES      8192
#define MAX_PARAMS     16
#define MAX_FUNCTIONS  128
#define MAX_SCOPE      64

/* ============================================================
   VALUE SYSTEM
   ============================================================ */
typedef enum { VAL_NUM, VAL_STR, VAL_BOOL, VAL_ARR, VAL_NIL } VType;

typedef struct Val Val;
struct Val {
    VType  t;
    double n;
    char  *s;
    Val   *arr;
    int    len;
};

static Val make_nil (void)           { Val v={VAL_NIL,0,NULL,NULL,0}; return v; }
static Val make_num (double n)       { Val v={VAL_NUM,n,NULL,NULL,0}; return v; }
static Val make_bool(int b)          { Val v={VAL_BOOL,b?1.0:0.0,NULL,NULL,0}; return v; }

static Val make_str(const char *s) {
    Val v={VAL_STR,0,NULL,NULL,0};
    if(s){ v.s=malloc(strlen(s)+1); strcpy(v.s,s); }
    return v;
}

static void free_val(Val *v) {
    if(!v) return;
    if((v->t==VAL_STR||v->t==VAL_ARR) && v->s){ free(v->s); v->s=NULL; }
    if(v->t==VAL_ARR && v->arr){
        for(int i=0;i<v->len;i++) free_val(&v->arr[i]);
        free(v->arr); v->arr=NULL;
    }
    v->t=VAL_NIL;
}

static Val copy_val(const Val *v) {
    Val c=*v;
    if(v->t==VAL_STR){ c.s=v->s?strdup(v->s):NULL; }
    if(v->t==VAL_ARR && v->arr){
        c.arr=malloc(sizeof(Val)*v->len);
        for(int i=0;i<v->len;i++) c.arr[i]=copy_val(&v->arr[i]);
        c.s=NULL;
    }
    return c;
}

static int val_truthy(const Val *v){
    if(v->t==VAL_NIL)  return 0;
    if(v->t==VAL_BOOL) return v->n!=0.0;
    if(v->t==VAL_NUM)  return v->n!=0.0;
    if(v->t==VAL_STR)  return v->s && v->s[0];
    if(v->t==VAL_ARR)  return v->len>0;
    return 0;
}
static double val_num(const Val *v){
    if(v->t==VAL_NUM||v->t==VAL_BOOL) return v->n;
    if(v->t==VAL_STR) return v->s?atof(v->s):0.0;
    return 0.0;
}
static void print_val(const Val *v){
    if(v->t==VAL_NIL)  { printf("khaali"); return; }
    if(v->t==VAL_BOOL) { printf("%s",v->n?"sach":"jhooth"); return; }
    if(v->t==VAL_NUM){
        if(v->n==(long long)v->n) printf("%lld",(long long)v->n);
        else printf("%g",v->n);
        return;
    }
    if(v->t==VAL_STR)  { printf("%s",v->s?v->s:""); return; }
    if(v->t==VAL_ARR){
        printf("[");
        for(int i=0;i<v->len;i++){ if(i)printf(", "); print_val(&v->arr[i]); }
        printf("]");
    }
}
static char *val_to_str(const Val *v, char *buf, int blen){
    if(v->t==VAL_NIL)  { snprintf(buf,blen,"khaali"); }
    else if(v->t==VAL_BOOL){ snprintf(buf,blen,"%s",v->n?"sach":"jhooth"); }
    else if(v->t==VAL_NUM){
        if(v->n==(long long)v->n) snprintf(buf,blen,"%lld",(long long)v->n);
        else snprintf(buf,blen,"%g",v->n);
    }
    else if(v->t==VAL_STR){ snprintf(buf,blen,"%s",v->s?v->s:""); }
    else snprintf(buf,blen,"[array]");
    return buf;
}

/* ============================================================
   VARIABLE STORE  (scoped)
   ============================================================ */
typedef struct {
    char name[MAX_STR];
    Val  val;
    int  is_const;
    int  scope;
} Var;

static Var  vars[MAX_VARS];
static int  var_count   = 0;
static int  cur_scope   = 0;

static Var *find_var(const char *n){
    for(int i=var_count-1;i>=0;i--)
        if(strcmp(vars[i].name,n)==0) return &vars[i];
    return NULL;
}
static void set_var(const char *n, Val v, int is_const){
    Var *existing=find_var(n);
    if(existing){
        if(existing->is_const){ fprintf(stderr,"Error: '%s' pakka hai, badal nahi sakde\n",n); free_val(&v); return; }
        free_val(&existing->val);
        existing->val=copy_val(&v);
        free_val(&v);
        return;
    }
    if(var_count>=MAX_VARS){ fprintf(stderr,"Error: bahut saare variables\n"); free_val(&v); return; }
    strncpy(vars[var_count].name,n,MAX_STR-1);
    vars[var_count].val      = copy_val(&v);
    vars[var_count].is_const = is_const;
    vars[var_count].scope    = cur_scope;
    var_count++;
    free_val(&v);
}
static void push_scope(){ cur_scope++; }
static void pop_scope(){
    while(var_count>0 && vars[var_count-1].scope==cur_scope){
        free_val(&vars[var_count-1].val);
        var_count--;
    }
    if(cur_scope>0) cur_scope--;
}

/* ============================================================
   SOURCE LINES
   ============================================================ */
static char *src[MAX_LINES];
static int   src_count=0;

static char *my_trim(char *s){
    while(*s&&isspace((unsigned char)*s)) s++;
    if(!*s) return s;
    char *e=s+strlen(s)-1;
    while(e>s&&isspace((unsigned char)*e)) e--;
    *(e+1)='\0';
    return s;
}
static char *trimdup(const char *s){
    char *d=strdup(s);
    char *t=my_trim(d);
    if(t!=d){ memmove(d,t,strlen(t)+1); }
    return d;
}

/* ============================================================
   FUNCTION REGISTRY
   ============================================================ */
typedef struct {
    char name[MAX_STR];
    char params[MAX_PARAMS][MAX_STR];
    int  param_count;
    int  body_start; /* line after seva ... */
    int  body_end;   /* line of matching samapt */
} Func;

static Func funcs[MAX_FUNCTIONS];
static int  func_count=0;

/* ============================================================
   CONTROL FLOW FLAGS
   ============================================================ */
static Val  ret_val;
static int  did_ret   =0;
static int  did_break =0;
static int  did_cont  =0;

/* ============================================================
   FORWARD DECLS
   ============================================================ */
static Val  eval(const char *expr);
static void exec_stmt(int ln);
static void exec_block(int start, int end);

/* ============================================================
   FIND MATCHING samapt  (depth-aware)
   ============================================================ */
static int find_samapt(int from){
    int depth=1;
    for(int i=from+1;i<src_count;i++){
        char *l=trimdup(src[i]);
        if(strcmp(l,"samapt")==0){ depth--; if(!depth){free(l);return i;} }
        else if( (strncmp(l,"je ",3)==0||strcmp(l,"je")==0)
               ||(strncmp(l,"dohrao ",7)==0)
               ||(strncmp(l,"jad tak ",8)==0||strncmp(l,"jad tak\n",8)==0)
               ||(strncmp(l,"seva ",5)==0)
               || strcmp(l,"hukam")==0 )
            depth++;
        free(l);
    }
    return src_count-1;
}

static int find_nhite(int from, int end){
    int depth=0;
    for(int i=from+1;i<end;i++){
        char *l=trimdup(src[i]);
        if((strncmp(l,"je ",3)==0||strcmp(l,"je")==0)
          ||strncmp(l,"dohrao ",7)==0
          ||(strncmp(l,"jad tak ",8)==0)
          ||strncmp(l,"seva ",5)==0
          ||strcmp(l,"hukam")==0) depth++;
        else if(strcmp(l,"samapt")==0) depth--;
        else if(depth==0&&strncmp(l,"nhite",5)==0&&(l[5]=='\0'||l[5]==' '||l[5]=='\n'))
            { free(l); return i; }
        free(l);
    }
    return -1;
}

/* ============================================================
   REGISTER FUNCTIONS  (scan src for seva)
   ============================================================ */
static void register_functions(){
    func_count=0;
    for(int i=0;i<src_count;i++){
        char *l=trimdup(src[i]);
        if(strncmp(l,"seva ",5)!=0){ free(l); continue; }
        if(func_count>=MAX_FUNCTIONS){ free(l); break; }
        char *rest=l+5;
        char *paren=strchr(rest,'(');
        if(!paren){ free(l); continue; }
        /* name */
        char fname[MAX_STR]={0}; int fn=0;
        char *p=rest;
        while(p<paren&&!isspace((unsigned char)*p)&&fn<MAX_STR-1) fname[fn++]=*p++;
        strncpy(funcs[func_count].name,fname,MAX_STR-1);
        /* params */
        funcs[func_count].param_count=0;
        p=paren+1;
        while(*p&&*p!=')'){
            while(*p==' '||*p==',') p++;
            if(*p==')') break;
            char pn[MAX_STR]={0}; int pk=0;
            while(*p&&*p!=','&&*p!=')'&&!isspace((unsigned char)*p)&&pk<MAX_STR-1) pn[pk++]=*p++;
            if(pk&&funcs[func_count].param_count<MAX_PARAMS)
                strncpy(funcs[func_count].params[funcs[func_count].param_count++],pn,MAX_STR-1);
        }
        funcs[func_count].body_start = i+1;
        funcs[func_count].body_end   = find_samapt(i);
        func_count++;
        free(l);
    }
}

/* ============================================================
   EXPRESSION PARSER  (recursive descent)
   ============================================================ */
static const char *skip_ws(const char *p){ while(*p&&isspace((unsigned char)*p))p++; return p; }

static Val parse_expr (const char **p);
static Val parse_or   (const char **p);
static Val parse_and  (const char **p);
static Val parse_cmp  (const char **p);
static Val parse_add  (const char **p);
static Val parse_mul  (const char **p);
static Val parse_unary(const char **p);
static Val parse_atom (const char **p);
static Val call_func  (const char *name, const char **p);

/* ---- ATOM ---- */
static Val parse_atom(const char **p){
    *p=skip_ws(*p);

    /* parentheses */
    if(**p=='('){
        (*p)++;
        Val v=parse_expr(p);
        *p=skip_ws(*p);
        if(**p==')') (*p)++;
        return v;
    }

    /* array literal */
    if(**p=='['){
        (*p)++;
        Val a={VAL_ARR,0,NULL,NULL,0};
        int cap=8; a.arr=malloc(sizeof(Val)*cap);
        *p=skip_ws(*p);
        if(**p==']'){(*p)++;return a;}
        while(**p&&**p!=']'){
            if(a.len>=cap){cap*=2;a.arr=realloc(a.arr,sizeof(Val)*cap);}
            a.arr[a.len++]=parse_expr(p);
            *p=skip_ws(*p);
            if(**p==','){(*p)++;*p=skip_ws(*p);}
        }
        if(**p==']') (*p)++;
        return a;
    }

    /* string literal (double or single quote) */
    if(**p=='"'||**p=='\''){
        char delim=**p; (*p)++;
        char buf[MAX_STR*2]; int j=0;
        while(**p&&**p!=delim){
            if(**p=='\\'&&*(*p+1)){
                (*p)++;
                switch(**p){
                    case 'n': buf[j++]='\n'; break;
                    case 't': buf[j++]='\t'; break;
                    case 'r': buf[j++]='\r'; break;
                    default:  buf[j++]=**p;  break;
                }
            } else {
                if(j<MAX_STR*2-1) buf[j++]=**p;
            }
            (*p)++;
        }
        if(**p) (*p)++;
        buf[j]='\0';
        return make_str(buf);
    }

    /* number */
    if(isdigit((unsigned char)**p)||(**p=='.'&&isdigit((unsigned char)*(*p+1)))){
        char *end;
        double n=strtod(*p,&end);
        *p=end;
        return make_num(n);
    }

    /* identifier / keyword */
    if(isalpha((unsigned char)**p)||**p=='_'){
        char name[MAX_STR]; int j=0;
        while(**p&&(isalnum((unsigned char)**p)||**p=='_')&&j<MAX_STR-1)
            name[j++]=*(*p)++;
        name[j]='\0';
        *p=skip_ws(*p);

        /* boolean / nil literals */
        if(strcmp(name,"sach"  )==0) return make_bool(1);
        if(strcmp(name,"jhooth")==0) return make_bool(0);
        if(strcmp(name,"khaali")==0) return make_nil();

        /* built-in functions that look like identifiers: lambayi, gol, etc. */
        if(**p=='(') return call_func(name,p);

        /* variable with optional array index */
        Var *var=find_var(name);
        Val base = var ? copy_val(&var->val) : make_nil();

        while(**p=='['){
            (*p)++;
            Val idx=parse_expr(p);
            *p=skip_ws(*p);
            if(**p==']') (*p)++;
            if(base.t==VAL_ARR){
                int i=(int)val_num(&idx);
                Val elem=(i>=0&&i<base.len)?copy_val(&base.arr[i]):make_nil();
                free_val(&base); free_val(&idx);
                base=elem;
            } else {
                free_val(&base); free_val(&idx);
                base=make_nil();
            }
        }
        return base;
    }

    return make_nil();
}

/* ---- FUNCTION CALL ---- */
static Val call_func(const char *name, const char **p){
    (*p)++; /* skip '(' */
    Val args[MAX_PARAMS]; int argc=0;
    *p=skip_ws(*p);
    while(**p&&**p!=')'){
        if(argc<MAX_PARAMS) args[argc++]=parse_expr(p);
        else { Val d=parse_expr(p); free_val(&d); }
        *p=skip_ws(*p);
        if(**p==','){(*p)++;*p=skip_ws(*p);}
    }
    if(**p==')') (*p)++;

    /* ---- BUILT-INS ---- */
    #define A0 (argc>0?&args[0]:&(Val){VAL_NIL})
    #define A1 (argc>1?&args[1]:&(Val){VAL_NIL})

    if(!strcmp(name,"lambayi")||!strcmp(name,"len")){
        int n=0;
        if(argc>0){
            if(args[0].t==VAL_ARR) n=args[0].len;
            else if(args[0].t==VAL_STR) n=args[0].s?(int)strlen(args[0].s):0;
        }
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(n);
    }
    if(!strcmp(name,"gol")||!strcmp(name,"round")){
        double n=argc>0?val_num(&args[0]):0;
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(round(n));
    }
    if(!strcmp(name,"ghat")||!strcmp(name,"floor")){
        double n=argc>0?val_num(&args[0]):0;
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(floor(n));
    }
    if(!strcmp(name,"vadh")||!strcmp(name,"ceil")){
        double n=argc>0?val_num(&args[0]):0;
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(ceil(n));
    }
    if(!strcmp(name,"mutlaq")||!strcmp(name,"abs")){
        double n=argc>0?val_num(&args[0]):0;
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(fabs(n));
    }
    if(!strcmp(name,"varg")||!strcmp(name,"sqrt")){
        double n=argc>0?val_num(&args[0]):0;
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(sqrt(n));
    }
    if(!strcmp(name,"taaqat")||!strcmp(name,"pow")){
        double a=argc>0?val_num(&args[0]):0, b=argc>1?val_num(&args[1]):1;
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(pow(a,b));
    }
    if(!strcmp(name,"bakiya")||!strcmp(name,"mod")){
        double a=argc>0?val_num(&args[0]):0, b=argc>1?val_num(&args[1]):1;
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(b!=0?fmod(a,b):0);
    }
    if(!strcmp(name,"ank")||!strcmp(name,"toNum")){
        double n=argc>0?val_num(&args[0]):0;
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(n);
    }
    if(!strcmp(name,"cheez")||!strcmp(name,"toStr")){
        char buf[MAX_STR];
        Val r=argc>0?make_str(val_to_str(&args[0],buf,MAX_STR)):make_str("");
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return r;
    }
    if(!strcmp(name,"halat")||!strcmp(name,"toBool")){
        int b=argc>0?val_truthy(&args[0]):0;
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_bool(b);
    }
    /* max / min */
    if(!strcmp(name,"vadda")||!strcmp(name,"max")){
        double best=argc>0?val_num(&args[0]):-1e300;
        for(int i=1;i<argc;i++){ double v=val_num(&args[i]); if(v>best)best=v; }
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(best);
    }
    if(!strcmp(name,"chhota")||!strcmp(name,"min")){
        double best=argc>0?val_num(&args[0]):1e300;
        for(int i=1;i<argc;i++){ double v=val_num(&args[i]); if(v<best)best=v; }
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_num(best);
    }
    /* String built-ins */
    if(!strcmp(name,"upar")||!strcmp(name,"upper")){
        char buf[MAX_STR]={0};
        if(argc>0&&args[0].t==VAL_STR&&args[0].s){
            strncpy(buf,args[0].s,MAX_STR-1);
            for(int i=0;buf[i];i++) buf[i]=toupper((unsigned char)buf[i]);
        }
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_str(buf);
    }
    if(!strcmp(name,"thhalle")||!strcmp(name,"lower")){
        char buf[MAX_STR]={0};
        if(argc>0&&args[0].t==VAL_STR&&args[0].s){
            strncpy(buf,args[0].s,MAX_STR-1);
            for(int i=0;buf[i];i++) buf[i]=tolower((unsigned char)buf[i]);
        }
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_str(buf);
    }
    /* likh as function (for shell expression results) */
    if(!strcmp(name,"likh")){
        for(int i=0;i<argc;i++){ print_val(&args[i]); if(i<argc-1)printf(" "); }
        printf("\n");
        for(int i=0;i<argc;i++) free_val(&args[i]);
        return make_nil();
    }

    /* User-defined function */
    for(int fi=0;fi<func_count;fi++){
        if(strcmp(funcs[fi].name,name)!=0) continue;
        Func *f=&funcs[fi];
        push_scope();
        for(int i=0;i<f->param_count&&i<argc;i++)
            set_var(f->params[i], args[i], 0);
        for(int i=0;i<argc;i++) free_val(&args[i]);
        did_ret=0; free_val(&ret_val); ret_val=make_nil();
        exec_block(f->body_start, f->body_end);
        Val rv=copy_val(&ret_val);
        free_val(&ret_val); ret_val=make_nil();
        did_ret=0;
        pop_scope();
        return rv;
    }

    fprintf(stderr,"Error: function '%s' nahi mili\n",name);
    for(int i=0;i<argc;i++) free_val(&args[i]);
    return make_nil();
}

/* ---- UNARY ---- */
static Val parse_unary(const char **p){
    *p=skip_ws(*p);
    if(**p=='-'){
        (*p)++;
        Val v=parse_unary(p);
        double n=-val_num(&v); free_val(&v);
        return make_num(n);
    }
    if(**p=='!'){ /* C-style not */
        (*p)++;
        Val v=parse_unary(p);
        int b=!val_truthy(&v); free_val(&v);
        return make_bool(b);
    }
    /* "na " = logical not */
    if(strncmp(*p,"na ",3)==0||strncmp(*p,"na\t",3)==0){
        *p+=2;
        Val v=parse_unary(p);
        int b=!val_truthy(&v); free_val(&v);
        return make_bool(b);
    }
    return parse_atom(p);
}

/* ---- MUL / DIV / MOD / POW ---- */
static Val parse_mul(const char **p){
    Val L=parse_unary(p);
    for(;;){
        *p=skip_ws(*p);
        char op=**p;
        if(op!='*'&&op!='/'&&op!='%'&&op!='^') break;
        (*p)++;
        Val R=parse_unary(p);
        double l=val_num(&L), r=val_num(&R);
        free_val(&L); free_val(&R);
        if     (op=='*') L=make_num(l*r);
        else if(op=='/') L=make_num(r?l/r:0);
        else if(op=='%') L=make_num(r?fmod(l,r):0);
        else             L=make_num(pow(l,r));
    }
    return L;
}

/* ---- ADD / SUB  (also string concat with +) ---- */
static Val parse_add(const char **p){
    Val L=parse_mul(p);
    for(;;){
        *p=skip_ws(*p);
        char op=**p;
        if(op!='+'&&op!='-') break;
        (*p)++;
        Val R=parse_mul(p);
        if(op=='+'&&(L.t==VAL_STR||R.t==VAL_STR)){
            char lb[MAX_STR],rb[MAX_STR];
            val_to_str(&L,lb,MAX_STR); val_to_str(&R,rb,MAX_STR);
            char *nb=malloc(strlen(lb)+strlen(rb)+1);
            strcpy(nb,lb); strcat(nb,rb);
            free_val(&L); free_val(&R);
            L=make_str(nb); free(nb);
        } else {
            double l=val_num(&L),r=val_num(&R);
            free_val(&L); free_val(&R);
            L=make_num(op=='+'?l+r:l-r);
        }
    }
    return L;
}

/* ---- COMPARE ---- */
static Val parse_cmp(const char **p){
    Val L=parse_add(p);
    for(;;){
        *p=skip_ws(*p);
        char o1=**p, o2=*(*p+1);
        int two=0;
        if((o1=='='&&o2=='=')||(o1=='!'&&o2=='=')||
           (o1=='<'&&o2=='=')||(o1=='>'&&o2=='=')) two=1;
        else if(o1=='<'||o1=='>') two=0;
        else break;

        if(two) (*p)+=2; else (*p)+=1;
        Val R=parse_add(p);
        int res=0;
        if(L.t==VAL_STR&&R.t==VAL_STR){
            int c=strcmp(L.s?L.s:"",R.s?R.s:"");
            if(o1=='='&&o2=='=') res=c==0;
            else if(o1=='!'&&o2=='=') res=c!=0;
            else if(o1=='<'&&o2=='=') res=c<=0;
            else if(o1=='>'&&o2=='=') res=c>=0;
            else if(o1=='<') res=c<0;
            else res=c>0;
        } else {
            double l=val_num(&L),r=val_num(&R);
            if(o1=='='&&o2=='=') res=l==r;
            else if(o1=='!'&&o2=='=') res=l!=r;
            else if(o1=='<'&&o2=='=') res=l<=r;
            else if(o1=='>'&&o2=='=') res=l>=r;
            else if(o1=='<') res=l<r;
            else res=l>r;
        }
        free_val(&L); free_val(&R);
        L=make_bool(res);
    }
    return L;
}

/* ---- AND  (te) ---- */
static Val parse_and(const char **p){
    Val L=parse_cmp(p);
    for(;;){
        const char *save=*p;
        *p=skip_ws(*p);
        if(strncmp(*p,"te ",3)==0||strncmp(*p,"te\t",3)==0||
           (strncmp(*p,"te",2)==0&&!isalnum((unsigned char)(*p)[2])&&(*p)[2]!='_')){
            *p+=2;
            Val R=parse_cmp(p);
            int b=val_truthy(&L)&&val_truthy(&R);
            free_val(&L); free_val(&R);
            L=make_bool(b);
        } else { *p=save; break; }
    }
    return L;
}

/* ---- OR  (ya) ---- */
static Val parse_or(const char **p){
    Val L=parse_and(p);
    for(;;){
        const char *save=*p;
        *p=skip_ws(*p);
        if(strncmp(*p,"ya ",3)==0||strncmp(*p,"ya\t",3)==0||
           (strncmp(*p,"ya",2)==0&&!isalnum((unsigned char)(*p)[2])&&(*p)[2]!='_')){
            *p+=2;
            Val R=parse_and(p);
            int b=val_truthy(&L)||val_truthy(&R);
            free_val(&L); free_val(&R);
            L=make_bool(b);
        } else { *p=save; break; }
    }
    return L;
}

static Val parse_expr(const char **p){ return parse_or(p); }

static Val eval(const char *expr){
    const char *p=expr;
    return parse_expr(&p);
}

/* ============================================================
   EXEC BLOCK / STMT
   ============================================================ */
static void exec_block(int start, int end){
    for(int i=start;i<end&&!did_ret&&!did_break&&!did_cont;i++){
        char *l=trimdup(src[i]);
        int skip=( strcmp(l,"samapt")==0
                 ||(strncmp(l,"nhite",5)==0&&(l[5]=='\0'||l[5]==' ')) );
        free(l);
        if(skip) continue;

        exec_stmt(i);

        /* jump i past body of compound statements so we don't double-execute */
        char *l2=trimdup(src[i]);
        int is_compound = (  (strncmp(l2,"je ",3)==0||strcmp(l2,"je")==0)
                           || strncmp(l2,"dohrao ",7)==0
                           || strncmp(l2,"jad tak ",8)==0
                           || strncmp(l2,"seva ",5)==0
                           || strcmp(l2,"hukam")==0 );
        if(is_compound){ int sm=find_samapt(i); i=sm; }
        free(l2);
    }
}

static void exec_stmt(int ln){
    char *raw=trimdup(src[ln]);
    char *line=raw;
    if(!line||!*line){ free(raw); return; }

    /* ---- ignore ---- */
    if(line[0]=='#'||strcmp(line,"hukam")==0||strcmp(line,"samapt")==0
       ||(strncmp(line,"nhite",5)==0&&(line[5]=='\0'||line[5]==' '))){ free(raw); return; }

    /* ---- comment  waak ---- */
    if(strncmp(line,"waak",4)==0&&(line[4]==' '||line[4]=='['||line[4]=='\0')){ free(raw); return; }

    /* ---- function definition: skip body ---- */
    if(strncmp(line,"seva ",5)==0){ free(raw); return; }

    /* ---- wapas (return) ---- */
    if(strncmp(line,"wapas",5)==0&&(line[5]==' '||line[5]=='\0')){
        char *expr=my_trim(line+5);
        free_val(&ret_val);
        ret_val = *expr ? eval(expr) : make_nil();
        did_ret=1; free(raw); return;
    }

    /* ---- rok (break) ---- */
    if(strcmp(line,"rok")==0){ did_break=1; free(raw); return; }

    /* ---- agle (continue) ---- */
    if(strcmp(line,"agle")==0){ did_cont=1; free(raw); return; }

    /* ---- likh (print) — supports multiple comma-separated args ---- */
    if(strncmp(line,"likh",4)==0&&(line[4]==' '||line[4]=='\0')){
        char *rest=my_trim(line+4);
        if(!*rest){ printf("\n"); free(raw); return; }
        /* split by commas that aren't inside strings/brackets */
        int depth=0; int in_str=0; char sq=0;
        char arg[MAX_STR]; int aj=0;
        int first=1;
        for(int i=0;;i++){
            char c=rest[i];
            if(!in_str&&(c=='"'||c=='\'')){ in_str=1; sq=c; }
            else if(in_str&&c==sq&&(i==0||rest[i-1]!='\\')) in_str=0;
            if(!in_str){
                if(c=='['||c=='(') depth++;
                else if(c==']'||c==')') depth--;
            }
            if((!in_str&&depth==0&&c==',')||c=='\0'){
                arg[aj]='\0';
                char *a=my_trim(arg);
                Val v=eval(a);
                if(!first) printf(" ");
                print_val(&v);
                free_val(&v);
                first=0;
                aj=0;
                if(c=='\0') break;
            } else {
                if(aj<MAX_STR-1) arg[aj++]=c;
            }
        }
        printf("\n");
        free(raw); return;
    }

    /* ---- suno (input) ---- */
    if(strncmp(line,"suno",4)==0&&(line[4]==' '||line[4]=='\0')){
        char *prompt_and_var=my_trim(line+4);
        /* optional: suno "prompt" varname  OR  suno varname */
        char vname[MAX_STR]={0};
        char prompt[MAX_STR]={0};
        if(*prompt_and_var=='"'||*prompt_and_var=='\''){
            char delim=*prompt_and_var; char *s=prompt_and_var+1;
            int j=0;
            while(*s&&*s!=delim){ if(j<MAX_STR-1) prompt[j++]=*s++; else s++; }
            if(*s) s++;
            s=my_trim(s);
            strncpy(vname,s,MAX_STR-1);
        } else {
            strncpy(vname,prompt_and_var,MAX_STR-1);
        }
        if(*prompt) printf("%s",prompt);
        fflush(stdout);
        char inp[MAX_STR]={0};
        if(fgets(inp,MAX_STR,stdin)){
            inp[strcspn(inp,"\n")]=0;
            inp[strcspn(inp,"\r")]=0;
        }
        char *endp; double num=strtod(inp,&endp);
        Val v=(*endp=='\0'&&endp!=inp)?make_num(num):make_str(inp);
        set_var(vname,v,0);
        free(raw); return;
    }

    /* ---- variable declaration: ank / cheez / halat / sangat ---- */
    int is_typed = ( strncmp(line,"ank ",4)==0||strncmp(line,"cheez ",6)==0
                   ||strncmp(line,"halat ",6)==0||strncmp(line,"sangat ",7)==0 );
    int is_const = strncmp(line,"pakka ",6)==0;

    if(is_typed||is_const){
        char *rest=line;
        if(is_const){
            rest=my_trim(line+6);
            /* optional type word after pakka */
            if(strncmp(rest,"ank ",4)==0||strncmp(rest,"cheez ",6)==0||
               strncmp(rest,"halat ",6)==0||strncmp(rest,"sangat ",7)==0)
                rest=my_trim(strchr(rest,' '));
        } else {
            rest=my_trim(strchr(line,' '));
        }
        char *eq=strchr(rest,'=');
        if(eq&&(eq==rest||(*(eq-1)!='!'&&*(eq-1)!='<'&&*(eq-1)!='>'))&&*(eq+1)!='='){
            *eq='\0';
            char *vname=my_trim(rest);
            char *expr =my_trim(eq+1);
            Val v=eval(expr);
            set_var(vname,v,is_const);
        } else {
            char *vname=my_trim(rest);
            if(*vname) set_var(vname,make_nil(),is_const);
        }
        free(raw); return;
    }

    /* ---- je (if) ---- */
    if(strncmp(line,"je ",3)==0){
        char *cond_str=my_trim(line+3);
        /* strip trailing ':' or ' ta' */
        int cl=strlen(cond_str);
        if(cl>0&&cond_str[cl-1]==':') cond_str[cl-1]='\0';
        cl=strlen(cond_str);
        if(cl>=3&&strcmp(cond_str+cl-3," ta")==0) cond_str[cl-3]='\0';

        int sm=find_samapt(ln);
        int nh=find_nhite(ln,sm);
        Val cv=eval(cond_str);
        int truth=val_truthy(&cv); free_val(&cv);
        push_scope();
        if(truth)  exec_block(ln+1, nh>=0?nh:sm);
        else if(nh>=0) exec_block(nh+1, sm);
        pop_scope();
        free(raw); return;
    }

    /* ---- dohrao (for) ---- */
    if(strncmp(line,"dohrao ",7)==0){
        char *rest=my_trim(line+7);
        char tmp[MAX_STR]; strncpy(tmp,rest,MAX_STR-1);
        char vname[MAX_STR]={0};
        double from=0,to=0,step=1;
        char *tok=strtok(tmp," \t");
        if(!tok){ free(raw); return; }
        strncpy(vname,tok,MAX_STR-1);
        tok=strtok(NULL," \t");
        if(!tok||strcmp(tok,"ton")!=0){ fprintf(stderr,"dohrao: 'ton' chahida\n"); free(raw); return; }
        tok=strtok(NULL," \t");
        if(!tok){ free(raw); return; }
        /* evaluate start expression */
        from=atof(tok);
        tok=strtok(NULL," \t");
        if(!tok||strcmp(tok,"to")!=0){ fprintf(stderr,"dohrao: 'to' chahida\n"); free(raw); return; }
        tok=strtok(NULL," \t");
        if(!tok){ free(raw); return; }
        to=atof(tok);
        /* optional step */
        tok=strtok(NULL," \t");
        if(tok&&strcmp(tok,"kadam")==0){
            tok=strtok(NULL," \t");
            if(tok) step=atof(tok);
        }
        int sm=find_samapt(ln);
        if(step==0) step=1;
        int going_up=(step>0);
        for(double iv=from;going_up?(iv<=to):(iv>=to);iv+=step){
            if(did_ret) break;
            push_scope();
            set_var(vname,make_num(iv),0);
            did_break=0; did_cont=0;
            exec_block(ln+1,sm);
            pop_scope();
            if(did_break){ did_break=0; break; }
            if(did_cont)  { did_cont=0; continue; }
        }
        free(raw); return;
    }

    /* ---- jad tak (while) ---- */
    if(strncmp(line,"jad tak ",8)==0){
        char *cond_str=my_trim(line+8);
        int sm=find_samapt(ln);
        for(;;){
            if(did_ret) break;
            Val cv=eval(cond_str);
            int truth=val_truthy(&cv); free_val(&cv);
            if(!truth) break;
            push_scope();
            did_break=0; did_cont=0;
            exec_block(ln+1,sm);
            pop_scope();
            if(did_break){ did_break=0; break; }
            if(did_cont)  { did_cont=0; continue; }
        }
        free(raw); return;
    }

    /* ---- array index assignment: name[idx] = expr ---- */
    if(isalpha((unsigned char)line[0])||line[0]=='_'){
        char *br=strchr(line,'[');
        char *eq=strstr(line,"=");
        /* make sure eq isn't == != <= >= */
        while(eq&&(*(eq+1)=='='||(eq>line&&(*(eq-1)=='!'||*(eq-1)=='<'||*(eq-1)=='>'))))
            eq=strstr(eq+1,"=");

        if(br&&eq&&br<eq){
            char vname[MAX_STR]={0};
            int nl=br-line; strncpy(vname,line,nl); vname[nl]='\0'; my_trim(vname);
            char *idx_s=br+1;
            char *cb=strchr(idx_s,']');
            if(cb){
                *cb='\0'; *eq='\0';
                char *val_s=my_trim(eq+1);
                Val idx=eval(idx_s);
                Val val=eval(val_s);
                int i=(int)val_num(&idx); free_val(&idx);
                Var *v=find_var(vname);
                if(v&&v->val.t==VAL_ARR&&i>=0&&i<v->val.len){
                    free_val(&v->val.arr[i]);
                    v->val.arr[i]=copy_val(&val);
                }
                free_val(&val);
                free(raw); return;
            }
        }

        /* ---- compound assignment: +=  -=  *=  /= ---- */
        char *peq=NULL; char pop=0;
        char *pe;
        if((pe=strstr(line,"+="))){ peq=pe; pop='+'; }
        else if((pe=strstr(line,"-="))){ peq=pe; pop='-'; }
        else if((pe=strstr(line,"*="))){ peq=pe; pop='*'; }
        else if((pe=strstr(line,"/="))){ peq=pe; pop='/'; }

        if(peq){
            *peq='\0';
            char *vname=my_trim(line);
            char *vexpr=my_trim(peq+2);
            Val cur; Var *v=find_var(vname);
            cur = v ? copy_val(&v->val) : make_num(0);
            Val rhs=eval(vexpr);
            Val res;
            if(pop=='+') res=make_num(val_num(&cur)+val_num(&rhs));
            else if(pop=='-') res=make_num(val_num(&cur)-val_num(&rhs));
            else if(pop=='*') res=make_num(val_num(&cur)*val_num(&rhs));
            else res=make_num(val_num(&rhs)?val_num(&cur)/val_num(&rhs):0);
            free_val(&cur); free_val(&rhs);
            set_var(vname,res,0);
            free(raw); return;
        }

        /* ---- plain assignment: name = expr ---- */
        /* find = that isn't == != <= >= */
        char *aeq=line;
        for(;;){
            aeq=strchr(aeq,'=');
            if(!aeq) break;
            if(aeq>line&&(*(aeq-1)=='!'||*(aeq-1)=='<'||*(aeq-1)=='>')){aeq++;continue;}
            if(*(aeq+1)=='='){aeq++;continue;}
            break;
        }
        if(aeq){
            *aeq='\0';
            char *vname=my_trim(line);
            char *vexpr=my_trim(aeq+1);
            Val val=eval(vexpr);
            set_var(vname,val,0);
            free(raw); return;
        }

        /* bare function call as statement */
        char *lpar=strchr(line,'(');
        if(lpar){
            const char *pp=line;
            Val v=parse_expr(&pp);
            free_val(&v);
            free(raw); return;
        }
    }

    free(raw);
}

/* ============================================================
   EXECUTE PROGRAM
   ============================================================ */

static void run_program(){
    register_functions();
    /* look for hukam block */
    int hs=-1, he=-1;
    for(int i=0;i<src_count;i++){
        char *l=trimdup(src[i]);
        if(strcmp(l,"hukam")==0){ hs=i; he=find_samapt(i); free(l); break; }
        free(l);
    }
    free_val(&ret_val); ret_val=make_nil();
    did_ret=did_break=did_cont=0;
    if(hs>=0) exec_block(hs+1,he);
    else      exec_block(0,src_count);
}

static void add_line(const char *s){
    if(src_count<MAX_LINES){
        src[src_count++]=strdup(s);
    }
}

/* ============================================================
   INTERACTIVE SHELL  (like Python REPL)
   ============================================================ */
static void shell(){
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   13# (13 Sharp) - Punjabi Language v3.0   ║\n");
    printf("║   Interactive Shell  |  'madad' for help    ║\n");
    printf("║   'band' ya Ctrl+C to exit                  ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    /* persistent session: vars survive across entries */
    char line[MAX_STR*4];
    static char block_buf[512][MAX_STR]; /* buffer for multi-line blocks */
    int  block_lines=0;
    int  in_block=0;   /* depth of open block (je/dohrao/jad tak/seva/hukam) */

    for(;;){
        /* prompt: >>> at top level, ... inside a block */
        if(in_block>0) printf("... ");
        else           printf(">>> ");
        fflush(stdout);

        if(!fgets(line,sizeof(line),stdin)){
            printf("\nBand ho raha hai...\n");
            break;
        }
        line[strcspn(line,"\n")]=0;
        line[strcspn(line,"\r")]=0;

        char *l=my_trim(line);

        /* exit commands */
        if(strcmp(l,"band")==0||strcmp(l,"exit")==0||strcmp(l,"quit")==0){
            printf("Sat Sri Akal! 🙏\n"); break;
        }

        /* help */
        if(strcmp(l,"madad")==0||strcmp(l,"help")==0){
            printf("\n  Variables:   ank x = 5   |   cheez s = \"hello\"   |   halat b = sach   |   sangat a = [1,2,3]\n");
            printf("  Constants:   pakka ank PI = 3.14\n");
            printf("  Print:       likh x, y, \"text\"          (multiple values OK)\n");
            printf("  Input:       suno \"Enter name: \" naam\n");
            printf("  If:          je x > 5 ... nhite ... samapt\n");
            printf("  For:         dohrao i ton 1 to 10 ... samapt\n");
            printf("  While:       jad tak x > 0 ... samapt\n");
            printf("  Function:    seva jodhna(a, b)  wapas a+b  samapt\n");
            printf("  Built-ins:   lambayi() gol() ghat() vadh() mutlaq() varg() taaqat()\n");
            printf("               vadda() chhota() upar() thhalle() cheez() ank()\n");
            printf("  Logic:       te (and)  ya (or)  na (not)\n");
            printf("  Boolean:     sach  jhooth\n");
            printf("  Loop ctrl:   rok (break)   agle (continue)\n");
            printf("  Commands:    band / exit   madad / help   saaf (clear vars)\n\n");
            continue;
        }

        /* clear variables */
        if(strcmp(l,"saaf")==0||strcmp(l,"clear")==0){
            for(int i=0;i<var_count;i++) free_val(&vars[i].val);
            var_count=0; func_count=0;
            printf("(saare variables saaf ho gaye)\n");
            continue;
        }

        if(!*l) continue;

        /* accumulate into block buffer */
        strncpy(block_buf[block_lines],line,MAX_STR-1);
        block_lines++;

        /* track block depth */
        if(  strncmp(l,"je ",3)==0||strcmp(l,"je")==0
           ||strncmp(l,"dohrao ",7)==0
           ||strncmp(l,"jad tak ",8)==0
           ||strncmp(l,"seva ",5)==0
           ||strcmp(l,"hukam")==0 ) in_block++;
        if(strcmp(l,"samapt")==0&&in_block>0) in_block--;

        /* execute when we have a complete statement */
        if(in_block==0){
            /* Check if this is a function definition */
            char *first=trimdup(block_buf[0]);
            int is_func_def=(strncmp(first,"seva ",5)==0);
            free(first);

            if(is_func_def){
                /* Append to src (don't clear) so the function persists */
                for(int i=0;i<block_lines;i++) add_line(block_buf[i]);
                block_lines=0;
                /* re-register all known functions including new one */
                register_functions();
                printf("(seva nondhi ho gayi)\n");
            } else {
                /* Normal statement: run in fresh src window but keep func lines */
                /* Save current src_count (function definitions live at the top) */
                int func_end=src_count;
                for(int i=0;i<block_lines;i++) add_line(block_buf[i]);
                block_lines=0;

                free_val(&ret_val); ret_val=make_nil();
                did_ret=did_break=did_cont=0;
                exec_block(func_end, src_count);

                /* Remove the just-executed lines, keep function defs */
                for(int i=func_end;i<src_count;i++) free(src[i]);
                src_count=func_end;
            }
        }
    }
}

/* ============================================================
   MAIN
   ============================================================ */
int main(int argc, char *argv[]){
    ret_val=make_nil();

    if(argc>=2){
        /* file mode */
        FILE *f=fopen(argv[1],"r");
        if(!f){ fprintf(stderr,"File nahi khulli: %s\n",argv[1]); return 1; }
        char buf[MAX_STR*4];
        while(fgets(buf,sizeof(buf),f)&&src_count<MAX_LINES){
            buf[strcspn(buf,"\n")]=0;
            buf[strcspn(buf,"\r")]=0;
            src[src_count++]=strdup(buf);
        }
        fclose(f);
        /* strip BOM */
        if(src_count>0&&(unsigned char)src[0][0]==0xEF&&
           (unsigned char)src[0][1]==0xBB&&(unsigned char)src[0][2]==0xBF)
            memmove(src[0],src[0]+3,strlen(src[0])-2);
        run_program();
    } else {
        /* interactive shell */
        shell();
    }

    for(int i=0;i<src_count;i++) free(src[i]);
    for(int i=0;i<var_count;i++) free_val(&vars[i].val);
    free_val(&ret_val);
    return 0;
}
