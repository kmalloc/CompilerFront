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
    c9.states_[0]['a'].push_back(1);
    c9.states_[1]['b'].push_back(3);
    c9.states_[4]['a'].push_back(5);
    c9.states_[5]['b'].push_back(7);
    c9.states_[8]['a'].push_back(9);
    c9.states_[9]['b'].push_back(11);
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

    // TODO, more cases
    RegExpNFA nfa;
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

        nfa_case(const char* pattern)
            :pattern_(pattern)
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

    nfa_case* c1 = new nfa_case("([abc]+\\d)*(a|b)+3\\w2e");
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

    nfa_case* c2 = new nfa_case("(abc)+\\d");
    c2->AddTestCase("abc3", true);
    c2->AddTestCase("abcabcabc3", true);
    c2->AddTestCase("abcara3", false);
    c2->AddTestCase("abcabbaae2", false);
    cases.push_back(c2);

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

