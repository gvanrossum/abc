/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1988. */

/* Header file with grammar table structure. */

/* WARNING: this file is constructed by 'mktable'. */
/* If you want to change the grammar, see ../boot/README. */

typedef char classelem;
typedef classelem *classptr;

struct classinfo {
   classptr c_class;
   classptr c_insert;
   classptr c_append;
   classptr c_join;
};

#define MAXCHILD 4

struct table {
   string r_name;
   string r_repr[MAXCHILD+1];
   struct classinfo *r_class[MAXCHILD];
   node r_node;
};

extern struct table *table;
#define TABLEN 95
struct lexinfo {
   string l_start;
   string l_continue;
};

extern struct lexinfo *lextab;

/* Symbols indexing grammar table */

#define Rootsymbol 0
#define Name 1
#define Keyword 2
#define Number 3
#define Comment 4
#define Text1 5
#define Text2 6
#define Operator 7
#define Rawinput 8
#define Collateral 9
#define Compound 10
#define Blocked 11
#define Grouped 12
#define Sel_expr 13
#define List_or_table_display 14
#define List_filler_series 15
#define Table_filler_series 16
#define Table_filler 17
#define Text1_display 18
#define Text1_plus 19
#define Text2_display 20
#define Text2_plus 21
#define Conversion 22
#define Multiple_address 23
#define Compound_address 24
#define Selection 25
#define Behead 26
#define Curtail 27
#define Multiple_naming 28
#define Compound_naming 29
#define Else_kw 30
#define Not 31
#define Some_in 32
#define Each_in 33
#define No_in 34
#define And 35
#define Or 36
#define And_kw 37
#define Or_kw 38
#define Cmt_cmd 39
#define Short_comp 40
#define Cmt_comp 41
#define Long_comp 42
#define Put 43
#define Insert 44
#define Remove 45
#define Delete 46
#define Share 47
#define Write 48
#define Read 49
#define Read_raw 50
#define Set 51
#define Pass 52
#define For 53
#define Quit 54
#define Succeed 55
#define Fail 56
#define Check 57
#define If 58
#define While 59
#define Select 60
#define Return 61
#define Report 62
#define Kw_plus 63
#define Exp_plus 64
#define Suite 65
#define Test_suite 66
#define Head 67
#define Cmt_head 68
#define Long_unit 69
#define Short_unit 70
#define Formal_return 71
#define Formal_report 72
#define Blocked_ff 73
#define Grouped_ff 74
#define Formal_kw_plus 75
#define Formal_naming_plus 76
#define Ref_join 77
#define Refinement 78
#define Keyword_list 79
#define Unit_edit 80
#define Target_edit 81
#define Imm_cmd 82
#define Edit_unit 83
#define Colon 84
#define Edit_address 85
#define Equals 86
#define Workspace_cmd 87
#define Right 88
#define Expression 89
#define Raw_input 90
#define Suggestion 91
#define Sugghowname 92
#define Optional 93
#define Hole 94

/* LEXICAL symbols */

#define LEXICAL 95

#define NAME 95
#define KEYWORD 96
#define NUMBER 97
#define COMMENT 98
#define TEXT1 99
#define TEXT2 100
#define OPERATOR 101
#define RAWINPUT 102
#define SUGGESTION 103
#define SUGGHOWNAME 104

#define NLEX 10
