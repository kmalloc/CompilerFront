#include "RegExpTokenizer.h"

#include <string>
#include "LexException.h"

bool RegExpTokenizer::IsCharEscape(const char* ps, const char* pc)
{
    const char* p = pc - 1;
    if (*p != '\\') return false;

    int c = 1;
    --p;

    while (p >= ps && *p == '\\')
    {
        c++;
        p--;
    }

    return c%2;
}

static inline bool IsPlaceHolderMetaChar(char c)
{
    return (c == '.' || c == '^' || c == '$');
}

static inline bool IsRegExpMetaChar(char c)
{
    return (IsPlaceHolderMetaChar(c)
            || c == '*' || c == '+' || c == '?'
            || c == '|' || c == '[' || c == ']'
            || c == '(' || c == ')' || c == '{'
            || c == '}' || c == '\\');
}

bool RegExpTokenizer::IsMetaChar(char c) const
{
    return IsRegExpMetaChar(c);
}

bool RegExpTokenizer::CanCharEscape(char c)
{
    return IsRegExpMetaChar(c) || (c == 's' || c == 'w' || c == 'd');
}

const char* RegExpTokenizer::IsToken(const char* s, const char* e) const
{
    if (*s == '[')
    {
        const char* p = s + 1;
        while (*p != ']' && p <= e)
        {
            p++;
        }

        if (*p == ']' && p == e) return p;

        return NULL;
    }

    if (*s == '\\' && CanCharEscape(*(s + 1)) && s + 1 == e) return s;

    if (IsPlaceHolderMetaChar(*s) && s == e) return s;

    if (s < e) return NULL;

    if (IsRegExpMetaChar(*s)) return NULL;

    return s;
}

const char* RegExpTokenizer::IsToken(const std::string& str) const
{
    if (str.empty()) return false;

    return IsToken(&str[0], &str[str.size() - 1]);
}

bool RegExpTokenizer::ExtractRepeatCount(const char* ps, const char* pe, int& min, int& max) const
{
    // {12,23}
    // {,23} -> {0,23}

    assert(*ps == '{');

    ++ps;

    int f = 0;
    int l = 0;
    while (*ps != ',' && *ps != '}' && ps <= pe)
    {
       if (std::isdigit(*ps))
       {
           f = f * 10 + (*ps - '0');
       }
       else if (*ps != ' ')
       {
           throw LexErrException("invalid repeat number, left", ps);
       }

       ++ps;
    }

    if (*ps == '}')
    {
        min = max = f;
        return true;
    }

    ps++;
    while (*ps != '}' && ps <= pe)
    {
        if (std::isdigit(*ps))
        {
            l = l * 10 + (*ps - '0');
        }
        else if (*ps != ' ')
        {
            throw LexErrException("invalid repeat number, right", ps);
        }

        ++ps;
    }

    if (l == 0)
    {
        // {2, } --> {2, INT_MAX}
        l = INT_MAX;
    }

    if (f > l)
    {
        throw LexErrException("invalid repeat number, min > max", ps);
    }

    min = f;
    max = l;
    return true;
}


void RegExpTokenizer::ExtractRegUnit(const char* ps, const char* pe,
        const char*& us, const char*& ue,
        const char*& bu, const char*& au) const
{
    char ec = *pe;
    const char* p = pe;

    if ((ec == '+' || ec == '*' || ec == '?') && !IsCharEscape(ps, p)) // *(p - 1) != '\\')
    {
        // quantifiers are not regarded as 'unit'
        --p;
        ec = *p;
    }

    if ((ec == ')' || ec == '}' || ec == ']') && !IsCharEscape(ps, p)) // *(p - 1) != '\\')
    {
        // case 1: ((abc)eef)
        // case 2: ((ab))
        // case 3: (ab(cc)ef)
        // case 4: (ab(eff))
        int cc = 1;
        char mc = '[';

        if (ec != ']') mc = (ec == ')')? '(' : '{';

        const char* ptmp = p--;
        while (p >= ps)
        {
            if (!IsCharEscape(ps, p)) // *(p - 1) != '\\')
            {
                if (*p == ec)
                {
                    cc++;
                }
                else if (*p == mc && cc == 1)
                {
                    break;
                }
                else if (*p == mc)
                {
                    cc--;
                }
            }

            p--;
        }

        // error, no matching () or {}
        if (*p != mc) throw LexErrException("unmatch parenthesis:\"(\", \"{\", or \"[\".", p);

        if (ec == ']')
        {
            us = p;
            ue = ptmp;
        }
        else if (ec == '}')
        {
            ExtractRegUnit(ps, p - 1, us, ue, bu, au);
            if (au != p) throw LexErrException("invalid expression before {}", p);

            return;
        }
        else
        {
            us = p + 1;
            ue = ptmp - 1;

            if (ue < us)
            {
                // null unit ()
                us = NULL;
                ue = NULL;
            }
        }

        bu = p - 1;
        au = ptmp + 1;
        return;
    }

    if ((ec == '(' || ec == '{' || ec == '[' | ec == '|'
            || ec == '*' || ec == '?' || ec == '+') && !IsCharEscape(ps, p)) // *(p - 1) != '\\')
    {
        throw LexErrException("invalid occurance of meta-character", p);
    }

    ue = p;
    au = p + 1;

    if (IsCharEscape(ps, p)) // *(p - 1) == '\\')
    {
        // escape meta-character
        if (!CanCharEscape(ec)) throw LexErrException("invalid escape character", p);

        us = p - 1;
        bu = p - 2;
    }
    else
    {
        us = p;
        bu = p - 1;
    }

    return;
}

