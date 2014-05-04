#include "gtest.h"

#include <map>
#include <string>
#include <sstream>
#include <climits>
#include <sstream>
#include <algorithm>

#define private public
#define protected public

#include "RegExpAutomata.h"
#include "RegExpSyntaxTree.h"

class RegExpNFACase
{
    public:

        typedef std::map<int, std::map<int, std::vector<int> > > nfa_set;
        RegExpNFACase(const char* txt):txt_(txt)
        {
        }

    private:

        const char* txt_;
        nfa_set states_;
};

TEST(test_reg_exp_nfa_gen, test_automata_gen)
{
    std::vector<RegExpNFACase> cases;

    RegExpNFACase c1("a");
    c1.states_[0]['a'].push_back(1);
    cases.push_back(c1);

    RegExpNFACase c2("ab");
    c2.states_[0]['a'].push_back(1);
    c2.states_[1]['b'].push_back(3); // state 2 will be merged into 1
    cases.push_back(c2);

    RegExpNFACase c2_0("abcde");
    c2_0.states_[0]['a'].push_back(1);
    c2_0.states_[1]['b'].push_back(3); // state 2 will be merged into 1
    c2_0.states_[3]['c'].push_back(5);
    c2_0.states_[5]['d'].push_back(7);
    c2_0.states_[7]['e'].push_back(9);
    cases.push_back(c2_0);

    RegExpNFACase c2_1("a*b");
    c2_1.states_[0][STATE_EPSILON].push_back(1);
    c2_1.states_[0][STATE_EPSILON].push_back(3);
    c2_1.states_[1]['a'].push_back(2);
    c2_1.states_[2][STATE_EPSILON].push_back(1);
    c2_1.states_[2][STATE_EPSILON].push_back(3);
    cases.push_back(c2_1);

    RegExpNFACase c2_2("a.b");
    c2_2.states_[0]['a'].push_back(1);
    for (int i = 0; i < STATE_TRAN_MAX - 1; ++i)
    {
        c2_2.states_[1][i].push_back(3);
    }
    c2_2.states_[3]['b'].push_back(5);
    cases.push_back(c2_2);

    RegExpNFACase c2_3("a+b");
    c2_3.states_[0]['a'].push_back(1);
    c2_3.states_[1][STATE_EPSILON].push_back(0);
    c2_3.states_[1][STATE_EPSILON].push_back(2);
    c2_3.states_[2]['b'].push_back(4);
    cases.push_back(c2_3);

    RegExpNFACase c2_4("abc");
    c2_4.states_[0]['a'].push_back(1);
    c2_4.states_[1]['b'].push_back(3);
    c2_4.states_[3]['c'].push_back(5);
    cases.push_back(c2_4);

    RegExpNFACase c2_5("(ab)c");
    c2_5.states_[0]['a'].push_back(1);
    c2_5.states_[1]['b'].push_back(3);
    c2_5.states_[3]['c'].push_back(5);
    cases.push_back(c2_5);

    // case 8
    RegExpNFACase c3("ab*");
    c3.states_[0]['a'].push_back(1);
    c3.states_[1][STATE_EPSILON].push_back(5);
    c3.states_[1][STATE_EPSILON].push_back(3);
    c3.states_[3]['b'].push_back(4);
    c3.states_[4][STATE_EPSILON].push_back(3);
    c3.states_[4][STATE_EPSILON].push_back(5);
    cases.push_back(c3);

    RegExpNFACase c4("(ab)*");
    c4.states_[0][STATE_EPSILON].push_back(1);
    c4.states_[0][STATE_EPSILON].push_back(5);
    c4.states_[1]['a'].push_back(2);
    c4.states_[2]['b'].push_back(4);
    c4.states_[4][STATE_EPSILON].push_back(1);
    c4.states_[4][STATE_EPSILON].push_back(5);
    cases.push_back(c4);

    // case 10
    RegExpNFACase c5("(ab)+");
    c5.states_[0]['a'].push_back(1);
    c5.states_[1]['b'].push_back(3);
    c5.states_[3][STATE_EPSILON].push_back(0);
    c5.states_[3][STATE_EPSILON].push_back(4);
    cases.push_back(c5);

    RegExpNFACase c6("(ab)?");
    c6.states_[0][STATE_EPSILON].push_back(1);
    c6.states_[0][STATE_EPSILON].push_back(5);
    c6.states_[1]['a'].push_back(2);
    c6.states_[2]['b'].push_back(4);
    c6.states_[4][STATE_EPSILON].push_back(5);
    cases.push_back(c6);

    RegExpNFACase c7("(a|b)?");
    c7.states_[0][STATE_EPSILON].push_back(1);
    c7.states_[0][STATE_EPSILON].push_back(7);
    c7.states_[1][STATE_EPSILON].push_back(2);
    c7.states_[1][STATE_EPSILON].push_back(4);
    c7.states_[2]['a'].push_back(3);
    c7.states_[4]['b'].push_back(5);
    c7.states_[3][STATE_EPSILON].push_back(6);
    c7.states_[5][STATE_EPSILON].push_back(6);
    c7.states_[6][STATE_EPSILON].push_back(7);
    cases.push_back(c7);

    // case 13
    RegExpNFACase c8("(a|b){2,}[abce]\\d$");
    c8.states_[0][STATE_EPSILON].push_back(1);
    c8.states_[0][STATE_EPSILON].push_back(3);
    c8.states_[1]['a'].push_back(2);
    c8.states_[3]['b'].push_back(4);
    c8.states_[2][STATE_EPSILON].push_back(5);
    c8.states_[4][STATE_EPSILON].push_back(5);
    c8.states_[5][STATE_EPSILON].push_back(6);

    c8.states_[6][STATE_EPSILON].push_back(7);
    c8.states_[6][STATE_EPSILON].push_back(9);
    c8.states_[7]['a'].push_back(8);
    c8.states_[9]['b'].push_back(10);
    c8.states_[8][STATE_EPSILON].push_back(11);
    c8.states_[10][STATE_EPSILON].push_back(11);
    c8.states_[11][STATE_EPSILON].push_back(6);
    c8.states_[11][STATE_EPSILON].push_back(12);

    c8.states_[12]['a'].push_back(14);
    c8.states_[12]['b'].push_back(14);
    c8.states_[12]['c'].push_back(14);
    c8.states_[12]['e'].push_back(14);

    for (int i = '0'; i <= '9'; ++i)
    {
        c8.states_[14][i].push_back(16);
    }

    c8.states_[16][STATE_EPSILON].push_back(18);
    cases.push_back(c8);

    RegExpNFACase c9("(ab){3}");
    c9.states_[1]['a'].push_back(2);
    c9.states_[2]['b'].push_back(4);
    c9.states_[5]['a'].push_back(6);
    c9.states_[6]['b'].push_back(8);
    c9.states_[9]['a'].push_back(10);
    c9.states_[10]['b'].push_back(12);
    c9.states_[12][STATE_EPSILON].push_back(0);
    cases.push_back(c9);

    // case 14
    RegExpNFACase c10("([abc]+\\d)*(a|b)+3\\w2e");
    c10.states_[0][STATE_EPSILON].push_back(1);
    c10.states_[0][STATE_EPSILON].push_back(6);
    c10.states_[1]['a'].push_back(2);
    c10.states_[1]['b'].push_back(2);
    c10.states_[1]['c'].push_back(2);
    c10.states_[2][STATE_EPSILON].push_back(1);
    c10.states_[2][STATE_EPSILON].push_back(3);

    for (int i = '0'; i <= '9'; ++i)
    {
        c10.states_[3][i].push_back(5);
    }

    c10.states_[5][STATE_EPSILON].push_back(6);
    c10.states_[5][STATE_EPSILON].push_back(1);
    c10.states_[6][STATE_EPSILON].push_back(8);
    c10.states_[6][STATE_EPSILON].push_back(10);
    c10.states_[8]['a'].push_back(9);
    c10.states_[10]['b'].push_back(11);
    c10.states_[9][STATE_EPSILON].push_back(12);
    c10.states_[11][STATE_EPSILON].push_back(12);
    c10.states_[12][STATE_EPSILON].push_back(13);
    c10.states_[12][STATE_EPSILON].push_back(6);
    c10.states_[13]['3'].push_back(15);

    for (int i = 'a'; i <= 'z'; ++i)
    {
        c10.states_[15][i].push_back(17);
        c10.states_[15][i + 'A' - 'a'].push_back(17);
    }

    c10.states_[17]['2'].push_back(19);
    c10.states_[19]['e'].push_back(21);
    cases.push_back(c10);

    RegExpNFACase c12("(abc)+\\d((ev){2,5})?");
    c12.states_[0]['a'].push_back(1);
    c12.states_[1]['b'].push_back(3);
    c12.states_[3]['c'].push_back(5);
    c12.states_[5][STATE_EPSILON].push_back(6);
    c12.states_[5][STATE_EPSILON].push_back(0);

    for (int i = '0'; i <= '9'; ++i)
    {
        c12.states_[6][i].push_back(8);
    }

    c12.states_[8][STATE_EPSILON].push_back(11);
    c12.states_[8][STATE_EPSILON].push_back(31);
    c12.states_[10][STATE_EPSILON].push_back(31);

    c12.states_[11]['e'].push_back(12);
    c12.states_[12]['v'].push_back(14);
    c12.states_[14][STATE_EPSILON].push_back(15);

    c12.states_[15]['e'].push_back(16);
    c12.states_[16]['v'].push_back(18);
    c12.states_[18][STATE_EPSILON].push_back(19);
    c12.states_[18][STATE_EPSILON].push_back(10);

    c12.states_[19]['e'].push_back(20);
    c12.states_[20]['v'].push_back(22);
    c12.states_[22][STATE_EPSILON].push_back(23);
    c12.states_[22][STATE_EPSILON].push_back(10);

    c12.states_[23]['e'].push_back(24);
    c12.states_[24]['v'].push_back(26);
    c12.states_[26][STATE_EPSILON].push_back(27);
    c12.states_[26][STATE_EPSILON].push_back(10);

    c12.states_[27]['e'].push_back(28);
    c12.states_[28]['v'].push_back(30);
    c12.states_[30][STATE_EPSILON].push_back(10);

    cases.push_back(c12);

    // test (ab){2, 5}
    RegExpNFACase c13("(ab){2,5}");
    c13.states_[1]['a'].push_back(2);
    c13.states_[2]['b'].push_back(4);
    c13.states_[4][STATE_EPSILON].push_back(5);

    c13.states_[5]['a'].push_back(6);
    c13.states_[6]['b'].push_back(8);
    c13.states_[8][STATE_EPSILON].push_back(9);
    c13.states_[8][STATE_EPSILON].push_back(0);

    c13.states_[9]['a'].push_back(10);
    c13.states_[10]['b'].push_back(12);
    c13.states_[12][STATE_EPSILON].push_back(13);
    c13.states_[12][STATE_EPSILON].push_back(0);

    c13.states_[13]['a'].push_back(14);
    c13.states_[14]['b'].push_back(16);
    c13.states_[16][STATE_EPSILON].push_back(17);
    c13.states_[16][STATE_EPSILON].push_back(0);

    c13.states_[17]['a'].push_back(18);
    c13.states_[18]['b'].push_back(20);
    c13.states_[20][STATE_EPSILON].push_back(0);

    cases.push_back(c13);

    RegExpNFACase c14("([abcdef][0123456]+,)+");
    c14.states_[0]['a'].push_back(1);
    c14.states_[0]['b'].push_back(1);
    c14.states_[0]['c'].push_back(1);
    c14.states_[0]['d'].push_back(1);
    c14.states_[0]['e'].push_back(1);
    c14.states_[0]['f'].push_back(1);
    c14.states_[1]['0'].push_back(3);
    c14.states_[1]['1'].push_back(3);
    c14.states_[1]['2'].push_back(3);
    c14.states_[1]['3'].push_back(3);
    c14.states_[1]['4'].push_back(3);
    c14.states_[1]['5'].push_back(3);
    c14.states_[1]['6'].push_back(3);
    c14.states_[3][STATE_EPSILON].push_back(4);
    c14.states_[3][STATE_EPSILON].push_back(1);
    c14.states_[4][','].push_back(6);
    c14.states_[6][STATE_EPSILON].push_back(0);
    c14.states_[6][STATE_EPSILON].push_back(7);

    cases.push_back(c14);

    // TODO, more cases
    RegExpNFA nfa(false);
    RegExpSyntaxTree regSynTree;

    int start, accept;
    for (int i = 0; i < cases.size(); ++i)
    {
        try
        {
            regSynTree.BuildSyntaxTree(cases[i].txt_, cases[i].txt_ + strlen(cases[i].txt_) - 1);
            nfa.BuildMachine(&regSynTree);
        }
        catch (...)
        {
            std::cout << "parsing exception, case:" << i << std::endl;
            continue;
        }

        start = nfa.GetStartState();
        accept = nfa.GetAcceptState();

        RegExpNFA::NFA_TRAN_T trans = nfa.GetNFATran();

        for (RegExpNFACase::nfa_set::iterator nit = cases[i].states_.begin(); nit != cases[i].states_.end(); ++nit)
        {
            int st = nit->first;
            for (std::map<int, std::vector<int> >::iterator it = cases[i].states_[st].begin(); it != cases[i].states_[st].end(); ++it)
            {
                int ch = it->first;
                EXPECT_EQ(it->second.size(), trans[st][ch].size()) << "tran state not equal, case: " << i << ", txt:" << cases[i].txt_ << ", state:" << st << ", ch:" << ch << std::endl;

                std::sort(it->second.begin(), it->second.end());
                std::sort(trans[st][ch].begin(), trans[st][ch].end());

                for (int j = 0; j < it->second.size(); ++j)
                {
                    EXPECT_EQ(it->second[j], trans[st][ch][j]) << "case: " << i << ", txt:" << cases[i].txt_ << ", state:" << st << ", ch:" << ch << ", j:" << j << std::endl;
                }
            }
        }
    }
}


class nfa_case
{
    public:

        nfa_case(const char* pattern, bool partial = true)
            :pattern_(pattern), nfa_(partial)
        {
            tree_.BuildSyntaxTree(pattern, pattern + strlen(pattern) - 1);
            nfa_.BuildMachine(&tree_);
        }

        void AddTestCase(const std::string& txt, bool ismatch)
        {
            txt2match_[txt] = ismatch;
        }

        bool IsMatch(const std::string& txt) { return txt2match_[txt]; }

    private:

        std::string pattern_;
        RegExpNFA nfa_;
        RegExpSyntaxTree tree_;
        std::map<std::string, bool> txt2match_;
};

TEST(test_matching_txt, test_automata_gen)
{
    std::vector<nfa_case*> cases;

    nfa_case* c1 = new nfa_case("^([abc]+\\d)*(a|b)+3\\w2e");
    c1->AddTestCase("a3b3c2e", true);
    c1->AddTestCase("aa3b3c2e", true);
    c1->AddTestCase("ab3b3c2e", true);
    c1->AddTestCase("ab32ab3e2e", false);
    c1->AddTestCase("ab3ac32e", false);
    c1->AddTestCase("ab3ab3aa3e2e", true);
    c1->AddTestCase("ab3aa3aa3v2e", true);
    c1->AddTestCase("ab32ab32e", false);
    c1->AddTestCase("ab3b4c2a3e", false);
    cases.push_back(c1);

    nfa_case* c2 = new nfa_case("(abc)+\\d((ev){2,5})?$");
    c2->AddTestCase("abc3", true);
    c2->AddTestCase("abc3evevev", true);
    c2->AddTestCase("abc3evevevevevev", false);
    c2->AddTestCase("abcabcabc3", true);
    c2->AddTestCase("abcara3", false);
    c2->AddTestCase("abcabbaae2", false);
    cases.push_back(c2);

    nfa_case* c3 = new nfa_case("regexp|coding", false);
    c3->AddTestCase("regexp", true);
    c3->AddTestCase("sregexp", false);
    c3->AddTestCase("coding", true);
    c3->AddTestCase("codingv", false);
    c3->AddTestCase("\\|", false);
    cases.push_back(c3);

    nfa_case* c3_0 = new nfa_case("(regexp|coding)", false);
    c3_0->AddTestCase("regexp", true);
    c3_0->AddTestCase("coding", true);
    c3_0->AddTestCase("codingv", false);
    c3_0->AddTestCase("\\|", false);
    cases.push_back(c3_0);

    nfa_case* c3_1 = new nfa_case("(regexp|(coding))", false);
    c3_1->AddTestCase("regexp", true);
    c3_1->AddTestCase("coding", true);
    c3_1->AddTestCase("codingv", false);
    c3_1->AddTestCase("\\|", false);
    cases.push_back(c3_1);

    nfa_case* c4 = new nfa_case("([abcdef][0123456]+,)+", false);
    c4->AddTestCase("a33", false);
    c4->AddTestCase("a33,", true);
    c4->AddTestCase("a2,a3,b4", false);
    c4->AddTestCase("a2,a3,", true);
    c4->AddTestCase("a332,b3,b34,", true);
    c4->AddTestCase("aa332,b3,b34,", false);
    c4->AddTestCase("aa332,bb3,b34,", false);
    c4->AddTestCase("a332,bb3,b34,", false);
    cases.push_back(c4);

    nfa_case* c5 = new nfa_case(".*regexp.*", false);
    c5->AddTestCase("regexp", true);
    c5->AddTestCase("aaregexp", true);
    c5->AddTestCase("regexpbb", true);
    c5->AddTestCase("aaregexpbb", true);
    c5->AddTestCase("aaraegexpbb", false);
    c5->AddTestCase("aaregexpregexpbb", true);
    c5->AddTestCase("aaregsexpregexpbb", true);
    c5->AddTestCase("aaregesxpxpbb", false);
    cases.push_back(c5);

    nfa_case* c6 = new nfa_case("abc", false);
    c6->AddTestCase("", false);
    cases.push_back(c6);

    nfa_case* c7 = new nfa_case("^(ab|cd)e");
    c7->AddTestCase("abcde", false);
    c7->AddTestCase("cde", true);
    c7->AddTestCase("abe", true);
    cases.push_back(c7);

    nfa_case* c7_0 = new nfa_case("(ab|cd)e", false);
    c7_0->AddTestCase("abcde", false);
    c7_0->AddTestCase("cde", true);
    c7_0->AddTestCase("cvde", false);
    c7_0->AddTestCase("abe", true);
    cases.push_back(c7_0);

    nfa_case* c8 = new nfa_case("a([bc]*)(c*d)", false);
    c8->AddTestCase("abcd", true);
    c8->AddTestCase("abccd", true);
    c8->AddTestCase("cde", false);
    c8->AddTestCase("ab", false);
    c8->AddTestCase("abd", true);
    cases.push_back(c8);

    nfa_case* c8_0 = new nfa_case("a([bc]+)(c*d)", false);
    c8_0->AddTestCase("abcd", true);
    c8_0->AddTestCase("abcbccd", true);
    c8_0->AddTestCase("abccd", true);
    c8_0->AddTestCase("cde", false);
    c8_0->AddTestCase("abc", false);
    c8_0->AddTestCase("abd", true);
    cases.push_back(c8_0);

    nfa_case* c9 = new nfa_case("(ab|a)b*c", false);
    c9->AddTestCase("abc", true);
    c9->AddTestCase("abbc", true);
    c9->AddTestCase("cde", false);
    c9->AddTestCase("ab", false);
    c9->AddTestCase("abbc", true);
    cases.push_back(c9);

    nfa_case* c10 = new nfa_case("((a)(b)c)(d)", false);
    c10->AddTestCase("abcd", true);
    c10->AddTestCase("cde", false);
    c10->AddTestCase("ab", false);
    c10->AddTestCase("abc", false);
    cases.push_back(c10);

    nfa_case* c11 = new nfa_case("^(ab|ab+)ef");
    c11->AddTestCase("abef", true);
    c11->AddTestCase("abbbbef", true);
    c11->AddTestCase("ababef", false);
    cases.push_back(c11);

    nfa_case* c12 = new nfa_case("^(a(bc+|b[eh])g|.h)$");
    c12->AddTestCase("bbabhg", false);
    c12->AddTestCase("abh", false);
    c12->AddTestCase("babh", false);
    c12->AddTestCase("abccg", true);
    c12->AddTestCase("abeg", true);
    c12->AddTestCase("gh", true);
    cases.push_back(c12);

    for (int i = 0; i < cases.size(); ++i)
    {
        for (std::map<std::string, bool>::iterator it = cases[i]->txt2match_.begin();
                it != cases[i]->txt2match_.end(); ++it)
        {
            EXPECT_EQ(it->second, cases[i]->nfa_.RunMachine(it->first.c_str(), it->first.c_str() + it->first.size() - 1)) \
                << "case:" << i << ", pattern:" << cases[i]->pattern_ << ", test:" << it->first << std::endl;
        }

        delete cases[i];
    }
}

