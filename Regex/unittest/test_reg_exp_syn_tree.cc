#include "gtest/gtest.h"

#include <string>
#include <sstream>
#include <climits>
#include <stdlib.h>

#define private public

#include "RegExpAutomata.h"
#include "RegExpTokenizer.h"
#include "RegExpSyntaxTree.h"

/*
positive case:
1) (a|b)*ccb
2) (a|b)+ccb(ef|vv)?
3) a*
4) (cc)a+
5) (c|a)?
6) (c|a){1, 4}
7) (eea){, 4}
8) a{1, }

*/

using namespace std;

#define ArrSize(arr) (sizeof(arr)/sizeof(arr[0]))

class RepeatCount
{
    public:

        RepeatCount(const string& txt, int min, int max)
            :min_(min), max_(max), txt_(txt)
        {
        }

        const string& GetText() const { return txt_; }

        int GetMin() const { return min_; }
        int GetMax() const { return max_; }

    private:

        int min_;
        int max_;
        string txt_;
};


TEST(test_extract_repeat_count, test_reg_exp_automata_gen)
{
    RepeatCount cases[] =
    {
        RepeatCount("{1}", 1, 1),
        RepeatCount("{1, }", 1, INT_MAX),
        RepeatCount("{,}", 0, INT_MAX),
        RepeatCount("{,23}", 0, 23),
        RepeatCount("{,  33}", 0, 33),
        RepeatCount("{11,  33}", 11, 33)
    };

    RegExpTokenizer tokenizer;

    int min, max;
    for (int i = 0; i < ArrSize(cases); ++i)
    {
        string s = cases[i].GetText();
        tokenizer.ExtractRepeatCount(&s[0], &s[s.size() - 1], min, max);

        EXPECT_EQ(cases[i].GetMin(), min) << " case: " << i << endl;
        EXPECT_EQ(cases[i].GetMax(), max) << " case: " << i << endl;
    }
}


class RegUnit
{
    public:

        RegUnit(const char* txt, int us, int ue, int bu, int au)
            :txt_(txt), us_(txt + us), ue_(txt + ue), bu_(txt + bu), au_(txt + au)
        {
        }

        RegUnit(const char* txt, int bu, int au)
            :txt_(txt), us_(NULL), ue_(NULL), bu_(txt + bu), au_(txt + au)
        {
        }

        const char* GetText() const { return txt_; }
        const char* GetUnitStart() const { return us_; }
        const char* GetUnitEnd() const { return ue_; }

        const char* GetBeforeUnit() const { return bu_; }
        const char* GetAfterUnit() const { return au_; }

    private:

        const char* txt_;
        const char* us_;
        const char* ue_;
        const char* bu_;
        const char* au_;
};

TEST(test_extract_unit, test_reg_exp_automata_gen)
{
    RegUnit cases[] =
    {
        RegUnit(".", 0, 0, -1, 1),
        RegUnit(".*", 0, 0, -1, 1),
        RegUnit("\\\\", 0, 1, -1, 2),
        RegUnit("\\.", 0, 1, -1, 2),
        RegUnit("\\*", 0, 1, -1, 2),
        RegUnit("\\s+", 0, 1, -1, 2),
        RegUnit("(\\d)*", 1, 2, -1, 4),
        RegUnit("c\\w+", 1, 2, 0, 3),
        RegUnit("\\?", 0, 1, -1, 2),
        RegUnit("\\+", 0, 1, -1, 2),
        RegUnit("a\\(", 1, 2, 0, 3),
        RegUnit("a\\[", 1, 2, 0, 3),
        RegUnit("a\\{", 1, 2, 0, 3),
        RegUnit("vb(ab\\()*", 3, 6, 1, 8),
        RegUnit("a", 0, 0, -1, 1),
        RegUnit("a*", 0, 0, -1, 1),
        RegUnit("abc", 2, 2, 1, 3),
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        RegUnit("\\0abc", 4, 4, 3, 5),
#endif
        RegUnit("abc+", 2, 2, 1, 3),
        RegUnit("abc.?", 3, 3, 2, 4),
        RegUnit("abc{1,2}", 2, 2, 1, 3),
        RegUnit("(abc){1,2}", 1, 3, -1, 5),
        RegUnit("()", -1, 2),
        RegUnit("(a)", 1, 1, -1, 3),
        RegUnit("abc(ef\\\\|\\w*)", 4, 11, 2, 13), // 22
        RegUnit("abc(ef\\\\)\\\\\\)", 11, 12, 10, 13),
        RegUnit("((a))", 1, 3, -1, 5),
        RegUnit("(a)*", 1, 1, -1, 3),
        RegUnit("(\\0ab)", 1, 4, -1, 6),
        RegUnit("(a|b|(\\s+))+", 1, 9, -1, 11),
        RegUnit("(abc|efg)?", 1, 7, -1, 9),
        RegUnit("(ab)(efg)(vv)*", 10, 11, 8, 13),
        RegUnit("([abc]+\\w)*(a|b)+", 12, 14, 10, 16),
        RegUnit("(abc)+\\d((ev){2,5})?", 9, 17, 7, 19), 
    };

    RegExpTokenizer tokenizer;
    const char* us, *ue, *bu, *au;

    for (int i = 0; i < ArrSize(cases); ++i)
    {
        const char* s = cases[i].GetText();
        tokenizer.ExtractRegUnit(s, s + strlen(s) - 1, us, ue, bu, au);

        EXPECT_EQ(cases[i].GetUnitStart(), us) << "us case: " << i << endl;
        char buf[8];
        EXPECT_EQ(cases[i].GetUnitEnd(), ue) << "ue case: " << i << endl;
        EXPECT_EQ(cases[i].GetBeforeUnit(), bu) << "bu case: " << i << endl;
        EXPECT_EQ(cases[i].GetAfterUnit(), au) << "au case: " << i << endl;
    }
}

static string ConstructRegExpFromSynTree(RegExpSynTreeNode* root)
{
    if (!root) return "";

    string out;
    RegExpSynTreeNode* left = dynamic_cast<RegExpSynTreeNode*>(root->GetLeftChild());
    RegExpSynTreeNode* right = dynamic_cast<RegExpSynTreeNode*>(root->GetRightChild());

    string ls = ConstructRegExpFromSynTree(left);
    string rs = ConstructRegExpFromSynTree(right);

    if (root->GetNodeType() == RegExpSynTreeNodeType_Or)
    {
        out = ls + "|" + rs;
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Concat)
    {
        out = ls + rs;
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Star)
    {
        RegExpSynTreeStarNode* sn = dynamic_cast<RegExpSynTreeStarNode*>(root);
        int min = sn->GetMinRepeat();
        int max = sn->GetMaxRepeat();
        if (min == 0 && max == INT_MAX)
        {
            out = ls + rs + "*";
        }
        else if (min == 1 && max == INT_MAX)
        {
            out = ls + rs + "+";
        }
        else if (min == 0 && max == 1)
        {
            out = ls + rs + "?";
        }
        else
        {
            ostringstream ss1, ss2;

            if (min != INT_MAX) ss1 << min;
            if (max != INT_MAX) ss2 << max;

            out = ls + rs + "{" + ss1.str() + "," + ss2.str() + "}";
        }
    }
    else if (root->IsLeafNode())
    {
        RegExpSynTreeLeafNode* sn = dynamic_cast<RegExpSynTreeLeafNode*>(root);
        out = root->GetOrigText();

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        if (sn->GetLeafNodeType() == RegExpSynTreeNodeLeafNodeType_Ref)
        {
            char buf[8];
            RegExpSynTreeRefNode* rn = dynamic_cast<RegExpSynTreeRefNode*>(sn);
            snprintf(buf, sizeof(buf), "%d", rn->GetRef());
            out = "\\" + string(buf);
        }
#endif
    }
    else
    {
        EXPECT_TRUE(0) << "invalid node type" << endl;
    }

    int unit = root->IsUnit();

    if (unit)
    {
        string lp(unit, '(');
        string rp(unit, ')');
        return lp + out + rp;
    }

    return out;
}

TEST(test_construct_reg_syn_tree, test_reg_exp_automata_gen)
{
    const char* cases[] =
    {
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        "(\\0df)",
        "a(bc)(\\0df)(g\\1)e",
#endif
        "\\+vv", // 3
        ".*",
        ".?", // 1
        ".+", // 2
        "\\*v", // 4
        "\\s+.*", // 5
        "(\\w+)+\\s(a.)*[abc]+\\d$", // 6
        "(\\s?)+\\w(a.)*", // 6
        "\\.ee", // 7
        "\\{v", // 8
        "(ab)", // 9
        "((abc)+ef)", // 10
        "()", // 11
        "((a))", // 12
        "((ba\\d))", // 13
        "(((a)))", // 14
        "((ab)+ve(ss)e)", // 15
        "(a|b)*ccb+", // 16
        "abf|ef.f", // 17
        "abc(ef\\\\)\\\\ab\\*\\\\\\+", // 18
        "(abf)|eff", // 19
        "abc|efg|vvv|efff", // 20
        "[ve f]+abc$", // 21
        "[ve*f+]+abc$", // 22
        "[vef]{23,}abc+$", // 23
        "ab\\\\|ef\\+e|vv\\|", // 24
        "((fae)|(abcd))+|vv\\[ee(fff|vvvv)*",
        "^abc(ef){1,23}([abcefg]{1,4}|gve)",
        "(abc)+\\d((ev){2,5})?",
        "a",
        "[ve-jf]+abc", // 21
        "[^ve-jf]+abc", // 21
        "[b^ve-jf]+abc", // 21
        "ab[^qwerty]vn",
    };

    RegExpSyntaxTree regSynTree;
    for (int i = 0; i < ArrSize(cases); ++i)
    {
        try
        {
            regSynTree.BuildSyntaxTree(cases[i], cases[i] + strlen(cases[i]) - 1);
            SynTreeNodeBase* node = regSynTree.GetSynTree();

            if (!node) continue;
            string out = ConstructRegExpFromSynTree(dynamic_cast<RegExpSynTreeNode*>(node));

            EXPECT_STREQ(cases[i], out.c_str()) << "case:" << i << " failed" << endl;
        }
        catch (...)
        {
            EXPECT_TRUE(0) << "case: " << i << " failed" << endl;
        }
    }
}


class RegToken
{
    public:

        RegToken(const char* s, bool isToken)
            :isToken_(isToken), txt_(s)
        {
        }

        const char* GetText() { return txt_; }
        bool IsToken() const { return isToken_; }

    private:

        bool isToken_;
        const char* txt_;
};

TEST(test_reg_exp_tokenizer, test_reg_exp_automata_gen)
{
    RegToken cases[] =
    {
        RegToken("*", false),
        RegToken("?", false),
        RegToken("+", false),
        RegToken("|", false),
        RegToken("[", false),
        RegToken("]", false),
        RegToken("(", false),
        RegToken(")", false),
        RegToken("{", false),
        RegToken("}", false),
        RegToken("\\d", true),
        RegToken("\\s", true),
        RegToken("\\w", true),
        RegToken("\\*", true),
        RegToken("\\?", true),
        RegToken("\\+", true),
        RegToken("\\|", true),
        RegToken("\\[", true),
        RegToken("\\]", true),
        RegToken("\\(", true),
        RegToken("\\)", true),
        RegToken("\\{", true),
        RegToken("\\}", true),
        RegToken("\\\\", true),
        RegToken(".", true),
        RegToken("c", true),
        RegToken("b", true),
        RegToken("a", true),
        RegToken("cc", false),
        RegToken("c*", false),
        RegToken("b+", false),
        RegToken("c?", false),
        RegToken("c{12, 23}", false),
        RegToken("ceef", false),
    };

    RegExpTokenizer tokenizer;
    for (int i = 0; i < ArrSize(cases); ++i)
    {
        const char* s = cases[i].GetText();
        const char* e = s + strlen(s) - 1;

        EXPECT_EQ(cases[i].IsToken(), tokenizer.IsToken(s, e) != NULL) << "case:" << i << " failed." << endl;
    }
}

class StrToken
{
    public:

        StrToken(const char* str, const char* expect)
            :str_(str), expect_(expect)
        {
        }

        void test(const std::string& s)
        {
            EXPECT_STREQ(expect_.c_str(), s.c_str()); // << "actual:" << s << endl << "expect:" << expect_ << endl;
        }

        const std::string& get() const { return str_; }

    private:

        std::string str_;
        std::string expect_;
};

std::string GenNegString(const std::string& s)
{
    string neg;
    for (int i = 1; i < STATE_TRAN_MAX - 1; ++i)
    {
        if (s.find(i) != std::string::npos) continue;

        neg.push_back(i);
    }

    return neg;
}

TEST(test_string_constructor, test_reg_exp_tokenizer)
{
    StrToken cases[] =
    {
        StrToken("abcd", "abcd"),
        StrToken("abcdnnnw", "abcdnw"),
        StrToken("1234", "1234"),
        StrToken("vb-f\\-", "-\\bcdefv"),
        StrToken("^1234", GenNegString("1234").c_str()),
        StrToken("^qwerty", GenNegString("qwerty").c_str()),
        StrToken("^qwa-d\\-p", GenNegString("qwabcd-p").c_str()),
        StrToken("ab-fi-l", "abcdefijkl"),
        StrToken("ab-fg2-5l", "2345abcdefgl"),
    };

    for (int i = 0; i < ArrSize(cases); ++i)
    {
        const char* s = &((cases[i].get())[0]);
        const char* e = &((cases[i].get())[cases[i].get().size() - 1]);
        cases[i].test(RegExpTokenizer::ConstructOptionString(s, e));
    }

    StrToken cases2[] =
    {
        StrToken("\\w", "aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"),
        StrToken("\\d", "0123456789"),
        StrToken("\\s", " "),
        StrToken("\\g", "g"),
    };

    for (int i = 0; i < ArrSize(cases2); ++i)
    {
        const char* s = &((cases2[i].get())[0]);
        const char* e = &((cases2[i].get())[cases2[i].get().size() - 1]);
        cases2[i].test(RegExpTokenizer::ConstructEscapeString(s, e));
    }
}
