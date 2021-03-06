%{
// Gmsh - Copyright (C) 1997-2015 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@geuz.org>.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "GmshConfig.h"
#include "GmshMessage.h"
#include "Geo.h"
#include "Gmsh.tab.hpp"
#include "GmshIO.h"

void   parsestring(char endchar);
char  *strsave(char *ptr);
void   skipcomments(void);
void   skipline(void);

#if defined(HAVE_COMPRESSED_IO) && defined(HAVE_ZLIB)
#define YY_INPUT(buf,result,max_size)                                   \
     {                                                                  \
       int c = '*', n;                                                  \
       for ( n = 0; n < (int) max_size &&                               \
               (c = gzgetc( yyin )) != EOF && c != '\n'; ++n )          \
         buf[n] = (char) c;                                             \
       if ( c == '\n' ){                                                \
         buf[n++] = (char) c;                                           \
         yylineno++;                                                    \
       }                                                                \
       if ( c == EOF )  {                                               \
         int ernum;                                                     \
         const char *msg=gzerror(yyin,&ernum);                          \
         if (ernum)                                                     \
            Msg::Fatal("Input in flex scanner failed");                 \
       }                                                                \
       result = n;                                                      \
     }
#else
#define YY_INPUT(buf,result,max_size)					\
     {									\
       int c = '*', n;							\
       for ( n = 0; n < (int) max_size &&                               \
	       (c = fgetc( yyin )) != EOF && c != '\n'; ++n )		\
	 buf[n] = (char) c;						\
       if ( c == '\n' ){						\
	 buf[n++] = (char) c;						\
	 yylineno++;							\
       }								\
       if ( c == EOF && ferror( yyin ) )				\
	 Msg::Fatal("Input in flex scanner failed");			\
       result = n;							\
     }
#endif

#if defined(WIN32)
#define isatty(arg) -1
#define YY_NO_UNISTD_H
#endif

%}

alpha	[a-zA-Z\_]
dieze	[\#]
special	[\.]
digit	[0-9]
exp	[Ee][-+]?{digit}+
string	{alpha}({alpha}|{digit})*

%%

[\ \t\n\r\f]		/* none */;
";"                     return tEND;
"/*"			skipcomments();
"//"			skipline();
"\""			{ parsestring('\"'); return tBIGSTR; }
"\'"			{ parsestring('\''); return tBIGSTR; }
"newreg"		{ gmsh_yylval.d = NEWREG(); return tDOUBLE; }
"newp"  		{ gmsh_yylval.d = NEWPOINT(); return tDOUBLE; }
"newl"  		{ gmsh_yylval.d = NEWLINE(); return tDOUBLE; }
"newc"  		{ gmsh_yylval.d = NEWLINE(); return tDOUBLE; }
"newll"  		{ gmsh_yylval.d = NEWLINELOOP(); return tDOUBLE; }
"news"  		{ gmsh_yylval.d = NEWSURFACE(); return tDOUBLE; }
"newsl"  		{ gmsh_yylval.d = NEWSURFACELOOP(); return tDOUBLE; }
"newv"  		{ gmsh_yylval.d = NEWVOLUME(); return tDOUBLE; }
"newf"  		{ gmsh_yylval.d = NEWFIELD(); return tDOUBLE; }
"="                     return tAFFECT;
"+="                    return tAFFECTPLUS;
"-="                    return tAFFECTMINUS;
"*="                    return tAFFECTTIMES;
"/="                    return tAFFECTDIVIDE;
":"                     return tDOTS;
"..."                   return tDOTS;
"||"                    return tOR;
"&&"                    return tAND;
"++"                    return tPLUSPLUS;
"--"                    return tMINUSMINUS;
"=="                    return tEQUAL;
"!="                    return tNOTEQUAL;
"<="                    return tLESSOREQUAL;
">="                    return tGREATEROREQUAL;

Abort                   return tAbort;
Acos                    return tAcos;
AdaptMesh               return tAdaptMesh;
Affine                  return tAffine;
ArcCos                  return tAcos;
Alias                   return tAlias;
AliasWithOptions        return tAliasWithOptions;
Asin                    return tAsin;
ArcSin                  return tAsin;
Atan                    return tAtan;
ArcTan                  return tAtan;
Atan2                   return tAtan2;
ArcTan2                 return tAtan2;

Betti                   return tBetti;
Bezier			return tBezier;
BSpline			return tBSpline;
BoundingBox             return tBoundingBox;

Ceil                    return tCeil;
Circle                  return tCircle;
CopyOptions             return tCopyOptions;
Coherence               return tCoherence;
Cohomology              return tCohomology;
Combine                 return tCombine;
Cosh                    return tCosh;
Cos                     return tCos;
Characteristic          return tCharacteristic;
Complex                 return tComplex;
Color                   return tColor;
ColorTable              return tColorTable;
Compound                return tCompound;
Coordinates             return tCoordinates;
CatmullRom		return tSpline;
Call                    return tCall;
CreateTopology          return tCreateTopology;
CreateTopologyNoHoles   return tCreateTopologyNoHoles;
Cpu                     return tCpu;
CurrentDirectory        return tCurrentDirectory;
CurrentDir              return tCurrentDirectory;

DefineConstant          return tDefineConstant;
DefineNumber            return tDefineNumber;
DefineString            return tDefineString;
Delete                  return tDelete;
Degenerated             return tDegenerated;
Dilate                  return tDilate;
Draw                    return tDraw;
SetChanged              return tSetChanged;

Exp                     return tExp;
Ellipsis                return tEllipse;
Ellipse                 return tEllipse;
Else                    return tElse;
ElseIf                  return tElseIf;
Extrude                 return tExtrude;
Elliptic		return tElliptic;
EndFor                  return tEndFor;
EndIf                   return tEndIf;
Euclidian               return tEuclidian;
Error                   return tError;
Exit                    return tExit;
Exists                  return tExists;

Fabs                    return tFabs;
Field                   return tField;
Find                    return tFind;
FileExists              return tFileExists;
FixRelativePath         return tFixRelativePath;
Floor                   return tFloor;
Fmod                    return tFmod;
For                     return tFor;
Function                return tMacro;

GetEnv                  return tGetEnv;
GetString               return tGetString;
GetNumber               return tGetNumber;
GetValue                return tGetValue;
GMSH_MAJOR_VERSION      return tGMSH_MAJOR_VERSION;
GMSH_MINOR_VERSION      return tGMSH_MINOR_VERSION;
GMSH_PATCH_VERSION      return tGMSH_PATCH_VERSION;
GmshExecutableName      return tGmshExecutableName;

Hide                    return tHide;
Hole                    return tHole;
Homology                return tHomology;
Hypot                   return tHypot;

In                      return tIn;
If                      return tIf;
Intersect               return tIntersect;
INTERPOLATION_SCHEME    return tInterpolationScheme;

Knots			return tNurbsKnots;

Length                  return tLength;
Line                    return tLine;
List                    return tList;
Log                     return tLog;
Log10                   return tLog10;
Layers                  return tLayers;
Levelset                return tLevelset;

Macro                   return tMacro;
MeshAlgorithm           return tMeshAlgorithm;
Modulo                  return tModulo;
MPI_Rank                return tMPI_Rank;
MPI_Size                return tMPI_Size;
Memory                  return tMemory;

NameFromString          return tNameFromString;
NewModel                return tNewModel;
Nurbs			return tNurbs;

OnelabAction            return tOnelabAction;
OnelabRun               return tOnelabRun;
Order			return tNurbsOrder;

Periodic                return tPeriodic;
Physical                return tPhysical;
Pi                      return tPi;
Plane                   return tPlane;
Point                   return tPoint;
Parametric		return tParametric;
PolarSphere             return tPolarSphere;
Printf                  return tPrintf;
Plugin                  return tPlugin;

Quad[tT]ri[aA]dd[vV]erts        return tQuadTriAddVerts;
Quad[tT]ri[nN]o[nN]ew[vV]erts   return tQuadTriNoNewVerts;

Quad[tT]ri[dD]bl        return tQuadTriDbl;
Quad[tT]ri[sS]ngl       return tQuadTriSngl;

Recombine               return tRecombine;
Recomb[lL]aterals       return tRecombLaterals;
Recursive               return tRecursive;
Rotate                  return tRotate;
Round                   return tRound;
Ruled                   return tRuled;
Rand                    return tRand;
RefineMesh              return tRefineMesh;
RelocateMesh            return tRelocateMesh;
Return                  return tReturn;
Reverse                 return tReverse;

ScaleLastLayer          return tScaleLast;
Smoother                return tSmoother;
SetOrder                return tSetOrder;
SetNumber               return tSetNumber;
SetString               return tSetString;
SetPartition            return tSetPartition;
Sqrt                    return tSqrt;
Sin                     return tSin;
Sinh                    return tSinh;
Sphere                  return tSphere;
Spline                  return tSpline;
Split                   return tSplit;
Surface                 return tSurface;
Str                     return tStr;
StringFromName          return tStringFromName;
Sprintf                 return tSprintf;
StrCat                  return tStrCat;
StrReplace              return tStrReplace;
StrPrefix               return tStrPrefix;
StrRelative             return tStrRelative;
StrFind                 return tStrFind;
StrCmp                  return tStrCmp;
StrChoice               return tStrChoice;
Show                    return tShow;
Symmetry                return tSymmetry;
SyncModel               return tSyncModel;

T2                      return tText2D;
T3                      return tText3D;
TestLevel               return tTestLevel;
TextAttributes          return tTextAttributes;
TIME                    return tTime;
Transfinite             return tTransfinite;
Transf[qQ]uad[tT]ri     return tTransfQuadTri;
Translate               return tTranslate;
Tanh                    return tTanh;
Tan                     return tTan;
Today                   return tToday;
TotalMemory             return tTotalMemory;

UndefineConstant        return tUndefineConstant;
UpperCase               return tUpperCase;
LowerCase               return tLowerCase;
LowerCaseIn             return tLowerCaseIn;
Using                   return tUsing;

Volume                  return tVolume;

{digit}+ |
{digit}+"."{digit}*({exp})? |
{digit}*"."{digit}+({exp})? |
{digit}+{exp}           { gmsh_yylval.d = atof((char *)yytext); return tDOUBLE; }

{string}		{ gmsh_yylval.c = strsave((char*)yytext); return tSTRING; }

.                       return yytext[0];

%%

#undef gmsh_yywrap

int gmsh_yywrap() {return 1;}

void skipcomments(void)
{
  int c;

  while (1) {
    while ((c = yyinput()) != '*'){
      if(gmsheof(yyin)){
	Msg::Error("End of file in commented region");
        return;
      }
    }
    if ((c = yyinput()) == '/')
      return;
    unput(c);
  }
}

void parsestring(char endchar)
{
  int c;
  char tmp[1024];

  // Note that we keep special characters (end-of-line \n, tabs \t,
  // etc.) "as is" in the output string: see yyinput() above
  int i = 0;
  while ((c = yyinput()) != endchar) {
    if(gmsheof(yyin)){
      Msg::Error("End of file in string");
      break;
    }
    else if(i >= (int)sizeof(tmp)-1){
      Msg::Error("String too long");
      break;
    }
    else{
      tmp[i++] = (char)c;
    }
  }
  tmp[i] = '\0';
  gmsh_yylval.c = strsave(tmp);
}

char *strsave(char *ptr)
{
  return((char*)strcpy((char*)malloc(strlen(ptr)+1),ptr));
}

void skipline()
{
  int c;
  while ((c = yyinput()) != '\n'){
    if(gmsheof(yyin)) return;
  }
}

static bool is_alpha(const int c)
{
  return (c>='a' && c<='z') || (c>='A' && c<='Z') || c=='_';
}

void skip_until(const char *skip, const char *until)
{
  int i, nb_skip = 0;
  int l_skip, l_until, l_max, l;
  char chars[256];
  int c_next, c_next_skip, c_next_until, c_previous = 0;

  l_skip = (skip)? strlen(skip) : 0;
  l_until = strlen(until);

  l_max = std::max(l_skip,l_until);
  if(l_max >= (int)sizeof(chars)){
    Msg::Error("Search pattern too long in skip_until");
    return;
  }

  while(1){
    while (1){
      chars[0] = yyinput();
      if(gmsheof(yyin)){
	Msg::Error("Unexpected end of file");
	return;
      }
      if(chars[0] == '/'){
        c_next = yyinput();
        if     (c_next ==  '*') skipcomments();
        else if(c_next ==  '/') skipline();
        else unput(c_next);
      }
      if(!c_previous || !is_alpha(c_previous)){
        if(chars[0] == until[0]) break;
        if(skip && chars[0] == skip[0]) break;
      }
      c_previous = chars[0];
    }

    l = l_max;

    for(i=1; i<l; i++){
      chars[i] = yyinput();
      if(gmsheof(yyin)){
	l = i;
	break;
      }
    }

    c_next = yyinput(); unput(c_next);
    c_next_skip = (l_skip<l)? chars[l_skip] : c_next;
    c_next_until = (l_until<l)? chars[l_until] : c_next;

    if(!strncmp(chars,until,l_until) && !is_alpha(c_next_until)){
      if(!nb_skip){
	return;
      }
      else{
	nb_skip--;
      }
    }
    else if(skip && !strncmp(chars,skip,l_skip) && !is_alpha(c_next_skip)){
      nb_skip++;
      // Attention: unput(.) should be applied a number of times equal to
      // l-l_skip (always >0 for skip="For" and until="EndFor", or skip="If" and
      // until="EndIf"); in particular, because "If" is followed by a minimum of
      // 3 chars (e.g., '(1)'), with a total lenght thus exactly equal to the
      // one of "EndIf", one avoid an error when looking then for
      // "EndIf". (Patrick)
    }
    else{
      for(i=1;i<l-1;i++){
	unput(chars[l-i]);
      }
    }

  }
}

void skip_until_test(const char *skip, const char *until,
                     const char *until2, int l_until2_sub, int *type_until2)
{
  int i, nb_skip = 0;
  int l_skip, l_until, l_until2, l_max, l;
  char chars[256];
  int c_next, c_next_skip, c_next_until, c_next_until2, c_previous = 0, flag_EOL_EOF = 0;

  l_skip = (skip)? strlen(skip) : 0;
  l_until = strlen(until);
  l_until2 = (until2)? strlen(until2) : 0;

  l_max = std::max(l_skip,l_until);
  l_max = std::max(l_max,l_until2);
  if(l_max >= (int)sizeof(chars)){
    Msg::Error("Search pattern too long in skip_until_test");
    return;
  }

  while(1){
    while (1){
      chars[0] = yyinput();
      if(gmsheof(yyin)){
	Msg::Error("Unexpected end of file");
	return;
      }
      if(chars[0] == '/'){
        c_next = yyinput();
        if     (c_next ==  '*') skipcomments();
        else if(c_next ==  '/') skipline();
        else unput(c_next);
      }
      if(!c_previous || !is_alpha(c_previous)){
        if(chars[0] == until[0]) break;
        if(skip && chars[0] == skip[0]) break;
        if(!nb_skip && until2 && chars[0] == until2[0]) break;
        // Useless to search for until2 if nb_skip!=0
      }
      c_previous = chars[0];
    }

    l = l_max;
    flag_EOL_EOF = 0;

    for(i=1; i<l; i++){
      chars[i] = yyinput();
      if(chars[i] == '\n'){
        unput(chars[i]); chars[i] = 0; l = i; flag_EOL_EOF = 1;
        break;
      }
      if(gmsheof(yyin)){
	l = i; flag_EOL_EOF = 1;
	break;
      }
    }

    if(!flag_EOL_EOF){
      c_next = yyinput(); unput(c_next);
      c_next_skip = (l_skip<l)? chars[l_skip] : c_next;
      c_next_until = (l_until<l)? chars[l_until] : c_next;
      if (!nb_skip)
        c_next_until2 = (l_until2<l)? chars[l_until2] : c_next;
    }
    else{
      c_next = 0; c_next_skip = 0; c_next_until = 0; c_next_until2 = 0;
    }

    if(!nb_skip && !strncmp(chars,until2,l_until2) && !is_alpha(c_next_until2)){
      *type_until2 = 1; // Found word is full until2 (e.g., "ElseIf")
      for(int i = 1; i <= l; i++){ // Only correct if l == l_until2
        unput(chars[l-i]);
      } // New file position points "ElseIf", that will be then analysed by the parser
      return;
    }
    else if(!nb_skip && !strncmp(chars,until2,l_until2_sub) && !is_alpha(chars[l_until2_sub])){
      *type_until2 = 2; // Found word is subword from until2 (e.g., "Else")
      for(int i = 1; i <= l-l_until2_sub; i++){ // Only correct if l_until2_sub < l
        unput(chars[l-i]);
      }
      return;
    }
    else if(!strncmp(chars,until,l_until) && !is_alpha(c_next_until)){
      for(int i = 1; i <= l-l_until; i++){
        unput(chars[l-i]);
      }
      if(!nb_skip){
	return;
      }
      else{
	nb_skip--;
      }
    }
    else if(skip && !strncmp(chars,skip,l_skip) && !is_alpha(c_next_skip)){
      nb_skip++;
    }
    else{
      for(i=1;i<l-1;i++){
	unput(chars[l-i]);
      }
    }

  }
}

void gmsh_yyflush() { YY_FLUSH_BUFFER; }
