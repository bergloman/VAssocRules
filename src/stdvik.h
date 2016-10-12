#pragma once

////////////////////////////////////////////////////////
// Module: Index of standard libraries
// Author: Viktor Jovanoski
// Desc:   Includes all other libraries and also 
//         defines some handy macros
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// Libraries
////////////////////////////////////////////////////////

#include "ds.h"
#include "str.h"
#include "lx.h"
#include "binfilestream.h"
#include "datatable.h"
#include <time.h>
//#include <conio.h>

namespace VikStd {

    ////////////////////////////////////////////////////////
    // Macros
    ////////////////////////////////////////////////////////

    // CONSOLE_EXIT - at the end of execution of console program
    // wait for user to press some key (after (s)he saw the results)
#define CONSOLE_EXIT { char x; x=(char)getch; } 

// printf_newline - just prints new line to stdout
#define printf_newline printf("\n")
#define printf_PChar(X) printf("%s",X)

// obvious functions
#define ABS(X) ((X<0) ? (-(X)) : (X))
#define MAX(X,Y) ((X<Y) ? (Y) : (X))
//#define MIN(X,Y) ((X>Y) ? (Y) : (X))
#define SQR(X) ((X)*(X))
#define PI (3.1415926535)

    inline double log2(double t) { return log(t) / log(2.0); };
    //inline double log10(double t) { return log(t)/log(10); };

    inline int Trunc(double d) { int i = (int)d; return i; };

    inline int Round(double d) {
        int i = (int)d;
        if (Trunc(d + 0.5) > i) i++;
        return i;
    };

    /////////////////////////////////////////////////////////
    // A simple variant type
    /////////////////////////////////////////////////////////

    typedef enum { // possible types of "variant"
        VTInt, VTChar, VTBool, VTDouble, VTStr, VTNull
    } TVarType_enum;

    struct TVar { // something like "variant"
        TVarType_enum m_Type;
        union {
            int m_Int;
            double m_Double;
            bool m_Bool;
            char m_Char;
        };
        TStr m_Str; // it's left out of the union because of possible construction/destruction conflicts

        TVar() { m_Type = VTNull; };
        // utility functions
        void SetInt(int i) { m_Type = VTInt; m_Int = i; };
        void SetDouble(double i) { m_Type = VTDouble; m_Double = i; };
        void SetBool(bool i) { m_Type = VTBool; m_Bool = i; };
        void SetChar(char i) { m_Type = VTChar; m_Char = i; };
        void SetStr(TStr &i) { m_Type = VTStr; m_Str = i; };
        void SetStr(char *i) { m_Type = VTStr; m_Str = i; };
    };

    typedef TPair<TStr, TVar> TProp;

    /////////////////////////////////////////////////////////
    // TProperties - utility class for easy exchange of
    // parameters and/or properties - uses variant TVar.
    /////////////////////////////////////////////////////////

    class TProperties {
        TVec<TProp> m_Vec;
    public:

        TProperties() {};

        void Add(TPair<TStr, TVar> p) { m_Vec.Add(p); };

        bool Find(TStr a, TVar &result) {
            for (int i = 0; i < m_Vec.Len(); i++) {
                if (m_Vec[i].m_c1 == a) {
                    result = m_Vec[i].m_c2;
                    return true;
                };
            };
            return false;
        };

        bool Find(char *ta, TVar &result) { TStr temp(ta); return Find(temp, result); };
        void Clear() { m_Vec.Clear(); };
    };

    ///////////////////////////////////////////////////////////
    // TVTime - utility class for easy time formating
    // and manipulation.
    ///////////////////////////////////////////////////////////

    class TVTime {

        time_t t1;
        int msec, sec, min, hour;
        int day, month, year;

    public:

        struct tm *m_Tm;

        TVTime() { msec = 0; sec = 0; min = 0; hour = 0; };
        void Now() { // get current time
            time(&t1);
            m_Tm = localtime(&t1);
            sec = m_Tm->tm_sec;
            min = m_Tm->tm_min;
            hour = m_Tm->tm_hour;
            day = m_Tm->tm_mday;
            month = m_Tm->tm_mon;
            year = m_Tm->tm_year;
        };

        void operator-=(const TVTime &t) { // applicable only to msec, sec, min, hour
            while (msec < t.msec) { msec += 1000; sec--; };
            msec -= t.msec;
            while (sec < t.sec) { sec += 60; min--; };
            sec -= t.sec;
            while (min < t.min) { msec += 60; hour--; };
            min -= t.min;
            hour -= t.hour;
        };

        void SetTime(time_t xt) { t1 = xt; m_Tm = localtime(&t1); }; //set time
        int GetHour() { return hour; };
        int GetMin() { return min; };
        int GetSec() { return sec; };
        int GetDayOfMonth() { return day; };
        //int GetDayOfWeek(){ return wday; };
        int GetMonth() { return month; };
        int GetYear() { return year; };

        TStr GetDateAsStr(char *format) { return GetDateAsStr(TStr(format)); };
        TStr GetDateAsStr(TStr format) {
            int i = 0;
            TStr res;
            while (i < format.Len()) {
                if (format[i] != '%') { res += format[i]; } else {
                    i++;
                    if (i < format.Len()) {
                        if ((format[i] == 'd') || (format[i] == 'D')) { res += GetDayOfMonth(); } else if ((format[i] == 'm') || (format[i] == 'M')) { res += GetMonth(); } else if ((format[i] == 'y') || (format[i] == 'Y')) {
                            if ((GetYear() >= 100) && (GetYear() < 110)) { res += "200"; } else if (GetYear() >= 110) { res += "20"; } else res += "19";
                            res += GetYear() % 100;
                        } else if (format[i] == '%') { res += '%'; }
                    };
                };
                i++;
            };
            return res;
        };

        TStr GetTimeAsStr(char *format) { return GetTimeAsStr(TStr(format)); };
        TStr GetTimeAsStr(TStr format) {
            int i = 0;
            TStr res;
            while (i < format.Len()) {
                if (format[i] != '%') { res += format[i]; } else {
                    i++;
                    if (i < format.Len()) {
                        if ((format[i] == 'h') || (format[i] == 'H')) { res += GetHour(); } else if ((format[i] == 'm') || (format[i] == 'M')) { res += GetMin(); } else if ((format[i] == 's') || (format[i] == 'S')) { res += GetSec(); } else if (format[i] == '%') { res += '%'; }
                    };
                };
                i++;
            };
            return res;
        };

    };

    ///////////////////////////////////////////////////////////
    // TVTimer - utility class for neat timer
    // Use Start() and Stop() functions to start and stop timer
    // and after that query m_Diff member for time difference.
    ///////////////////////////////////////////////////////////

    class TVTimer {
        TVTime t1;
    public:
        TVTime m_Diff;
        void Start() { t1.Now(); };
        void Stop() { m_Diff.Now(); m_Diff -= t1; };
    };

    //////////////////////////////////////////////////////////
    // String and character utilities
    //////////////////////////////////////////////////////////

    inline bool IsWhiteSpace(char c) {
        if ((c == ' ') || (c == '\t') || (c == 10) || (c == 13)) return true;
        return false;
    };

    inline bool IsDigit(char c) {
        if ((c >= '0') && (c <= '9')) return true;
        return false;
    };

    //////////////////////////////////////////////////////////

    inline int ToInteger(char *s, char *Message = NULL) {
        int r = 0, i = 0;
        TStr a = s;
        while ((i < a.Len()) && (IsWhiteSpace(a.read(i) != false))) i++; // skip trailling blanks
        while (i < a.Len()) {
            if (IsDigit(a.read(i)) == true) { r = r * 10 + (a.read(i) - '0'); i++; } else {
                if (Message == NULL) {
                    TStr x = "ToInteger: Invalid parameter: '";
                    x += s;
                    x += "'";
                    throw TExc(x);
                };
                throw TExc(Message);
            };
        };
        return r;
    };

    inline int ToInteger(TStr &s, char *Message = NULL) {
        return ToInteger(s.c_str(), Message);
    };

    //////////////////////////////////////////////////////////

    inline double ToFloat(char *s, char *Message = NULL) {

        double r = atof(s);
        if (r != 0.0) return r;

        int i = 0;
        bool Dot = false;
        TStr a(s);

        while ((i < a.Len()) && (IsWhiteSpace(a.read(i) != false))) i++; // skip trailling blanks
        while (i < a.Len()) {
            if (a.read(i) == '0') { i++; } else if (a.read(i) == '.') {
                if (Dot == false) { Dot = true; i++; } else {
                    if (Message == NULL) {
                        TStr x = "ToInteger: Invalid parameter: '";
                        x += s;	x += "'";	throw TExc(x);
                    };
                    throw TExc(Message);
                };
            };
        };
        return r;
    };

    inline double ToFloat(TStr &s, char *Message = NULL) {
        return ToFloat(s.c_str(), Message);
    };

    ///////////////////////////////////////////////////////////
    // split and trimm words in a string into TVec<TStr>
    // Example:
    // s=" amms   534 hjwargf:"{}  aewryt_9 _ 765"
    // vec=["amms","534","hjwargf:"{}","aewryt_9","_","765"]

    inline void SplitIntoWords(TStr &s, TVec<TStr> &vec) {
        vec.Clear();
        int pos1 = 0, pos2 = 0;
        while ((pos1 < s.Len()) && (s[pos1] == ' ')) pos1++; // skipp trailing blanks
        pos2 = MAX(pos1 - 1, 0);
        while (pos1 < s.Len()) {
            if (s[pos1] == ' ') {
                vec.Add();
                vec.Last() = s.Mid(pos2, pos1 - pos2);
                vec.Last().TrimLeftRight();
                while ((pos1 < s.Len()) && (s[pos1] == ' ')) pos1++;
                pos2 = pos1;
                pos1--;
            };
            pos1++;
        };
        if (pos2 < pos1) {
            vec.Add();
            vec.Last() = s.Mid(pos2, pos1 - pos2);
            vec.Last().TrimLeftRight();
        };
    };

    ///////////////////////////////////////////////////////////

    class TFileTagReplacer {

        FILE *fpi;
        FILE *fpo;
        TStr m_LastParam;

        TStr GetParamName() {
            char c = fgetc(fpi);
            TStr p;
            while (c != '>') {
                if (feof(fpi)) return TStr("");
                p += c;
                c = fgetc(fpi);
            };
            m_LastParam = p;
            return p;
        };

    public:
        TProperties m_Param;

        void ProcessFile(TStr &InFile, TStr &OutFile) { // processes input file and writes results in output file
            fpi = fopen(InFile.c_str(), "r");
            if (fpi == NULL) throw TExc("TFileTagReplacer.ProcessFile: Cannot open input file");
            FILE *fpo = fopen(OutFile.c_str(), "W");
            if (fpo == NULL) {
                fclose(fpi);
                throw TExc("TFileTagReplacer.ProcessFile: Cannot open output file");
            };

            try {
                char c = fgetc(fpi);
                while (!feof(fpi)) {
                    if (c == '<') {
                        c = fgetc(fpi);
                        if (feof(fpi)) { fclose(fpi); fclose(fpo); return; };
                        if (c == '!') {
                            c = fgetc(fpi);
                            if (feof(fpi)) { fclose(fpi); fclose(fpo); return; };
                            if (c == '#') {
                                TVar var;
                                if (m_Param.Find(GetParamName(), var) == false) { fprintf(fpo, "<!#%s>", m_LastParam.c_str()); } else {
                                    fprintf(fpo, "%s", var.m_Str.c_str());
                                };
                            };
                        } else fputc(c, fpo);
                    } else fputc(c, fpo);
                };
            } catch (...) { fclose(fpi); fclose(fpo); throw; };
            fclose(fpi); fclose(fpo);
        };
    };

    ///////////////////////////////////////////////////////////


    class TCommon {

        TOut *m_Out;
    public:

        TCommon(TOut *o) { m_Out = o; };

        virtual void Notify(TStr s) { m_Out->Put(s); };
        virtual void Notify(char *s) { m_Out->Put(s); };
        virtual void Notify(char c) { m_Out->Put(c); };

        virtual void NotifyP(TStr s) { Notify(s); Post(); };
        virtual void NotifyP(char *s) { Notify(s); Post(); };
        virtual void NotifyP(char c) { Notify(c); Post(); };

        virtual void Post() { m_Out->Put('\n'); };
    };

}
